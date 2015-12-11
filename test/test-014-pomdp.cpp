/* -*- coding: utf-8 -*- */

/** 
 * Teste Model::POMDP.
 *  - Node : creation, affichage
 *  - Transition : creation, affichage, get_next
 *  - POMDP : creation, affichage, simulation
 */

#include <iostream>                  // std::cout

#include <pomdp.hpp>
#include <gsl/gsl_rng.h>             // gsl random generator
#include <ctime>                     // std::time
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

  // Essai d'action
  const std::vector<Model::Node>& list_action = pomdp.actions();
  // Transation
  for( unsigned int i = 0; i < 5; ++i) {
    // Choix d'une action aléatoire
    const Model::Node& act = list_action[gsl_rng_uniform_int( rnd, list_action.size() )];
    pomdp.simul_trans( act );
    std::cout << "CURRENT : " << pomdp.str_state() << " r= " << pomdp.cur_reward() << std::endl;
  }
  return 0;
}
