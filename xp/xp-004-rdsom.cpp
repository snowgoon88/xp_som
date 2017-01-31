/* -*- coding: utf-8 -*- */

/** 
 * XP with recurrent DSOM.
 *  - load HMM
 *  - load Traj
 *  - create and save DSOM
 *  - load DSOM
 *  - TODO run XP
 *  - TODO graphic
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
#include <dsom/r_network.hpp>

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

// RDSOM
using RDSOM = Model::DSOM::RNetwork;
std::unique_ptr<RDSOM>     _rdsom;

// ******************************************************************* Options
// Options
std::unique_ptr<std::string> _opt_fileload_hmm       = nullptr;
std::unique_ptr<std::string> _opt_fileload_traj      = nullptr;
int                          _opt_rdsom_size           = 10;
std::unique_ptr<std::string> _opt_filesave_rdsom     = nullptr;
std::unique_ptr<std::string> _opt_fileload_rdsom     = nullptr;
bool                         _opt_verb               = false;

// ***************************************************************************
// ******************************************************************* options
// ***************************************************************************
void setup_options(int argc, char **argv)
{
  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "produce help message")
	("load_hmm,m", po::value<std::string>(), "load HMM from filename")
	("load_traj,t", po::value<std::string>(), "load Traj from filename")
	("rdsom_size", po::value<int>(&_opt_rdsom_size)->default_value(_opt_rdsom_size),"rdsom size")
	("save_rdsom", po::value<std::string>(), "save RDSOM in filename")
    ("load_rdsom,d", po::value<std::string>(), "load RDSOM from filename")
	("verb,v", "verbose" )
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
  if (vm.count("load_hmm")) {
    _opt_fileload_hmm = make_unique<std::string>(vm["load_hmm"].as< std::string>());
  }
   // TRAJ
  if (vm.count("load_traj")) {
    _opt_fileload_traj = make_unique<std::string>(vm["load_traj"].as< std::string>());
  }
  // RDSOM
  if (vm.count("save_rdsom")) {
    _opt_filesave_rdsom = make_unique<std::string>(vm["save_rdsom"].as< std::string>());
  }
  if (vm.count("load_rdsom")) {
    _opt_fileload_rdsom = make_unique<std::string>(vm["load_rdsom"].as< std::string>());
  }

  // Options
  if( vm.count("verb") ) {
    _opt_verb = true;
  }
}

// ****************************************************************** load_hmm
Problem create_hmm( const std::string& expr = "ABCD" )
{
  Problem pb;
  pb.expr = expr;
  auto hmm = bica::hmm::make(expr);
  std::tie(pb.t, pb.o) = hmm.first;
  pb.nb_states = hmm.second;

  return pb;
}Problem load_hmm( const std::string& filename )
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
}
// ***************************************************************** load_traj
Traj load_traj( const std::string& filename )
{
  Traj traj;
  auto pfile = std::ifstream( filename );
  Trajectory::HMM::read( pfile, traj );
  pfile.close();

  return traj;
}
// ************************************************************** create_rdsom
RDSOM create_rdsom(int input_dim=1, int rdsom_size=10, int rdsom_nb_link=-1,
				   float w_min=0.0, float w_max=1.0 )
{
  RDSOM rdsom = RDSOM( input_dim, rdsom_size, rdsom_nb_link,
				  w_min, w_max );
  return rdsom;
}
// **************************************************************** save_rdsom
void save_rdsom( const std::string& filename, const RDSOM& rdsom )
{
  auto ofile = std::ofstream( filename );

  rapidjson::Document doc;
  rapidjson::Value obj = _rdsom->serialize( doc );
  //std::cout << str_obj( obj ) << std::endl;
  // Write in a file
  ofile << str_obj( obj ) << std::endl;
  ofile.close();
}
RDSOM load_rdsom( const std::string& filename )
{
  std::ifstream ifile( filename );
  RDSOM net_read( ifile );
  ifile.close();

  return net_read;
}

// ***************************************************************************
// ********************************************************************* learn
// ***************************************************************************

// ***************************************************************************
// ********************************************************************** main
// ***************************************************************************
int main(int argc, char *argv[])
{
   setup_options( argc, argv );
   
   // HMM _______________________
   if( _opt_fileload_hmm ) {
    if( _opt_verb )
      std::cout << "__LOAD HMM from " << *_opt_fileload_hmm << std::endl;
     _pb = make_unique<Problem>( load_hmm( *_opt_fileload_hmm ));
     bica::sampler::HMM hmm1(_pb->t,_pb->o);
     if( _opt_verb )
       std::cout << "__" << _pb->expr << "__ with " << _pb->nb_states << " states" << std::endl;
   }
   // Traj_______________________
   if( _opt_fileload_traj ) {
     if( _opt_verb )
       std::cout << "__LOAD Traj from " << *_opt_fileload_traj << std::endl;
     _data = make_unique<Traj>( load_traj( *_opt_fileload_traj ) );
   }
   // RDSOM _____________________
   if( _opt_filesave_rdsom ) {
     if( _opt_verb )
       std::cout << "__CREATE RDSOM " << std::endl;
     _rdsom = make_unique<RDSOM>( create_rdsom(1, _opt_rdsom_size, -1,
											 0.0, 1.0 ));
	 if( _opt_verb )
       std::cout << "__SAVE RDSOM to " << *_opt_filesave_rdsom << std::endl;
     save_rdsom( *_opt_filesave_rdsom, *_rdsom);
   }
   if( _opt_fileload_rdsom ) {
     if( _opt_verb )
       std::cout << "__LOAD RDSOM from " << *_opt_fileload_rdsom << std::endl;
     _rdsom = make_unique<RDSOM>( load_rdsom( *_opt_fileload_rdsom ));

	 // test
	 std::cout << _rdsom->str_dump() << std::endl;
   }
   
   return 0;
}
