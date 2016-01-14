/* -*- coding: utf-8 -*- */

/** 
 * XP with POMDP and Reservoir Computing
 *
 * - load POMDP and generate Trajectories of S,O,A,S',O',R
 * TODO : store Trajectories
 * TODO : load Trajectories
 * TODO : try to learn O,A -> O' (??????)
 */

#include <iostream>                // std::cout
#include <fstream>                 // std::ofstream
#include <string>                  // std::string
#include <rapidjson/document.h>    // rapidjson's DOM-style API
#include <json_wrapper.hpp>        // JSON::IStreamWrapper

#include <pomdp/pomdp.hpp>
#include <pomdp/trajectory.hpp>
#include <gsl/gsl_rng.h>             // gsl random generator
#include <ctime>                     // std::time

// Parsing command line options
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <utils.hpp>                  // various str_xxx
using namespace utils::rj;
// ********************************************************************* param
std::string*           _filename_pomdp = nullptr;
std::string*           _fileload_traj = nullptr;
std::string*           _filegene_traj = nullptr;

Model::POMDP*          _pomdp = nullptr;
unsigned int           _length;
Trajectory::POMDP::Data _traj_data;

gsl_rng*               _rnd = gsl_rng_alloc( gsl_rng_taus );
// ****************************************************************** free_mem
void free_mem()
{
  if( _pomdp ) delete _pomdp;
}
// ******************************************************************* options
void setup_options(int argc, char **argv)
{
  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "produce help message")
    ("length,l", po::value<unsigned int>(&_length)->default_value(10), "generate Traj of length ")
    ("load_pomdp,p", po::value<std::string>(), "load POMDP from JSON file")
    ("load_traj,t", po::value<std::string>(), "load Trajectory from file")
    ("gene_traj,f", po::value<std::string>(), "gene Trajectory into file")
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
}
// ********************************************** load and generate Trajectory
void load_and_generate()
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

  std::ofstream* ofile = nullptr;
  if( _filegene_traj ) {
    ofile = new std::ofstream( *_filegene_traj + ".data" );
  }
  
  // Generate
  gsl_rng_set( _rnd, std::time( NULL ) );
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
// ********************************************************************** main
int main( int argc, char *argv[] )
{
  setup_options( argc, argv );

  if( _filename_pomdp) {
    load_and_generate();
  }

  if( _fileload_traj ) {
    std::cout << "** Load Trajectory::POMDP::Data in " << *_fileload_traj << std::endl;
    read_traj( *_fileload_traj );

    std::cout << "** Trajectory Read" << std::endl;
    for( auto& item: _traj_data) {
      std::cout << item.id_s << ":" << item.id_o << "+" << item.id_a << "->" << item.id_next_s << ":" << item.id_next_o << " = " << item.r << std::endl;
    }
  }
  
  free_mem();
  return 0;
}