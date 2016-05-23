/* -*- coding: utf-8 -*- */

/** 
 * Teste Model::POMDP.
 *  - Node : creation, affichage
 *  - Transition : creation, affichage, get_next
 *  - POMDP : creation, affichage, simulation, read/write
 */

#include <iostream>                  // std::cout
#include <fstream>                 // std::ofstream
// #include <string>                  // std::string
#include "rapidjson/document.h"         // rapidjson's DOM-style API
#include <json_wrapper.hpp>          // JSON::OStreamWrapper, JSON::IStreamWrapper

#include <pomdp/pomdp.hpp>
#include <pomdp/prog_dynamique.hpp>

#include <gsl/gsl_rng.h>             // gsl random generator
#include <ctime>                     // std::time

#include <utils.hpp>                  // various str_xxx
using namespace utils::rj;

// ***************************************************************************
#define FILENAME "pomdp_save.json"

//******************************************************************************
int main( int argc, char *argv[] )
{
  Model::Node node = {0, "zero"};
  std::cout << "node = " << node.str_dump() << std::endl;

  Model::Transition trans = {{0.1, 0.3, 0.6}};
  std::cout << "trans = " << trans.str_dump() << std::endl;

  gsl_rng* rnd = gsl_rng_alloc( gsl_rng_taus );
  gsl_rng_set( rnd, std::time( NULL ) );
  double p = gsl_rng_uniform_pos(rnd);
  std::cout << "p=" << p << " : " << trans.get_next(p) << std::endl;

  Model::POMDP pomdp = {{{0, "zero"}, {1, "un"}},
			{{0, "0"}},
			{{0, "Up"}, {1, "Right"}, {2, "Down"}},
			{{ {{0.1, 0.9}}, {{0.5, 0.5}}, {{1.0, 0.0}}},
			 { {{0.9, 0.1}}, {{1.0, 0.0}}, {{0.3, 0.7}} }
			},
			{ {{1.0}}, {{1.0}} },
			{{-1, 10.0}}
  };
  std::cout << "POMDP *******" << std::endl << pomdp.str_dump() << std::endl;
  std::cout << "CURRENT : " << pomdp.str_state() << std::endl;


  // Serialisation dans FILENAME
  std::cout << "SERIALIZE " << std::endl;
  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember( "pomdp", pomdp.serialize(doc), doc.GetAllocator());

  // Write to file
  std::cout << "   => write to file" << std::endl;
  std::ofstream ofile( FILENAME );
  ofile << str_obj(doc) << std::endl;
  ofile.close();

  // Read from file
  std::cout << "UNSERIALZE" << std::endl;
  std::ifstream ifile( FILENAME );
  // Wrapper pour lire document
  JSON::IStreamWrapper instream(ifile);
  // Parse into a document
  rj::Document read_doc;
  read_doc.ParseStream( instream );
  ifile.close();

  Model::POMDP read_pomdp( read_doc["pomdp"] );
  std::cout << "READ POMDP *******" << std::endl << read_pomdp.str_dump() << std::endl;

  // Essai d'action
  std::cout << "SIMULATION" << std::endl;
  const std::vector<Model::Node>& list_action = pomdp.actions();

  std::cout << "CURRENT : " << pomdp.str_state() << " r= " << pomdp.cur_reward() << std::endl;
  
  // Transition
  for( unsigned int i = 0; i < 5; ++i) {
    // Choix d'une action aléatoire
    const Model::Node& act = list_action[gsl_rng_uniform_int( rnd, list_action.size() )];
    std::cout << "   +ACT : " << act.str_dump() << std::endl;
    pomdp.simul_trans( act );
    std::cout << "CURRENT : " << pomdp.str_state() << " r= " << pomdp.cur_reward() << std::endl;
  }

  // Copy
  Model::POMDP p1(pomdp);
  pomdp._states[0] = {0,"changed"};
  pomdp._reward[1] = 12;
  Model::Transition t;
  t._proba = {0.5, 0.5};
  pomdp._percep[1] = t;
  pomdp._percep[0]._proba[0] = 0.8;
  pomdp._trans[0][0]._proba[0] = 0.666;
  Model::Transition t1;
  t1._proba = {0.666, 0.777};
  pomdp._trans[0][1] = t1;
  std::cout << "** ORIG \n" << pomdp.str_dump() << std::endl;
  std::cout << "** COPY \n" << p1.str_dump() << std::endl;
  
  Model::POMDP p2 = pomdp;
  pomdp._actions[1]._label = "Two";
  Model::Transition t3;
  t3._proba = {0.123, 0.456};
  pomdp._percep[1] = t3;
  pomdp._percep[0]._proba[0] = 0.789;
  pomdp._trans[0][1]._proba[0] = 0.222;
  Model::Transition t4;
  t4._proba = {0.111, 0.222};
  pomdp._trans[1][1] = t4;

  std::cout << "** ORIG \n" << pomdp.str_dump() << std::endl;
  std::cout << "** OPER \n" << p2.str_dump() << std::endl;

  // Tester les algorithmes
  Algorithms::TVal valQ = Algorithms::compute_Q( p1 );
  std::cout << "** QVal *****"  << std::endl;
  for( auto& s: pomdp._states) {
    for( auto& a: pomdp._actions ) {
      std::cout << "Q["<< s._id << ", " << a._id << "]=" << valQ[s._id][a._id] << std::endl;
    }
  }

  return 0;
}
