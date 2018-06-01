/* -*- coding: utf-8 -*- */

/**
 * XP about Reservoir Computing
 *
 * 1) OK: generate MackeyGlass, store it (and parameters) and can load
 * 2) OK: generate ESN+LAY, store it and can load
 * 3) OK: learn using data
 *    => pour l'instant, apprend avec toutes les données
 *    => pour l'instant RidgeRegression cherche ma meilleure régulation
 *    TODO : faudrait faire du bruit dans RES avant apprentissage, non ?
 * 4) TODO: check/validate with leftover data
 * 5) OK: utilise ESN pour générer les données.
 * 6) OK: prepare files for display (using R ?)
 *    => juste un output par ligne dans un fichier.
 */

#include <iostream>                // std::cout
#include <fstream>                 // std::ofstream
#include <string>                  // std::string
#include "rapidjson/document.h"         // rapidjson's DOM-style API

// Parsing command line options
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <mackeyglass.hpp>
#include <reservoir.hpp>
#include <layer.hpp>
#include <ridge_regression.hpp>

#include <utils.hpp>                  // various str_xxx
using namespace utils::rj;

// ******************************************************************** global
#define MG_MEM_SIZE       10
#define MG_LENGTH       1000
#define MG_SEED   1434979356

// Reservoir + Layer
#define INPUT_SIZE         1
#define OUTPUT_SIZE        1
// ********************************************************************* param
std::string*           _gene_data = nullptr;
std::string*           _load_data = nullptr;
std::string*           _gene_esn = nullptr;
std::string*           _load_esn = nullptr;

Reservoir*             _res = nullptr;
Layer*                 _lay = nullptr;

MackeyGlass::Data      _mg_data;
RidgeRegression::Data  _data;
// ******************************************************************* options
void setup_options(int argc, char **argv)
{
  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "produce help message")
    ("gene_data", po::value<std::string>(), "generate MackeyGlass Data in file")
    ("load_data", po::value<std::string>(), "load Data from file")
    ("gene_esn",  po::value<std::string>(), "generate ESN in file")
    ("load_esn",  po::value<std::string>(), "load ESN from file")
    ;

  // Options en ligne de commande
  po::options_description cmdline_options;
  cmdline_options.add(desc);

  // Options qui sont 'après'
  po::positional_options_description pod;
  //pod.add( "data_file", 1);
  
  // Parse
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).
	    options(desc).positional(pod).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
      std::cout << desc << std::endl;
    exit(1);
  }

  if (vm.count("gene_data")) {
    _gene_data = new std::string(vm["gene_data"].as< std::string>());
  }
  if (vm.count("load_data")) {
    _load_data = new std::string(vm["load_data"].as< std::string>());
  }

    if (vm.count("gene_esn")) {
    _gene_esn = new std::string(vm["gene_esn"].as< std::string>());
  }
  if (vm.count("load_esn")) {
    _load_esn = new std::string(vm["load_esn"].as< std::string>());
  }
}
// ****************************************************************** free_mem
void free_esn()
{
  if( _res ) delete _res;
  if( _lay ) delete _lay;
}
// *********************************************************** generate_mackey
/** 
 * Les données dans 'filename.data' et les params dans 'filename.json'
 */
