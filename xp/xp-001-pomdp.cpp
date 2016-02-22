/* -*- coding: utf-8 -*- */

/** 
 * XP with POMDP and Reservoir Computing
 *
 * - load POMDP and generate Trajectories of S,O,A,S',O',R
 * - store Trajectories
 * - load Trajectories 
 * - générer un esn pour apprendre (bonne dimensions
 * - load un esn
 * TODO : try to learn O,A -> S (??????)
 */

#include <iostream>                // std::cout
#include <fstream>                 // std::ofstream
#include <string>                  // std::string
#include <sstream>                 // std::stringdtream
#include <rapidjson/document.h>    // rapidjson's DOM-style API
#include <json_wrapper.hpp>        // JSON::IStreamWrapper

#include <noise.hpp>

#include <pomdp/pomdp.hpp>
#include <pomdp/trajectory.hpp>

#include <reservoir.hpp>
#include <layer.hpp>
#include <ridge_regression.hpp>

#include <gsl/gsl_rng.h>             // gsl random generator
#include <ctime>                     // std::time
#include <algorithm>                 // fill

// Parsing command line options
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <utils.hpp>                  // various str_xxx
using namespace utils::rj;
// ********************************************************************* param
std::string*           _filename_pomdp = nullptr;
std::string*           _fileload_traj = nullptr;
std::string*           _filegene_traj = nullptr;
std::string*           _fileload_esn = nullptr;
std::string*           _filegene_esn = nullptr;
std::string*           _fileload_noise = nullptr;
std::string*           _filegene_noise = nullptr;
std::string*           _filegene_output = nullptr;

WNoise::Data           _wnoise;
unsigned int           _noise_length;
double                 _noise_level;

Model::POMDP*          _pomdp = nullptr;
unsigned int           _length;
Trajectory::POMDP::Data _traj_data;

Reservoir*             _res = nullptr;
Layer*                 _lay = nullptr;
RidgeRegression::Data  _data;
Reservoir::Toutput_size _res_size;
double                  _res_scaling;
double                  _res_radius;
double                  _res_leak;

