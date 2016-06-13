/* -*- coding: utf-8 -*- */

/** 
 * WP with HMM (cmp with BICA's Paper de Jeremy and Herve
 *
 * - generate/save/load HMM : expr
 * - generate Trajectories of (S,O). inspi from Trajectory
 * - save/load Trajectories
 * - create ESN : two initialisation options
 * - save/load ESN
 *
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
std::unique_ptr<Traj>    _learn_data = nullptr;

// ESN
using PtrReservoir =   std::unique_ptr<Reservoir>;
using PtrLayer     = std::unique_ptr<Layer>;
using ESN = struct {
  PtrReservoir      res = nullptr;
  PtrLayer          lay = nullptr;
  bool    input_forward = false;
};
std::unique_ptr<ESN>     _esn;

// Options
std::unique_ptr<std::string> _opt_expr               = nullptr;
std::unique_ptr<std::string> _opt_filesave_hmm       = nullptr;
std::unique_ptr<std::string> _opt_fileload_hmm       = nullptr;
unsigned int                 _opt_traj_length;
std::unique_ptr<std::string> _opt_filesave_traj      = nullptr;
std::unique_ptr<std::string> _opt_fileload_traj      = nullptr;
Reservoir::Toutput_size      _opt_res_size;
double                       _opt_res_scaling;
double                       _opt_res_radius;
double                       _opt_res_leak;
bool                         _opt_res_forward;
bool                         _opt_res_szita;
double                       _opt_res_szita_val;
std::unique_ptr<std::string> _opt_filesave_esn       = nullptr;
std::unique_ptr<std::string> _opt_fileload_esn       = nullptr;


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

    ("length_traj", po::value<unsigned int>(&_opt_traj_length)->default_value(10),
     "create a Traj with length")
    ("save_traj", po::value<std::string>(), "save Traj in filename")
    ("load_traj,t", po::value<std::string>(), "load Traj from filename")

    ("res_size", po::value<unsigned int>(&_opt_res_size)->default_value(10), "reservoir size")
    ("res_forward", po::value<bool>(&_opt_res_forward)->default_value(false), "reservoir input forwarding")
    ("res_szita_init", po::value<bool>(&_opt_res_szita)->default_value(false), "reservoir: use szita initialisation")
    ("res_szita_val", po::value<double>(&_opt_res_szita_val)->default_value(5.0), "reservoir: value for init weights")
    ("res_scaling", po::value<double>(&_opt_res_scaling)->default_value(1.0), "reservoir input scaling")
    ("res_radius", po::value<double>(&_opt_res_radius)->default_value(0.99), "reservoir spectral radius")
    ("res_leak", po::value<double>(&_opt_res_leak)->default_value(0.1), "reservoir leaking rate")
    ("save_esn", po::value<std::string>(), "save ESN in filename")
    ("load_esn,e", po::value<std::string>(), "load ESN from filename")
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
    esn.lay = make_unique<Layer>( input_size+reservoir_size+1, output_size );
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
  // Traj _learn_data = create_traj( 100, _pb );
  // save_traj( "tmp_traj.json", _learn_data, _pb.expr );
  // // Read Trajectory
  // _learn_data.clear();
  // _learn_data = load_traj( "tmp_traj.json" );
  // for (auto it = _learn_data.begin(); it != _learn_data.begin()+10; ++it) {
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
     std::cout << "__CREATE HMM with " << *_opt_expr << std::endl;
     _pb = make_unique<Problem>( create_hmm( *_opt_expr ));
     bica::sampler::HMM hmm1(_pb->t,_pb->o);
     std::cout << "__" << _pb->expr << "__ with " << _pb->nb_states << " states" << std::endl;
   }
   if( _opt_fileload_hmm ) {
     std::cout << "__LOAD HMM from " << *_opt_fileload_hmm << std::endl;
     _pb = make_unique<Problem>( load_hmm( *_opt_fileload_hmm ));
     bica::sampler::HMM hmm1(_pb->t,_pb->o);
     std::cout << "__" << _pb->expr << "__ with " << _pb->nb_states << " states" << std::endl;
   }
   if( _pb and _opt_filesave_hmm ) {
     std::cout << "__SAVE HMM to " << *_opt_filesave_hmm << std::endl;
     save_hmm( *_opt_filesave_hmm, _pb->expr );
   }

   // Traj_______________________
   if( _pb and _opt_filesave_traj ) {
     std::cout << "__CREATE Traj of length " << _opt_traj_length << " with hmm=" << _pb->expr << std::endl;
     _learn_data = make_unique<Traj>( create_traj( _opt_traj_length, *_pb ));
     std::cout << "__SAVE Traj to " << *_opt_filesave_traj << std::endl;
     save_traj( *_opt_filesave_traj, *_learn_data, _pb->expr );
   }
   if( _opt_fileload_traj ) {
     std::cout << "__LOAD Traj from " << *_opt_fileload_traj << std::endl;
     _learn_data = make_unique<Traj>( load_traj( *_opt_fileload_traj ) );
   }

   // ESN ________________________
   if( _opt_filesave_esn ) {
     std::cout << "__CREATE ESN " << std::endl;
     _esn = make_unique<ESN>( create_esn(1, 1, _opt_res_size, _opt_res_forward,
					 _opt_res_szita, _opt_res_szita_val,
					 _opt_res_scaling, _opt_res_radius, _opt_res_leak) );
     std::cout << "__SAVE ESN to " << *_opt_filesave_esn << std::endl;
     save_esn( *_opt_filesave_esn, *_esn);
   }
   if( _opt_fileload_esn ) {
     std::cout << "__LOAD ESN from " << *_opt_fileload_esn << std::endl;
     _esn = make_unique<ESN>( load_esn( *_opt_fileload_esn ));
   }
  return 0;
}
// ***************************************************************************



