/* -*- coding: utf-8 -*- */

/** 
 * WP with HMM (cmp with BICA's Paper de Jeremy and Herve
 *
 * - generate/save/load HMM : expr
 * - generate Trajectories of (S,O). inspi from Trajectory
 * - TODO save Trajectories
 * - TODO Load Trajectories
 *
 * TODO : make function that don't use globals...
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

#include <utils.hpp>                  // various str_xxx
using namespace utils::rj;
// ******************************************************************** Global
// HMM
std::string _expr = "ABCD";
bica::hmm::T _t;
bica::hmm::O _o;
unsigned int _nb_states;

// Trajectory
Trajectory::HMM::Data _learn_data;

// **************************************************************** create_hmm
void create_hmm( const std::string& expr = "ABCD" )
{
  _expr = expr;
  auto hmm = bica::hmm::make(expr);
  std::tie(_t, _o) = hmm.first;
  _nb_states = hmm.second;
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
void load_hmm( const std::string& filename )
{
  auto pfile = std::ifstream( filename );

  // Wrapper pour lire document
  JSON::IStreamWrapper instream(pfile);
  // Parse into a document
  rj::Document read_doc;
  read_doc.ParseStream( instream );
  pfile.close();

  auto expr = bica::hmm::unserialize( read_doc["hmm"] );
  create_hmm( expr );
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
void create_traj( const unsigned int length )
{
  // Sampler pour le HMM
  bica::sampler::HMM hmm(_t,_o);

  _learn_data.clear();
  for( unsigned int i = 0; i < length; ++i) {
    _learn_data.push_back( Trajectory::HMM::Item{hmm.input_id(), hmm.input()} );
    hmm.shift();
  }
};
// ***************************************************************** save_traj
void save_traj( const std::string& filename,
		const Trajectory::HMM::Data& data,
		const std::string hmm_expr)
{
  auto ofile = std::ofstream( filename );

  // header
  ofile << "## \"hmm_expr\": \"" << hmm_expr << "\"," << std::endl;

  Trajectory::HMM::save( ofile, data );
  
  ofile.close();
};
// ***************************************************************** load_traj
 void load_traj( const std::string& filename )
{
  auto pfile = std::ifstream( filename );
  Trajectory::HMM::read( pfile, _learn_data );
  pfile.close();
};
// ***************************************************************************
// ********************************************************************** main
// ***************************************************************************
int main(int argc, char *argv[])
{
  // Create a new HMM
  create_hmm( "ABCDCB" );
  bica::sampler::HMM hmm1(_t,_o);
  std::cout << "__" << _expr << "__ with " << _nb_states << " states" << std::endl;  
  // print_hmm("h1",hmm1,30);
  // // Save to JSON
  // save_hmm( "tmp_hmm.json", _expr );
  // // Reload from JSON
  // // first create another one
  // create_hmm( "AAAF" );
  // bica::sampler::HMM hmm2(_t,_o);
  // std::cout << "__" << _expr << "__ with " << _nb_states << " states" << std::endl;  
  // print_hmm("h2",hmm2,30);
  // // then load
  // load_hmm( "tmp_hmm.json" );
  // bica::sampler::HMM hmm3(_t,_o);
  // std::cout << "__" << _expr << "__ with " << _nb_states << " states" << std::endl;  
  // print_hmm("h3",hmm3,30);

  // Create and save trajectory
  create_traj( 100 );
  save_traj( "tmp_traj.json", _learn_data, _expr );
  // Read Trajectory
  _learn_data.clear();
  load_traj( "tmp_traj.json" );
  // for (auto it = _learn_data.begin(); it != _learn_data.begin()+10; ++it) {
  //   std::cout << "s=" << (*it).id_s << "\to=" << (*it).id_o << std::endl;    
  // }
  

  return 0;
}
// ***************************************************************************