gsl_rng*               _rnd = gsl_rng_alloc( gsl_rng_taus );
// ****************************************************************** free_mem
void free_mem()
{
  if( _pomdp ) delete _pomdp;
  if( _filename_pomdp ) delete _filename_pomdp;
  if( _fileload_traj ) delete _fileload_traj;
  if( _filegene_traj ) delete _filegene_traj;
  if( _fileload_esn ) delete _fileload_esn;
  if( _filegene_esn ) delete _filegene_esn;
  if( _fileload_noise ) delete _fileload_noise;
  if( _filegene_noise ) delete _filegene_noise;
  if( _filegene_output ) delete _filegene_output;
}
void free_esn()
{
  if( _res ) delete _res;
  if( _lay ) delete _lay;
}
// ******************************************************************* options
void setup_options(int argc, char **argv)
{
  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "produce help message")
    ("load_pomdp,p", po::value<std::string>(), "load POMDP from JSON file")
    ("gene_traj", po::value<std::string>(), "gene Trajectory into file")
    ("traj_length", po::value<unsigned int>(&_length)->default_value(10), "generate Traj of length ")
    ("gene_esn", po::value<std::string>(), "gene ESN into file")
    ("res_size", po::value<unsigned int>(&_res_size)->default_value(10), "reservoir size")
    ("res_scaling", po::value<double>(&_res_scaling)->default_value(1.0), "reservoir input scaling")
    ("res_radius", po::value<double>(&_res_radius)->default_value(0.99), "reservoir spectral radius")
    ("res_leak", po::value<double>(&_res_leak)->default_value(0.1), "reservoir leaking rate")
    ("gene_noise", po::value<std::string>(), "gene WNoise into file")
    ("length_noise", po::value<unsigned int>(&_noise_length)->default_value(100), "Length of noise to generate")
    ("level_noise",  po::value<double>(&_noise_level)->default_value(0.1), "Level of noise to generate")
    ("load_traj,t", po::value<std::string>(), "load Trajectory from file")
    ("load_esn,e",  po::value<std::string>(), "load ESN from file")
    ("load_noise,n", po::value<std::string>(), "load WNoise from file")
    ("output,o",  po::value<std::string>(), "Output file for results")
    ;

  // Options en ligne de commande
  po::options_description cmdline_options;
  cmdline_options.add(desc);

  // Options qui sont 'après'
  po::positional_options_description pod;
  //pod.add( "data_file", 1);

  // Parse
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).
	      options(desc).positional(pod).run(), vm);

    if (vm.count("help")) {
      std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
      std::cout << desc << std::endl;
      exit(1);
    }
    
    po::notify(vm);
  }
  catch(po::error& e)  { 
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << desc << std::endl; 
    exit(2);
  } 
  

  // file names
  if (vm.count("load_pomdp")) {
    _filename_pomdp = new std::string(vm["load_pomdp"].as< std::string>());
  }
  if (vm.count("load_traj")) {
    _fileload_traj = new std::string(vm["load_traj"].as< std::string>());
  }
  if (vm.count("gene_traj")) {
    _filegene_traj = new std::string(vm["gene_traj"].as< std::string>());
  }
  if (vm.count("load_noise")) {
    _fileload_noise = new std::string(vm["load_noise"].as< std::string>());
  }
  if (vm.count("gene_noise")) {
    _filegene_noise = new std::string(vm["gene_noise"].as< std::string>());
  }
  if (vm.count("gene_esn")) {
    _filegene_esn = new std::string(vm["gene_esn"].as< std::string>());
  }
  if (vm.count("load_esn")) {
    _fileload_esn = new std::string(vm["load_esn"].as< std::string>());
  }
  if (vm.count("output")) {
    _filegene_output = new std::string(vm["output"].as< std::string>());
  }
}
// ************************************************************** load_pomdp
void load_pomdp()
{
  // free pomdp
  if( _pomdp ) delete _pomdp;
  
  // Read from file
  // std::cout << "UNSERIALZE" << std::endl;
  std::ifstream pfile( *_filename_pomdp );
  // Wrapper pour lire document
  JSON::IStreamWrapper instream(pfile);
  // Parse into a document
  rj::Document read_doc;
  read_doc.ParseStream( instream );
  pfile.close();

  _pomdp = new Model::POMDP( read_doc["pomdp"] );
}
// *************************************************************** gene_traj
void gene_traj()
{
  std::ofstream* ofile = nullptr;
  if( _filegene_traj ) {
    ofile = new std::ofstream( *_filegene_traj + ".data" );
  }
  
  // Random with seed
  unsigned long seed = std::time( NULL );
  gsl_rng_set( _rnd, seed );

  // inform traj
  if( ofile ) {
    *ofile << "## \"pomdp_name\": \"" << *_filename_pomdp << "\"," << std::endl;
    *ofile << "## \"seed\": " << seed << ", \"length\": " << _length << std::endl;
      }

  // Generate
  const std::vector<Model::Node>& list_action = _pomdp->actions();
  // Current state, obs
  unsigned int idx_state = _pomdp->cur_state()._id;
  unsigned int idx_obs = _pomdp->cur_obs()._id;
  for( unsigned int i = 0; i < _length; ++i) {
    // random action
    const Model::Node& act = list_action[gsl_rng_uniform_int( _rnd, list_action.size() )];
    _pomdp->simul_trans( act );

    unsigned int idx_next_state = _pomdp->cur_state()._id;
    unsigned int idx_next_obs = _pomdp->cur_obs()._id;
    double reward = _pomdp->cur_reward();
    
    std::cout << idx_state << "\t" << idx_obs << "\t" << act._id << "\t" << idx_next_state << "\t" << idx_next_obs << "\t" << reward << std::endl;

    if( ofile ) {
      *ofile << idx_state << "\t" << idx_obs << "\t" << act._id << "\t" << idx_next_state << "\t" << idx_next_obs << "\t" << reward << std::endl;
    }
    
    idx_state = idx_next_state;
    idx_obs = idx_next_obs;
  }

  if( ofile ) delete ofile;
}
// ***************************************************************** read_traj
void read_traj( const std::string& filename )
{
  std::ifstream ifile( filename );
  Trajectory::POMDP::read( ifile, _traj_data);
  ifile.close();
}
// ****************************************************************** gene_esn
void gene_esn( const std::string& filename,
	       Reservoir::Tinput_size input_size = 1,
	       Layer::Toutput_size output_size = 1,   
	       Reservoir::Toutput_size reservoir_size = 10,
	       double input_scaling = 1.0,
	       double spectral_radius= 0.99,
	       double leaking_rate = 0.1
	       )
{
  free_esn();
  _res = new Reservoir( input_size, reservoir_size,
		    input_scaling, spectral_radius, leaking_rate );
  _lay = new Layer( reservoir_size+1, output_size );
  
  // Serialisation dans filename.json
  std::stringstream stream;
  stream << filename;
  // stream << "_" << _res_size;
  // stream << "_" << _res_scaling;
  // stream << "_" << _res_radius;
  // stream << "_" << _res_leak;
  stream << ".json";
  std::cout << "Write ESN dans " << stream.str() << std::endl;

  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember( "esn", _res->serialize(doc), doc.GetAllocator());
  doc.AddMember( "lay", _lay->serialize(doc), doc.GetAllocator());

  // Write to file
  std::ofstream ofile( stream.str() );
  ofile << str_obj(doc) << std::endl;
  ofile.close();
}
// ****************************************************************** read_esn
void read_esn( const std::string& filename )
{
  free_esn();
  std::ifstream ifile( filename );
  // Wrapper pour lire document
  JSON::IStreamWrapper instream(ifile);
  // Parse into a document
  rj::Document doc;
  doc.ParseStream( instream );
  ifile.close();

  _res = new Reservoir( doc["esn"] );
  _lay = new Layer( doc["lay"] );
}
// **************************************************************** load_noise
void read_noise( const std::string& filename )
{
  std::ifstream ifile( filename );
  WNoise::read( ifile, _wnoise);
  ifile.close();
}
// **************************************************************** gene_noise
void gene_noise( const std::string& filename,
		 const unsigned int length,
		 const double level,
		 const unsigned int dim)
{
  WNoise wnoise( length, level, dim );
  wnoise.create_sequence();
  _wnoise = wnoise.data();
  
  // Sauve dans JSON
  std::string fn_json = filename+".json";
  std::cout << "Write WNoise dans " << fn_json << std::endl;
  rapidjson::Document doc;
  rapidjson::Value obj = wnoise.serialize( doc );
  std::ofstream jfile( fn_json );
  jfile << str_obj(obj) << std::endl;
  jfile.close();

  // Sauve les données
  std::string fn_data = filename+".data";
  std::cout << "Write WNoise dans " << fn_data << std::endl;
  std::ofstream ofile( fn_data );

  // inform traj
  ofile << "## \"pomdp_name\": \"" << *_filename_pomdp << "\"" << std::endl;
  
  WNoise::write( ofile, _wnoise );
  ofile.close();
}
// ********************************************************************* learn
Reservoir::Tinput input_from( const Trajectory::POMDP::Item& item )
{
  // O+A neurones
  Reservoir::Tinput input(_res->input_size() );
  std::fill( input.begin(), input.end(), 0.0 );
  
  input[item.id_o] = 1.0;
  input[_pomdp->_obs.size()+item.id_a] = 1.0;

  return input;
}
RidgeRegression::Toutput target_from( const Trajectory::POMDP::Item& item )
{
  // Try to learn S
  RidgeRegression::Toutput target(_lay->output_size() );
  std::fill( target.begin(), target.end(), 0.0 );

  target[item.id_next_s] =1.0;

  return target;
}
void init()
{
  for( auto& item: _wnoise ) {
    // Passe dans réservoir
    auto out_res = _res->forward( item );
  }
}
void learn()
{
  // Preparation des données d'apprentissage et de test
  _data.clear();
  for( auto& item: _traj_data) {
    // Passe dans réservoir
    auto out_res = _res->forward( input_from(item) );
    // Ajoute 1.0 en bout (le neurone biais)
    out_res.push_back( 1.0 );
    
    // Ajoute dans Data l'échantillon (entré, sortie désirée)
    _data.push_back( RidgeRegression::Sample( out_res, target_from(item)) );
  }

  // Ridge Regression pour apprendre la couche de sortie
  RidgeRegression reg( _res->output_size()+1, /* res output size +1 */
		       _lay->output_size(), /*target size */
		       1.0  /* regule */
		       );
  // Apprend, avec le meilleur coefficient de régulation
  reg.learn( _data, _lay->weights() );
  std::cout << "***** POIDS après REGRESSION **" << std::endl;
  std::cout << _lay->str_dump() << std::endl;
}
// ******************************************************************* predict
std::vector<RidgeRegression::Toutput>
predict( Reservoir& res,
	 Layer& lay,
	 const Trajectory::POMDP::Data& traj )
{
  // un vecteur de output
  std::vector<RidgeRegression::Toutput> result;
  
  // suppose que _mg_data a été initialisé
  for( auto& item: traj) {
    // Passe dans réservoir
    auto out_res = res.forward( input_from(item) );
    // Ajoute 1.0 en bout (le neurone biais)
    out_res.push_back( 1.0 );
    // Passe dans layer
    auto out_lay = lay.forward( out_res ); 

    result.push_back( out_lay );
  }

  std::cout << "** PREDICT **" << std::endl;
  for( auto& item: result) {
    std::cout << utils::str_vec(item) << std::endl;
  }
  // // Serialisation dans filename.data
  // std::string fn_data = "data/result.data";
  // std::cout << "Write RESULTS dans " << fn_data << std::endl;
  // std::ofstream ofile( fn_data );
  // for( auto& item: result) {
  //   ofile << utils::str_vec(item) << std::endl;
  // }
  // ofile.close();

  return result;  
}
// ********************************************************************** main
int main( int argc, char *argv[] )
{
  setup_options( argc, argv );

  // Charger le POMDP
  if( _filename_pomdp) {
    std::cout << "** Load POMDP from " << *_filename_pomdp << std::endl;
    load_pomdp();
  }
  // Si POMDP + nom traj => générer et sauver une trajectoire
  if( _filename_pomdp and _filegene_traj ) {
    std::cout << "** Gene TRAJ into " << *_filegene_traj << std::endl;
    gene_traj();
  }
  // Si POMDP + gene_esn => générer et sauver un esn
  if( _filename_pomdp and _filegene_esn ) {
    std::cout << "** Gene ESN into " << *_filegene_esn << std::endl;
    gene_esn( *_filegene_esn,
	      _pomdp->_obs.size() + _pomdp->_actions.size(), // In = O+A
	      _pomdp->_states.size(),                    // out = S
	      _res_size,                                  // _res size
	      _res_scaling, _res_radius, _res_leak
	      );
  }  
  // Si POMDP + gene_noise => générer et sauver noise
  if( _filename_pomdp and _filegene_noise ) {
    std::cout << "** Gene NOISE into " << *_filegene_noise << std::endl;
    gene_noise( *_filegene_noise,
		_noise_length, _noise_level,
		_pomdp->_obs.size() + _pomdp->_actions.size()
		);
  }
  // Si load_traj => charger une trajectoire
  if( _fileload_traj ) {
    std::cout << "** Load Trajectory::POMDP::Data from " << *_fileload_traj << std::endl;
    read_traj( *_fileload_traj );

    std::cout << "** Trajectory Read" << std::endl;
    for( auto& item: _traj_data) {
      std::cout << item.id_s << ":" << item.id_o << "+" << item.id_a << "->" << item.id_next_s << ":" << item.id_next_o << " = " << item.r << std::endl;
    }
  }
  // Si load_noise => charger un noise
  if( _fileload_noise ) {
    std::cout << "** Load WNoise::Data from " << *_fileload_noise << std::endl;
    read_noise( *_fileload_noise );
    std::cout << "Read " << _wnoise.size() << " noise data" << std::endl;
  }
  // Si load_esn => charger un ESN
  if( _fileload_esn ) {
    std::cout << "** Load ESN from " << *_fileload_esn << std::endl;
    read_esn( *_fileload_esn );
  }

  // Si POMDP+ESN+TRAJ => learn
  if( _filename_pomdp and _fileload_esn and _fileload_traj ) {
    std::cout << "__ LEARNING **" << std::endl;
    // Si _noise, on commence par là
    std::cout << "___ init with noise" << std::endl;
    if( _fileload_noise ) {
      init();
    }
    // Sauve l'état présent du réseau
    Reservoir res_after_init( *_res );
    // Apprendre => modifie _lay par régression
    std::cout << "___ learn()" << std::endl;
    learn();
    // Première prédiction à partir de l'état du réseau appris
    std::cout << "___ predict follow" << std::endl;
    std::vector<RidgeRegression::Toutput> result_after_learn = predict( *_res, *_lay, _traj_data );
    // Deuxième prédiction à partir de l'état du réseau avant apprentissage
    // mais avec _lay modifié
    std::cout << "___ predict base" << std::endl;
    std::vector<RidgeRegression::Toutput> result_after_init = predict( res_after_init, *_lay, _traj_data );
    
    // Les résultats
    // Affiche targert: \n pred\n init\n
    unsigned int idx_out = 0;
    for( auto& item: _traj_data) {
      std::cout << "target:" << utils::str_vec(target_from(item))  << std::endl;
      std::cout << "pred:  " << utils::str_vec(result_after_learn[idx_out]) << std::endl;
      std::cout << "init:  " << utils::str_vec(result_after_init[idx_out]) << std::endl;
      idx_out ++;
    }

    // Dans un fichier
    if( _filegene_output ) {
      // Sauve les données
      std::cout << "** Write Output dans " << *_filegene_output << std::endl;
      std::ofstream ofile( *_filegene_output );
      // Header comments
      ofile << "## \"pomdp_name\": \"" << *_filename_pomdp << "\"," << std::endl;
      ofile << "## \"traj_name\" : \"" << *_fileload_traj << "\"," << std::endl;
      if( _fileload_noise ) {
	ofile << "## \"noise_name\" : \"" << *_fileload_noise << "\"," << std::endl;
      }
      // Header ColNames
      // target
      for( unsigned int i = 0; i < _lay->output_size(); ++i) {
	ofile << "ta_" << i << "\t";
      }
      // after learn
      for( unsigned int i = 0; i < _lay->output_size(); ++i) {
	ofile << "le_" << i << "\t";
      }
      // after init
      for( unsigned int i = 0; i < _lay->output_size(); ++i) {
	ofile << "in_" << i << "\t";
      }
      ofile << std::endl;
      // Data
      idx_out = 0;
      for( auto& item: _traj_data) {
	// target
	for( auto& var: target_from(item)) {
	  ofile << var << "\t";
	}
	// predict
	for( auto& var: result_after_learn[idx_out]) {
	  ofile << var << "\t";
	}
	// init
	for( auto& var: result_after_init[idx_out]) {
	  ofile << var << "\t";
	}
	ofile << std::endl;
	
	idx_out ++;
      }
      ofile.close();
    }

  }
  
  free_mem();
  free_esn();
  return 0;
}
