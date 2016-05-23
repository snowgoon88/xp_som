/* -*- coding: utf-8 -*- */

#ifndef DYN_PROG_HPP
#define DYN_PROG_HPP

/** 
 * Dynamic Progamming Algorithmes on (PO)MDPs.
 */

#include <iostream>                  // std::cout
#include <algorithm>                 // std::max_element
#include <limits>                    // max dbl

namespace Algorithms
{

typedef std::vector<std::vector<double>> TVal;

// ****************************************************************** comput_Q
/** 
 * Compute the Q value for the states of a given POMDP.
 * Iterates until the square of the Bellman Error is below `epsilon`
 * 
 * @param pomdp : a POMDP
 * @param gamma : discount factor for V
 * @param epsilon : precision for iteration
 * 
 * @return vQ : a TVal representation of the value-function
 */
TVal compute_Q( const Model::POMDP& pomdp,
		const double gamma = 0.9,
		const double epsilon = 0.1
		)
{
  TVal vQ;

  // Initialisation à 0.0
  for( unsigned int ids = 0; ids < pomdp._states.size(); ++ids) {
    std::vector<double> vQ_s;
    for( unsigned int ida = 0; ida < pomdp._actions.size(); ++ida) {
      vQ_s.push_back( 0.0 );
    }
    vQ.push_back( vQ_s );
  }
  // DEBUG
  std::cout << "___ INIT " << std::endl;
  for( auto& s: pomdp._states) {
    for( auto& a: pomdp._actions ) {
      std::cout << "Q["<< s._id << ", " << a._id << "]=" << vQ[s._id][a._id] << std::endl;
    }
  }
  
  // Iterate until convergence (< epsilon)
  double bellman_residual = std::numeric_limits<double>::max();
  TVal new_vQ = vQ;

  unsigned int num_ite = 0;
  while(  bellman_residual > epsilon ) {
    bellman_residual = 0.0;
    new_vQ = vQ;
    
    for( auto& s: pomdp._states) {
      for( auto& a: pomdp._actions) {
	double newQ = 0.0;
	Model::Transition pr_sa = pomdp._trans[s._id][a._id];
	for( auto& sprim: pomdp._states) {
	  // Trouver le max de vQ(sprim,.)
	  double maxQ = *std::max_element( vQ[sprim._id].begin(),
					   vQ[sprim._id].end() );
	  newQ += pr_sa._proba[sprim._id] * maxQ;
	}
	// std::cout << "("<<s._id<<","<<a._id<<")" << " newQ="<< newQ << std::endl;
	// r(s,a) + \gamma * E[maxQ]
	double newValQ = pomdp._reward[s._id] + gamma * newQ; 
	bellman_residual += (vQ[s._id][a._id] - newValQ)
	  * (vQ[s._id][a._id] - newValQ);
	new_vQ[s._id][a._id] = newValQ;
      }
    }
    vQ = new_vQ;
    // DEBUG
    std::cout << "___ STEP " << num_ite++ << std::endl;
    for( auto& s: pomdp._states) {
      for( auto& a: pomdp._actions ) {
	std::cout << "Q["<< s._id << ", " << a._id << "]=" << vQ[s._id][a._id] << std::endl;
      }
    }
    std::cout << "BE= " << bellman_residual << std::endl;
  }

  return vQ;
};

}; // namespace Algorithms

#endif // DYN_PROG_HPP
