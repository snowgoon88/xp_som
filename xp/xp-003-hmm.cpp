/* -*- coding: utf-8 -*- */

/** 
 * WP with HMM (cmp with BICA's Paper de Jeremy and Herve
 *
 * - generate/save/load HMM : expr
 * - generate Trajectories of (S,O). inspi from Trajectory
 * - save/load Trajectories
 * - create ESN : two initialisation options
 * - save/load ESN
 * - use command-line arguments: -h,--help
 *
 * In practice, the following data flow is used
 * [1.0, traj.id_o] ---<res>---> [res_out]
 * SI esn.input_forward :
 *       [1.0; res_out; traj.id_o] ---<lay>---> [traj(+1).id_o]
 * SINON 
 *       [1.0; res_out] ---<lay>---> [traj(+1).id_o]
 */

#include <iostream>                // std::cout
#include <iomanip>                 // std::setw
#include <fstream>                 // std::ofstream
#include <string>                  // std::string
#include <sstream>                 // std::stringdtream
#include <rapidjson/document.h>    // rapidjson's DOM-style API
#include <json_wrapper.hpp>        // JSON::IStreamWrapper

#include <hmm-json.hpp>
#include <input.hpp>
#include <hmm_trajectory.hpp>

#include <reservoir.hpp>
#include <layer.hpp>
#include <ridge_regression.hpp>

#include <noise.hpp>

#include <gl_plot.hpp>

// Parsing command line options
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <utils.hpp>                  // various str_xxx
using namespace utils::rj;
// ******************************************************************** Global
// HMM
using Problem = struct {
  std::string expr = "ABCD";
  bica::hmm::T t;
  bica::hmm::O o;
  unsigned int nb_states;
};
std::unique_ptr<Problem> _pb = nullptr;

// Trajectory
using Traj = Trajectory::HMM::Data;
std::unique_ptr<Traj>    _data = nullptr;

// ESN
using PtrReservoir =   std::unique_ptr<Reservoir>;
using PtrLayer     = std::unique_ptr<Layer>;
using ESN = struct {
  PtrReservoir      res = nullptr;
  PtrLayer          lay = nullptr;
  bool    input_forward = false;
};
std::unique_ptr<ESN>     _esn;
// Internal stat of ESN, input to Layer
using LayerData = std::vector<Layer::Tinput>;
LayerData                _data_lay_in;

// WNoise
using PtrWNoiseData = std::unique_ptr<WNoise::Data>;
PtrWNoiseData            _noise;

