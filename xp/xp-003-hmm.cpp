/* -*- coding: utf-8 -*- */

/** 
 * WP with HMM (cmp with BICA's Paper de Jeremy and Herve
 *
 * - generate/save/load HMM : expr
 * - TODO generate Trajectories of (S,O). inspi from History ??
 * - TODO save Trajectories
 * - TODO Load Trajectories
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

#include <utils.hpp>                  // various str_xxx
using namespace utils::rj;
// ******************************************************************** Global

std::string _expr = "ABCD";
bica::hmm::T _t;
bica::hmm::O _o;
unsigned int _nb_states;

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
// ***************************************************************************
// ********************************************************************** main
// ***************************************************************************
int main(int argc, char *argv[])
{
  // Create a new HMM
  create_hmm( "ABCDCB" );
  bica::sampler::HMM hmm1(_t,_o);
  std::cout << "__" << _expr << "__ with " << _nb_states << " states" << std::endl;  
  print_hmm("h1",hmm1,30);
  // Save to JSON
  save_hmm( "tmp_hmm.json", _expr );
  // Reload from JSON
  // first create another one
  create_hmm( "AAAF" );
  bica::sampler::HMM hmm2(_t,_o);
  std::cout << "__" << _expr << "__ with " << _nb_states << " states" << std::endl;  
  print_hmm("h2",hmm2,30);
  // then load
  load_hmm( "tmp_hmm.json" );
  bica::sampler::HMM hmm3(_t,_o);
  std::cout << "__" << _expr << "__ with " << _nb_states << " states" << std::endl;  
  print_hmm("h3",hmm3,30);
  
  
  return 0;
}
// ***************************************************************************



