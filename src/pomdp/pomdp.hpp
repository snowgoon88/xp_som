/* -*- coding: utf-8 -*- */

#ifndef POMDP_HPP
#define POMDP_HPP

/** 
 * Outils pour utiliser des POMDP
 * - Charger/Lire
 * - Simuler 
 */

#include <string>                     // std::string
#include <sstream>                   // std::stringstream
#include <vector>                     // std::vector

#include <gsl/gsl_rng.h>             // gsl random generator
#include <ctime>                     // std::time

// ********************************************************************* Model
namespace Model
{
// ***************************************************************************
// ********************************************************************** Node
// ***************************************************************************
struct Node
{
  unsigned int _id;
  std::string _label;
  std::string str_dump() const { 
    return std::to_string(_id)+std::string(":")+_label;
  }
};
// ***************************************************************************
// **************************************************************** Transition
// ***************************************************************************
/**
 * Transition est un tableau des probas de transiter
 * vers un autre noeud.
 */ 
class Transition
{
public:
  std::vector<double> _proba;
  // **************************************************** Transition::get_next
  unsigned int get_next( double random )
  {
    double sum_proba = 0;
    for( unsigned int i = 0; i < _proba.size(); ++i) {
      sum_proba += _proba[i];
      if( random <= sum_proba ) {
	return i;
      }
    }
    return _proba.size()-1;
  };
  // ******************************************************** Transistion::str
  std::string str_dump() const
  {
    std::stringstream dump;
    dump << "[";
    for( auto& prob: _proba) {
      dump << prob << "; ";
    }
    dump << "]";
    return dump.str();
  };
};
// ***************************************************************************
// ********************************************************************* POMDP
// ***************************************************************************
class POMDP
{
public:
  // ********************************************************* POMDP::creation
  POMDP( const std::vector<Node>& states,
	 const std::vector<Node>& obs,
	 const std::vector<Node>& actions,
	 const std::vector<std::vector<Transition>>& trans,
	 const std::vector<Transition>& percep,
	 const std::vector<double>& reward
	 ) :
    _states(states), _obs(obs), _actions(actions),
    _trans(trans), _percep(percep), _reward(reward),
    _rnd(nullptr),
    _cur_state(states[0]), _cur_obs()
  {
    // Générateur Aléatoire
    _rnd = gsl_rng_alloc( gsl_rng_taus );
    gsl_rng_set( _rnd, std::time( NULL ) );
    // obs
    simul_obs();
  };
  // ************************************************************** POMDP::str
  /** dump state */
  std::string str_state () const
  {
    std::stringstream state;
    state << cur_state().str_dump() << "[" << cur_obs().str_dump() << "]";
    
    return state.str();
  }
  /** dump model */
  std::string str_dump() const
  {
    std::stringstream dump;
    dump << "STATES : ";
    for( auto& s: _states) {
      dump << s.str_dump() << "; ";
    }
    dump << std::endl;
    dump << "OBS    : ";
    for( auto& o: _obs) {
      dump << o.str_dump() << "; ";
    }
    dump << std::endl;
    dump << "ACTIONS: ";
    for( auto& a: _actions) {
      dump << a.str_dump() << "; ";
    }
    dump << std::endl;
    dump << "TRANSITIONS : " << std::endl;
    for( auto& s: _states) {
      for( auto& a: _actions) {
	dump << "  T(" << s._label << "; " << a._label << ") = " << _trans[s._id][a._id].str_dump() << std::endl; 
      }
    }
    dump << "PERCEPTION : " << std::endl;
    for( auto& s: _states) {
      dump << "  O(" << s._label << ") =" << _percep[s._id].str_dump() << std::endl;
    }
    dump << "REWARD : " << std::endl;
    for( auto& s: _states) {
      dump << "  R(" << s._label << ") =" << _reward[s._id] << std::endl;
    }
    
    return dump.str();
  };
  // ************************************************************ POMDP::simul
  const Node& simul_trans(const Node& act)
  {
    // std::cout << "simul_trans from " << _cur_state.str_dump();
    // std::cout << " + " << act.str_dump();
    // std::cout << std::endl;
    double p = gsl_rng_uniform_pos(_rnd);
    _cur_state = _states[_trans[_cur_state._id][act._id].get_next(p)];
    return _cur_state;
  };
  const Node& simul_obs()
  {
    double p = gsl_rng_uniform_pos(_rnd);
    _cur_obs = _obs[_percep[_cur_state._id].get_next(p)];
    return _cur_obs;
  };
  // ************************************************ POMDP::set_current_state
  void set_current_state( unsigned int id_state )
  {
    _cur_state = _states[id_state];
  };
  // ******************************************************** POMDP::attributs
  const Node& cur_state() const { return _cur_state; };
  const Node& cur_obs() const { return _cur_obs; };
  double cur_reward() const { return _reward[_cur_state._id]; };
  const std::vector<Node>& actions() const { return _actions; };
  /** Le Modèle */
  std::vector<Node> _states;
  std::vector<Node> _obs;
  std::vector<Node> _actions;
  std::vector<std::vector<Transition>> _trans;
  std::vector<Transition> _percep;
  std::vector<double> _reward;
  /** Random Generator */
  gsl_rng* _rnd;
  /** Etat */
  Node _cur_state, _cur_obs;
};
  
};


#endif // POMDP_HPP