// Options
std::unique_ptr<std::string> _opt_expr               = nullptr;
std::unique_ptr<std::string> _opt_filesave_hmm       = nullptr;
std::unique_ptr<std::string> _opt_fileload_hmm       = nullptr;
unsigned int                 _opt_traj_length        = 10;
std::unique_ptr<std::string> _opt_filesave_traj      = nullptr;
std::unique_ptr<std::string> _opt_fileload_traj      = nullptr;
Reservoir::Toutput_size      _opt_res_size           = 10;
double                       _opt_res_scaling        = 1.0;
double                       _opt_res_radius         = 0.99; 
double                       _opt_res_leak           = 0.1;
bool                         _opt_res_forward        = false;
bool                         _opt_res_szita          = false;
double                       _opt_res_szita_val      = 5.0;
std::unique_ptr<std::string> _opt_filesave_esn       = nullptr;
std::unique_ptr<std::string> _opt_fileload_esn       = nullptr;
double                       _opt_noise_level        = 0.1;
unsigned int                 _opt_noise_length       = 100;
std::unique_ptr<std::string> _opt_filesave_noise     = nullptr;
std::unique_ptr<std::string> _opt_fileload_noise     = nullptr;
double                       _opt_regul              = 1.0;
unsigned int                 _opt_test_length        = 10;
std::unique_ptr<std::string> _opt_file_result        = nullptr;
bool                         _opt_graph              = false;
bool                         _opt_verb               = false;
// Learn
RidgeRegression::Data        _sample_data;
// ***************************************************************************
// ******************************************************************* options
// ***************************************************************************
void setup_options(int argc, char **argv)
{
  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "produce help message")
    ("create_hmm", po::value<std::string>(), "create an HMM from string")
    ("save_hmm", po::value<std::string>(), "save HMM in filename")
    ("load_hmm,m", po::value<std::string>(), "load HMM from filename")

    ("length_traj", po::value<unsigned int>(&_opt_traj_length)->default_value(_opt_traj_length), "create a Traj with length")
    ("save_traj", po::value<std::string>(), "save Traj in filename")
    ("load_traj,t", po::value<std::string>(), "load Traj from filename")

    ("res_size", po::value<unsigned int>(&_opt_res_size)->default_value(_opt_res_size),"reservoir size")
    ("res_forward", "reservoir input forwarding")
    ("res_szita_init", "reservoir: use szita initialisation")
    ("res_szita_val", po::value<double>(&_opt_res_szita_val)->default_value(_opt_res_szita_val), "reservoir: value for init weights")
    ("res_scaling", po::value<double>(&_opt_res_scaling)->default_value(_opt_res_scaling), "reservoir input scaling")
    ("res_radius", po::value<double>(&_opt_res_radius)->default_value(_opt_res_radius), "reservoir spectral radius")
    ("res_leak", po::value<double>(&_opt_res_leak)->default_value(_opt_res_leak), "reservoir leaking rate")
    ("save_esn", po::value<std::string>(), "save ESN in filename")
    ("load_esn,e", po::value<std::string>(), "load ESN from filename")
    
    ("noise_length", po::value<unsigned int>(&_opt_noise_length)->default_value(_opt_noise_length), "Length of noise to generate")
    ("noise_level",  po::value<double>(&_opt_noise_level)->default_value(_opt_noise_level), "Level of noise to generate")    
    ("save_noise", po::value<std::string>(), "save WNoise in filename")
    ("load_noise,n", po::value<std::string>(), "load WNoise from filename")
    
    ("regul", po::value<double>(&_opt_regul)->default_value(_opt_regul), "regul for RidgeRegrression")
    ("test_length,l", po::value<unsigned int>(&_opt_test_length)->default_value(_opt_test_length), "Length of test")
    
    ("output,o",  po::value<std::string>(), "Output file for results")
    ("graph,g", "graphics" )
    ("verb,v", "verbose" )
    //("verb,v", po::value<bool>(&_opt_verb)->default_value(false), "verbose" )
  ;

  // Options en ligne de commande
  po::options_description cmdline_options;
  cmdline_options.add(desc);
  
  // Options qui sont 'apres'
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

  // HMM
  if( vm.count("create_hmm")) {
    _opt_expr = make_unique<std::string>(vm["create_hmm"].as< std::string>());
  }
  if (vm.count("save_hmm")) {
    _opt_filesave_hmm = make_unique<std::string>(vm["save_hmm"].as< std::string>());
  }
  if (vm.count("load_hmm")) {
    _opt_fileload_hmm = make_unique<std::string>(vm["load_hmm"].as< std::string>());
  }
  // TRAJ
  if (vm.count("save_traj")) {
    _opt_filesave_traj = make_unique<std::string>(vm["save_traj"].as< std::string>());
  }
  if (vm.count("load_traj")) {
    _opt_fileload_traj = make_unique<std::string>(vm["load_traj"].as< std::string>());
  }

  // ESN
  if (vm.count("save_esn")) {
    _opt_filesave_esn = make_unique<std::string>(vm["save_esn"].as< std::string>());
  }
  if (vm.count("load_esn")) {
    _opt_fileload_esn = make_unique<std::string>(vm["load_esn"].as< std::string>());
  }

  // WNoise
  if (vm.count("save_noise")) {
    _opt_filesave_noise = make_unique<std::string>(vm["save_noise"].as< std::string>());
  }
  if (vm.count("load_noise")) {
    _opt_fileload_noise = make_unique<std::string>(vm["load_noise"].as< std::string>());
  }

  // RESULT
  if (vm.count("output")) {
    _opt_file_result = make_unique<std::string>(vm["output"].as< std::string>());
  }

  // Options
  if( vm.count("res_forward") ) {
    _opt_res_forward = true;
  }
  if( vm.count("res_szita_init") ) {
    _opt_res_szita = true;
  }
  if( vm.count("verb") ) {
    _opt_verb = true;
  }
  if( vm.count("graph") ) {
    _opt_graph = true;
  }
};