void generate_mackey( const std::string& filename )
{
  MackeyGlass mackey( MG_LENGTH , 1.0, 0.1, 0.5, 2.0,
		      MG_MEM_SIZE, MG_SEED );
  _mg_data = mackey.create_sequence();

  // Serialisation dans filename.data
  std::string fn_data = filename+".data";
  std::cout << "Write MG dans " << fn_data << std::endl;
  std::ofstream ofile( fn_data );
  mackey.write( ofile, _mg_data );
  ofile.close();

  // Serialisation des parametres
  std::string fn_json = filename+".json";
  std::cout << "Write MG dans " << fn_json << std::endl;
  rapidjson::Document doc;
  rapidjson::Value obj = mackey.serialize( doc );
  std::ofstream jfile( fn_json );
  jfile << str_obj(obj) << std::endl;
  jfile.close();
}
// *************************************************************** read_mackey
void read_mackey( const std::string& filename )
{
  std::ifstream ifile( filename );
  MackeyGlass::read( ifile, _mg_data);
  ifile.close();
}
// ************************************************************** generate_esn
void generate_esn( const std::string& filename,
		   Reservoir::Toutput_size reservoir_size = 10,
		   double input_scaling = 1.0,
		   double spectral_radius= 0.99,
		   double leaking_rate = 0.1
		   )
{
  free_esn();
  _res = new Reservoir( INPUT_SIZE, reservoir_size,
		    input_scaling, spectral_radius, leaking_rate );
  _lay = new Layer( reservoir_size+1, OUTPUT_SIZE );
  
  // Serialisation dans filename.data
  std::string fn_esn = filename+".esn";
  std::cout << "Write ESN dans " << fn_esn << std::endl;

  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember( "esn", _res->serialize(doc), doc.GetAllocator());
  doc.AddMember( "lay", _lay->serialize(doc), doc.GetAllocator());

  // Write to file
  std::ofstream ofile( fn_esn );
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
// ***************************************************************** display
std::string str_dump()
{
  std::stringstream dump;
  dump << "** RESERVOIR **" << std::endl;
  if( _res )
    dump << _res->str_dump() << std::endl;

  dump << "** LAYER ******" << std::endl;
  if( _lay )
    dump << _lay->str_dump() << std::endl;

  dump << "** MG_DATA has " << _mg_data.size() << " values" << std::endl;
  for( auto& item: _mg_data ) {
    dump << item << "; ";
  }
  dump << std::endl;
  
  return dump.str();
};
// ******************************************************************* predict
void predict()
{
  // un vecteur de output
  std::vector<RidgeRegression::Toutput> result;
  
  // suppose que _mg_data a été initialisé
  Reservoir::Tinput in;
  for( unsigned int i = 1; i < _mg_data.size(); ++i) {
    // Prépare input
    in.clear();
    in.push_back( _mg_data[i-1] );
    // Passe dans réservoir
    auto out_res = _res->forward( in );
    // Ajoute 1.0 en bout (le neurone biais)
    out_res.push_back( 1.0 );
    // Passe dans layer
    auto out_lay = _lay->forward( out_res ); 

    result.push_back( out_lay );
  }

  std::cout << "** PREDICT **" << std::endl;
  for( auto& item: result) {
    std::cout << utils::str_vec(item) << std::endl;
  }
  // Serialisation dans filename.data
  std::string fn_data = "data/result.data";
  std::cout << "Write RESULTS dans " << fn_data << std::endl;
  std::ofstream ofile( fn_data );
  for( auto& item: result) {
    ofile << utils::str_vec(item) << std::endl;
  }
  ofile.close();
  
}
// ********************************************************************* learn
void learn()
{
  // Preparation des données d'apprentissage et de test
  _data.clear();
  Reservoir::Tinput in;
  for( unsigned int i = 1; i < _mg_data.size(); ++i) {
    // Prépare input
    in.clear();
    in.push_back( _mg_data[i-1] );
    // Passe dans réservoir
    auto out_res = _res->forward( in );
    // Ajoute 1.0 en bout (le neurone biais)
    out_res.push_back( 1.0 );
    
    // Prépare target
    RidgeRegression::Toutput target;
    target.push_back( _mg_data[i] );

    // Ajoute dans Data l'échantillon (entré, sortie désirée)
    _data.push_back( RidgeRegression::Sample( out_res, target) );
  }

  // Ridge Regression pour apprendre la couche de sortie
  RidgeRegression reg( _res->output_size()+1, /* res output size +1 */
		       1, /*target size */
		       1.0  /* regule */
		       );
  // Apprend, avec le meilleur coefficient de régulation
  reg.learn( _data, _lay->weights() );
  std::cout << "***** POIDS après REGRESSION **" << std::endl;
  std::cout << _lay->str_dump() << std::endl;
}


// ********************************************************************** main
int main( int argc, char *argv[] )
{
  // init random by default
  // Generate seed
  unsigned int seed = utils::random::rnd_int<unsigned int>();
  std::srand( seed );
  
  setup_options( argc, argv );

  // Generate
  if( _gene_data ) {
    std::cout << "** Generate MackeyGlass::Data in " << *_gene_data << std::endl;
    generate_mackey( *_gene_data );
  }
  if( _gene_esn ) {
    std::cout << "** Generate ESN in " << *_gene_esn << std::endl;
    generate_esn( *_gene_esn );
  }

  // Load and use
  if( _load_data ) {
    std::cout << "** Load MackeyGlass::Data in " << *_load_data << std::endl;
    read_mackey( *_load_data );
  }
  if( _load_esn ) {
    std::cout << "** Load ESN in " << *_load_esn << std::endl;
    read_esn( *_load_esn );
  }

  std::cout << str_dump() << std::endl;

  // learn if conditions are met
  if( _res and _lay and _mg_data.size() > 0 ) {
    std::cout << "** LEARNING **" << std::endl;
    learn();
    predict();
  }
  
  // end
  free_esn();
  return 0;
}