// **************************************************************** create_hmm
Problem create_hmm( const std::string& expr = "ABCD" )
{
  Problem pb;
  pb.expr = expr;
  auto hmm = bica::hmm::make(expr);
  std::tie(pb.t, pb.o) = hmm.first;
  pb.nb_states = hmm.second;

  return pb;
}
// ****************************************************************** save_hmm
void save_hmm( const std::string& filename, const std::string hmm_expr )
{
  auto ofile = std::ofstream( filename );

  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember( "hmm",
		 bica::hmm::serialize( doc, hmm_expr ),
		 doc.GetAllocator());

  ofile << str_obj(doc) << std::endl;
  ofile.close();
};
// ****************************************************************** load_hmm
Problem load_hmm( const std::string& filename )
{
  auto pfile = std::ifstream( filename );

  // Wrapper pour lire document
  JSON::IStreamWrapper instream(pfile);
  // Parse into a document
  rj::Document read_doc;
  read_doc.ParseStream( instream );
  pfile.close();

  auto expr = bica::hmm::unserialize( read_doc["hmm"] );
  return create_hmm( expr );
};
// ***************************************************************** print_hmm
void print_hmm(const std::string& name, bica::sampler::HMM& h, int nb)
{
  std::cout << name << " : " << std::fixed;
  for(int i = 0; i < nb; ++i, h.shift())
    std::cout << std::setw(5) << std::setprecision(2) << h.input() << '('
	      << std::setw(2) << h.input_id() << ") ";
  std::cout << std::endl;
  std::cout << std::endl;
}
// *************************************************************** create_traj
Traj create_traj( const unsigned int length, const Problem& pb )
{
  Traj traj;
  // Sampler pour le HMM
  bica::sampler::HMM hmm( pb.t, pb.o);

  traj.clear();
  for( unsigned int i = 0; i < length; ++i) {
    traj.push_back( Trajectory::HMM::Item{ hmm.input_id(), hmm.input()} );
    hmm.shift();
  }

  return traj;
};
// ***************************************************************** save_traj
void save_traj( const std::string& filename,
		const Traj& data,
		const std::string hmm_expr)
{
  auto ofile = std::ofstream( filename );

  // header
  ofile << "## \"hmm_expr\": \"" << hmm_expr << "\"," << std::endl;

  Trajectory::HMM::save( ofile, data );
  
  ofile.close();
};
// ***************************************************************** load_traj
Traj load_traj( const std::string& filename )
{
  Traj traj;
  auto pfile = std::ifstream( filename );
  Trajectory::HMM::read( pfile, traj );
  pfile.close();

  return traj;
};
// **************************************************************** create_esn
ESN create_esn( Reservoir::Tinput_size input_size = 1,
		Layer::Toutput_size output_size = 1,   
		Reservoir::Toutput_size reservoir_size = 10,
		bool forward_input = false,
		bool szita_input = false,
		double szita_val = 1.0,
		double input_scaling = 1.0,
		double spectral_radius= 0.99,
		double leaking_rate = 0.1
		)
{
  ESN esn;
  esn.res = make_unique<Reservoir>( input_size, reservoir_size,
				   input_scaling, spectral_radius, leaking_rate );
  if( forward_input == true) {
    // layer take also input 
    esn.lay = make_unique<Layer>( input_size+reservoir_size, output_size );
  }
  else {
    esn.lay = make_unique<Layer>( reservoir_size+1, output_size );
  }
  esn.input_forward = forward_input;

  // Special Input Weights bu [Szita06]
  if( szita_input == true) {
    //std::cout << "  Szita initialization, val=" << szita_val << std::endl;
    esn.res->init_discrete_input( szita_val );
  }

  return esn;
};
// ****************************************************************** save_esn
void save_esn( const std::string& filename, const ESN& esn )
{
  auto ofile = std::ofstream( filename );

  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember( "res", esn.res->serialize(doc), doc.GetAllocator());
  doc.AddMember( "lay", esn.lay->serialize(doc), doc.GetAllocator());
  doc.AddMember( "input_forward", rj::Value(esn.input_forward), doc.GetAllocator() );

  ofile << str_obj(doc) << std::endl;
  ofile.close();
};
// ****************************************************************** load_esn
ESN load_esn(const std::string& filename )
{
  ESN esn;
  
  std::ifstream ifile( filename );
  // Wrapper pour lire document
  JSON::IStreamWrapper instream(ifile);
  // Parse into a document
  rj::Document doc;
  doc.ParseStream( instream );
  ifile.close();

  esn.res = make_unique<Reservoir>( doc["res"] );
  esn.lay = make_unique<Layer>( doc["lay"] );
  esn.input_forward = doc["input_forward"].GetBool();
  
  ifile.close();

  return esn;
}
// ************************************************************** create_noise
WNoise::Data save_noise( const std::string& filename,
			 const unsigned int length,
			 const double level,
			 const unsigned int dim)
{
  WNoise wnoise( length, level, dim );
  wnoise.create_sequence();
  
  // Sauve dans JSON
  std::string fn_json = filename+".json";
  if( _opt_verb )
    std::cout << "Write WNoise dans " << fn_json << std::endl;
  rapidjson::Document doc;
  rapidjson::Value obj = wnoise.serialize( doc );
  std::ofstream jfile( fn_json );
  jfile << str_obj(obj) << std::endl;
  jfile.close();

  // Sauve les data
  std::string fn_data = filename+".data";
  std::cout << "Write WNoise dans " << fn_data << std::endl;
  std::ofstream ofile( fn_data );

  // inform traj
  ofile << "## \"length\": \"" << _opt_noise_length << "\"" << std::endl;
  ofile << "## \"level\": \"" << _opt_noise_level << "\"" << std::endl;
  
  WNoise::write( ofile, wnoise.data() );
  ofile.close();

  return wnoise.data();
}
// **************************************************************** load_noise
WNoise::Data load_noise( const std::string& filename )
{
  WNoise::Data data_wnoise;
  std::ifstream ifile( filename );
  WNoise::read( ifile, data_wnoise );
  ifile.close();

  return data_wnoise;
}
// ***************************************************************************
// ***************************************************************** make data
// ***************************************************************************
void push_noise( const ESN& esn,
		 const WNoise::Data::iterator& it_begin,
		 const WNoise::Data::iterator& it_end )
{
  for (auto it = it_begin; it != it_end; ++it) {
    // input
    Reservoir::Tinput res_in;
    res_in.push_back( 1.0 ); // biais value
    res_in.insert( res_in.end(), it->begin(), it->end() );
    // forward through reservoir
    auto res_out = esn.res->forward( res_in );
  }
}
LayerData
compute_lay_input( const Traj::iterator& it_traj_begin,
		   const Traj::iterator& it_traj_end,
		   ESN& esn )
{
  LayerData result;

  for (auto it = it_traj_begin; it != it_traj_end; ++it) {
    // input
    Reservoir::Tinput res_in;
    res_in.push_back( 1.0 ); // biais value
    res_in.push_back( (*it).id_o );
    // forward through reservoir
    auto res_out = esn.res->forward( res_in );

    Layer::Tinput lay_in;
    lay_in.push_back( 1.0 );
    lay_in.insert( lay_in.end(), res_out.begin(), res_out.end() );
    if( esn.input_forward ) {
      lay_in.push_back( (*it).id_o );
    }
    
    result.push_back( lay_in );
  }
  return result;
}
// ***************************************************************************
// ********************************************************************* learn
// ***************************************************************************
void learn( ESN& esn,
	    const LayerData::iterator& it_input_begin,
	    const LayerData::iterator& it_input_end,
	    const Traj::iterator& it_target_begin,
	    const Traj::iterator& it_target_end,
	    const double regul )
{
  // learn data
  RidgeRegression::Data sample_data ;
  {
    auto it_input = it_input_begin;
    auto it_target = it_target_begin;
      for ( ;
	    it_input != it_input_end and it_target != it_target_end;
	    ++it_input, ++it_target) {
	Layer::Toutput samp_tar{ it_target->id_o};
	sample_data.push_back( RidgeRegression::Sample( *it_input, samp_tar) );
      }
  }
  // DEBUG
  // for( const auto& samp:  _sample_data) {
  //   std::cout << utils::str_vec(samp.first) << " -> " << utils::str_vec(samp.second) << std::endl;
  // }

  // DEBUG
  //std::cout << "___ Regression" << std::endl;
  //std::cout << "    Before W " << esn.lay->str_dump() << std::endl;
  RidgeRegression reg( esn.lay->input_size(),
		       esn.lay->output_size(),
		       0 /* idx intercept */
		       );
  // learn
  auto error = reg.learn( sample_data, esn.lay->weights(), regul );
  // DEBUG std::cout << "    After W " << esn.lay->str_dump() << std::endl;

  // DEBUG : learning sample in file
  auto ofile = std::ofstream( "dbg_learn_data" );
  // Header ColNames
  // input
  for( unsigned int i = 0; i < _esn->lay->input_size(); ++i) {
    ofile << "in_" << i << "\t";
  }
  // target
  for( unsigned int i = 0; i < _esn->lay->output_size(); ++i) {
    ofile << "ta_" << i << "\t";
  }
  ofile << std::endl;
  // Data
  for( auto& sample: sample_data) {
    //in
    for( auto& var: sample.first) {
      ofile << var << "\t";
    }
    // target
    for( auto& var: sample.second) {
      ofile << var << "\t";
    }
    ofile << std::endl;
  }
  ofile.close();
};
// ***************************************************************************
// ******************************************************************* graph
// ***************************************************************************
void graph( const std::string& title,
	    const Traj::iterator& target_begin, const Traj::iterator& target_end,
	    const std::vector<RidgeRegression::Toutput>::iterator& out_begin,
	    const std::vector<RidgeRegression::Toutput>::iterator& out_end,
	    const unsigned int test_length )
	    
{
  GLPlot plot( title, 800, 600 );

  // Add target in green
  auto c_tar = make_unique<Curve>();
  c_tar->set_color( {0,1,0} );
  // c_tar->add_time_serie( target_begin, target_end );
  // plot.add_curve( std::move(c_tar) );

  // Add learn in blue
  auto c_learn = make_unique<Curve>();
  c_learn->set_color( {0,0,1} );
  // and test in red
  auto c_test = make_unique<Curve>();
  c_test->set_color( {1,0,0} );

  auto tdata = 0.0;
  auto it_tar = target_begin;
  auto it_out = out_begin;
  for ( ; it_out != out_end-test_length and it_tar != target_end;
	++it_out, ++it_tar) {
    c_tar->add_sample( Curve::Sample{tdata, it_tar->id_o, 0} );
    c_learn->add_sample( Curve::Sample{tdata, (*it_out)[0], 0} );
    tdata += 1.0;
  }
  for ( ; it_out != out_end and it_tar != target_end;
	++it_out, ++it_tar) {
    c_tar->add_sample( Curve::Sample{tdata, it_tar->id_o, 0} );
    c_test->add_sample( Curve::Sample{tdata, (*it_out)[0], 0} );
    tdata += 1.0;
  }
  plot.add_curve( std::move(c_tar) );
  plot.add_curve( std::move(c_learn) );
  plot.add_curve( std::move(c_test) );

  plot.show();
}
// ***************************************************************************
// ********************************************************************** test
// ***************************************************************************
void test()
{
    // Create a new HMM
  Problem _pb = create_hmm( "ABCDCB" );
  bica::sampler::HMM hmm1(_pb.t,_pb.o);
  std::cout << "__" << _pb.expr << "__ with " << _pb.nb_states << " states" << std::endl;  
  // print_hmm("h1",hmm1,30);
  // // Save to JSON
  // save_hmm( "tmp_hmm.json", _pb.expr );
  // // Reload from JSON
  // // first create another one
  // _pb = create_hmm( "AAAF" );
  // bica::sampler::HMM hmm2(_pb.t,_pb.o);
  // std::cout << "__" << _pb.expr << "__ with " << _pb.nb_states << " states" << std::endl;  
  // print_hmm("h2",hmm2,30);
  // // then load
  // _pb = load_hmm( "tmp_hmm.json" );
  // bica::sampler::HMM hmm3(_pb.t,_pb.o);
  // std::cout << "__" << _pb.expr << "__ with " << _pb.nb_states << " states" << std::endl;  
  // print_hmm("h3",hmm3,30);

  // // Create and save trajectory
  // Traj _data = create_traj( 100, _pb );
  // save_traj( "tmp_traj.json", _data, _pb.expr );
  // // Read Trajectory
  // _data.clear();
  // _data = load_traj( "tmp_traj.json" );
  // for (auto it = _data.begin(); it != _data.begin()+10; ++it) {
  //   std::cout << "s=" << (*it).id_s << "\to=" << (*it).id_o << std::endl;    
  // }

  // Create and save ESN
  ESN _esn = create_esn( 2, 3, 5, true, true, 4);
  save_esn( "tmp_esn.json", _esn );
  ESN esn_read = load_esn( "tmp_esn.json" );
  std::cout << "__READ RES: " << esn_read.res->str_display();
  std::cout << " LAY: " << esn_read.lay->str_display() << std::endl;
};
// ***************************************************************************
// ********************************************************************** main
// ***************************************************************************
int main(int argc, char *argv[])
{
   setup_options( argc, argv );
   
   // HMM _______________________
   if( _opt_expr ) {
     if( _opt_verb )
       std::cout << "__CREATE HMM with " << *_opt_expr << std::endl;
     _pb = make_unique<Problem>( create_hmm( *_opt_expr ));
     bica::sampler::HMM hmm1(_pb->t,_pb->o);
     if( _opt_verb )
       std::cout << "__" << _pb->expr << "__ with " << _pb->nb_states << " states" << std::endl;
   }
   if( _opt_fileload_hmm ) {
    if( _opt_verb )
      std::cout << "__LOAD HMM from " << *_opt_fileload_hmm << std::endl;
     _pb = make_unique<Problem>( load_hmm( *_opt_fileload_hmm ));
     bica::sampler::HMM hmm1(_pb->t,_pb->o);
     if( _opt_verb )
       std::cout << "__" << _pb->expr << "__ with " << _pb->nb_states << " states" << std::endl;
   }
   if( _pb and _opt_filesave_hmm ) {
     if( _opt_verb )
       std::cout << "__SAVE HMM to " << *_opt_filesave_hmm << std::endl;
     save_hmm( *_opt_filesave_hmm, _pb->expr );
   }

   // Traj_______________________
   if( _pb and _opt_filesave_traj ) {
     if( _opt_verb )
       std::cout << "__CREATE Traj of length " << _opt_traj_length << " with hmm=" << _pb->expr << std::endl;
     _data = make_unique<Traj>( create_traj( _opt_traj_length, *_pb ));
     if( _opt_verb )
       std::cout << "__SAVE Traj to " << *_opt_filesave_traj << std::endl;
     save_traj( *_opt_filesave_traj, *_data, _pb->expr );
   }
   if( _opt_fileload_traj ) {
     if( _opt_verb )
       std::cout << "__LOAD Traj from " << *_opt_fileload_traj << std::endl;
     _data = make_unique<Traj>( load_traj( *_opt_fileload_traj ) );
   }

   // ESN ________________________
   if( _opt_filesave_esn ) {
     if( _opt_verb )
       std::cout << "__CREATE ESN " << std::endl;
     _esn = make_unique<ESN>( create_esn(1+1, 1, _opt_res_size, _opt_res_forward,
					 _opt_res_szita, _opt_res_szita_val,
					 _opt_res_scaling, _opt_res_radius, _opt_res_leak) );
     if( _opt_verb )
       std::cout << "__SAVE ESN to " << *_opt_filesave_esn << std::endl;
     save_esn( *_opt_filesave_esn, *_esn);
   }
   if( _opt_fileload_esn ) {
     if( _opt_verb )
       std::cout << "__LOAD ESN from " << *_opt_fileload_esn << std::endl;
     _esn = make_unique<ESN>( load_esn( *_opt_fileload_esn ));
   }

   // WNoise_______________________
   if( _opt_filesave_noise ) {
     if( _opt_verb )
       std::cout << "__CREATE and SAVE WNoise to " << *_opt_filesave_noise << std::endl;
     _noise = make_unique<WNoise::Data>( save_noise( *_opt_filesave_noise,
						     _opt_noise_length, _opt_noise_level,
						     1 /*dimension*/ )
					 );
   }
   if( _opt_fileload_noise ) {
     if( _opt_verb ) {
       std::cout << "__LOAD WNoise from " << *_opt_fileload_noise << std::endl;
     }
     _noise = make_unique<WNoise::Data>( load_noise( *_opt_fileload_noise ));
   }
   
  // Learn_________________________
  if( _pb and _data and _esn ) {
    if( _opt_verb )
      std::cout << "__LEARN" << std::endl;
    if( _noise ) {
      if( _opt_verb )
	std::cout << "  + WNoise" << std::endl;
      push_noise( *_esn, _noise->begin(), _noise->end() );
    }
    // Compute and save RES internal state (ie. layer input)
    _data_lay_in = compute_lay_input( _data->begin(), _data->end(), *_esn );
    // DEBUG
    //std::cout << "       RES: " << _esn->res->str_display();
    //std::cout << " LAY: " << _esn->lay->str_display() << std::endl;
    learn( *_esn,
	   _data_lay_in.begin(), _data_lay_in.end()-1-_opt_test_length, // input
	   _data->begin()+1, _data->end()-_opt_test_length,             // target
	   _opt_regul );

    // Erreur d'apprentissage
    std::vector<RidgeRegression::Toutput> result_learn;
    for( const auto& pred_in: _data_lay_in) {
      auto pred_out = _esn->lay->forward( pred_in );
      //std::cout << "IN: " << utils::str_vec(pred_in);
      //std::cout << " --> " << utils::str_vec(pred_out) << std::endl;
      result_learn.push_back( pred_out );
    }
    
    // to file ____________________
    auto idx_out = 0;
    //DEBUG
    // for (auto it = _data->begin(); it != _data->end()-1; ++it) {
    //   std::cout << "IN: " << (*_data)[idx_out].id_o ;
    //   std::cout << " TAR: " << (*_data)[idx_out+1].id_o;
    //   std::cout << " OUT: " << utils::str_vec(result_learn[idx_out]) << std::endl;

    //   idx_out ++;
    // }
    if( _opt_file_result ) {
       // Results on learn
       std::stringstream filename_learn;
       filename_learn << *_opt_file_result;
       filename_learn << "_learn";
       auto ofile = std::ofstream( filename_learn.str() );

       // Header comments
       ofile << "## \"hmm_exp\": \"" << _pb->expr << "\"," << std::endl;
       ofile << "## \"traj_name\" : \"" << *_opt_fileload_traj << "\"," << std::endl;
       ofile << "## \"esn_name\": \"" << *_opt_fileload_esn << "\"," << std::endl;
       ofile << "## \"regul\": " << _opt_regul << "," << std::endl;
       ofile << "## \"test_length\": " << _opt_test_length << "," << std::endl;
       // Header ColNames
       // target
       for( unsigned int i = 0; i < _esn->lay->output_size(); ++i) {
     	ofile << "ta_" << i << "\t";
       }
       // after learn
       for( unsigned int i = 0; i < _esn->lay->output_size(); ++i) {
     	ofile << "le_" << i << "\t";
       }
       ofile << std::endl;

       // Data
       idx_out = 0;
       for (auto it = _data->begin()+1; it != _data->end()-_opt_test_length; ++it) {
	 // TAR
	 ofile << it->id_o << "\t";
	 // RES
	 for( auto& var: result_learn[idx_out]) {
	   ofile << var << "\t";
	 }
	 ofile << std::endl;
	 
	 idx_out++;
       }
	 
       ofile.close();

       // Results on test
       std::stringstream filename_test;
       filename_test << *_opt_file_result;
       filename_test << "_test";
       ofile = std::ofstream( filename_test.str() );

       // Header comments
       ofile << "## \"hmm_exp\": \"" << _pb->expr << "\"," << std::endl;
       ofile << "## \"traj_name\" : \"" << *_opt_fileload_traj << "\"," << std::endl;
       ofile << "## \"esn_name\": \"" << *_opt_fileload_esn << "\"," << std::endl;
       ofile << "## \"regul\": " << _opt_regul << "," << std::endl;
       ofile << "## \"test_length\": " << _opt_test_length << "," << std::endl;
       // Header ColNames
       // target
       for( unsigned int i = 0; i < _esn->lay->output_size(); ++i) {
     	ofile << "ta_" << i << "\t";
       }
       // after learn
       for( unsigned int i = 0; i < _esn->lay->output_size(); ++i) {
     	ofile << "le_" << i << "\t";
       }
       ofile << std::endl;

       // Data
       // Keep going one with the next values of idx_out
       for (auto it = _data->end()-_opt_test_length; it != _data->end(); ++it) {
	 // TAR
	 ofile << it->id_o << "\t";
	 // RES
	 for( auto& var: result_learn[idx_out]) {
	   ofile << var << "\t";
	 }
	 ofile << std::endl;
	 
	 idx_out++;
       }
	 
       ofile.close();
    }

    // graphic
    if( _opt_graph ) {
      graph( "Predict O_{t+1} = f( h(o_i) )",
	     _data->begin()+1, _data->end(),
	     result_learn.begin(), result_learn.end(),
	     _opt_test_length );
    }
    
  }
  
  return 0;

}
// ***************************************************************************



