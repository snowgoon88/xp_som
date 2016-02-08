/* -*- coding: utf-8 -*- */

#ifndef POMDP_HPP
#define POMDP_HPP

/** 
 * Outils pour utiliser des POMDP
 * - Charger/Lire avec JSON
 * - Simuler 
 */

#include <string>                     // std::string
#include <sstream>                   // std::stringstream
#include <vector>                     // std::vector

#include <gsl/gsl_rng.h>             // gsl random generator
#include <ctime>                     // std::time

#include "rapidjson/prettywriter.h" // rapidjson
#include "rapidjson/document.h"     // rapidjson's DOM-style API
// #include <json_wrapper.hpp>         // JSON::OStreamWrapper et IStreamWrapper
namespace rj = rapidjson;

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
  rj::Value serialize( rj::Document& doc )
  {
    // rj::Object qui contient les données
    rj::Value rj_node;
    rj_node.SetObject();
    rj_node.AddMember( "id", rj::Value(_id), doc.GetAllocator() );
    rj_node.AddMember( "label", rj::StringRef(_label.c_str()), doc.GetAllocator() );
    
    return rj_node;
  }
  void unserialize( const rj::Value& obj )
  {
    _id = obj["id"].GetUint();
    _label = std::string( obj["label"].GetString() );
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
  // ********************************************************* Transition::str
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
  // *************************************************** Transition::serialize
  rj::Value serialize( rj::Document& doc )
  {
    // rj::Array qui contient les données
    rj::Value rj_trans;
    rj_trans.SetArray();
    for( auto& prob: _proba) {
      rj_trans.PushBack( prob, doc.GetAllocator());
    }
    
    return rj_trans;
  }
  // ************************************************* Transition::unserialize
  void unserialize( const rapidjson::Value& obj )
  {
    _proba.clear();
    const rj::Value& ar = obj;
    assert( ar.IsArray() );
    rapidjson::SizeType idx = 0;
    for( idx=0; idx < ar.Size(); ++idx ) {
      assert( ar[idx].IsNumber() );
      _proba.push_back( ar[idx].GetDouble() );
    }
  }
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
  POMDP( const rj::Value& obj ) :
    _states(), _obs(), _actions(),
    _trans(), _percep(), _reward(),
    _rnd(nullptr),
    _cur_state(), _cur_obs()
  {
    // decode d'après obj
    unserialize( obj );
    
    // Générateur Aléatoire
    _rnd = gsl_rng_alloc( gsl_rng_taus );
    gsl_rng_set( _rnd, std::time( NULL ) );

    // _cur_state(states[0])
    // obs
    // simul_obs();
  };
  // ************************************************************* POMDP::copy
  POMDP( const POMDP& other ) :
    _states(other._states), _obs(other._obs), _actions(other._actions),
    _trans(other._trans), _percep(other._percep), _reward(other._reward),
    _rnd(other._rnd),
    _cur_state(other.cur_state()), _cur_obs(other.cur_obs())
  {
  };
  POMDP& operator=(const POMDP& other)
  {
    if (this != &other) { // protect against invalid self-assignment
      _states = other._states;
      _obs = other._obs;
      _actions = other._actions;
      _trans = other._trans;
      _percep = other._percep;
      _reward = other._reward;
      _rnd = other._rnd;
      _cur_state = other.cur_state();
      _cur_obs = other.cur_obs();
    }
    return *this;
  }
  // ********************************************************** POMDP::destroy
  ~POMDP() 
  {
    if( _rnd ) gsl_rng_free( _rnd );
  }
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
  // ******************************************************** POMDP::serialize
  rj::Value serialize( rj::Document& doc )
  {
    // rj::Object qui contient les données
    rj::Value obj;
    obj.SetObject();

    // Ajoute les états sous forme d'array
    rj::Value ar_state;
    ar_state.SetArray();
    for( auto& s: _states) {
      ar_state.PushBack( s.serialize(doc), doc.GetAllocator() );
    }
    obj.AddMember( "states", ar_state, doc.GetAllocator() );
    // Ajoute les actions sous forme d'array
    rj::Value ar_act;
    ar_act.SetArray();
    for( auto& a: _actions) {
      ar_act.PushBack( a.serialize(doc), doc.GetAllocator() );
    }
    obj.AddMember( "actions", ar_act, doc.GetAllocator() );
    // Ajoute les observations sous forme d'array
    rj::Value ar_obs;
    ar_obs.SetArray();
    for( auto& o: _obs) {
      ar_obs.PushBack( o.serialize(doc), doc.GetAllocator() );
    }
    obj.AddMember( "obs", ar_obs, doc.GetAllocator() );

    // Les transitions
    rj::Value ar_trans;
    ar_trans.SetArray();
    for( auto& s: _states) {
      for( auto& a: _actions) {
	ar_trans.PushBack( _trans[s._id][a._id].serialize( doc ), doc.GetAllocator());
      }
    }
    obj.AddMember( "trans", ar_trans, doc.GetAllocator());
    // Les fonction de perception
    rj::Value ar_per;
    ar_per.SetArray();
    for( auto& s: _states) {
      ar_per.PushBack( _percep[s._id].serialize( doc ), doc.GetAllocator());
    }
    obj.AddMember( "perc", ar_per, doc.GetAllocator());

    // Les Reward
    rj::Value ar_rew;
    ar_rew.SetArray();
    for( auto& s: _states) {
      ar_rew.PushBack( _reward[s._id], doc.GetAllocator() );
    }
    obj.AddMember( "reward", ar_rew, doc.GetAllocator() );
    
    return obj;
  }
  void unserialize( const rapidjson::Value& obj )
  {
    // Array of states
    const rj::Value& ar_states = obj["states"];
    assert( ar_states.IsArray() );
    rj::SizeType idx = 0;
    for( idx=0; idx < ar_states.Size(); ++idx ) {
      Node n;
      n.unserialize( ar_states[idx] );
      _states.push_back( n );
    }
    // Array of actions
    const rj::Value& ar_actions = obj["actions"];
    assert( ar_actions.IsArray() );
    for( idx=0; idx < ar_actions.Size(); ++idx ) {
      Node n;
      n.unserialize( ar_actions[idx] );
      _actions.push_back( n );
    }
    // Array of obs
    const rj::Value& ar_obs = obj["obs"];
    assert( ar_obs.IsArray() );
    for( idx=0; idx < ar_obs.Size(); ++idx ) {
      Node n;
      n.unserialize( ar_obs[idx] );
      _obs.push_back( n );
    }

    // Transitions
    const rj::Value& ar_trans = obj["trans"];
    assert( ar_trans.IsArray() );
    idx = 0;
    for( unsigned int is = 0; is < _states.size(); ++is) {
      std::vector<Transition> v_tr_a;
      for( unsigned int ia = 0; ia < _actions.size(); ++ia) {
	Transition tr;
	tr.unserialize( ar_trans[idx] );
	v_tr_a.push_back( tr );
	idx++;
      }
      _trans.push_back( v_tr_a );
    }

    // Perceptions
    const rj::Value& ar_perc = obj["perc"];
    assert( ar_perc.IsArray() );
    idx = 0;
    for( unsigned int is = 0; is < _states.size(); ++is) {
      Transition tr;
      tr.unserialize( ar_perc[idx] );
      _percep.push_back( tr );
      idx++;
    }

    // Reward
    const rj::Value& ar_rew = obj["reward"];
    assert( ar_rew.IsArray() );
    idx = 0;
    for( unsigned int is = 0; is < _states.size(); ++is) {
      assert( ar_rew[idx].IsNumber() );
      _reward.push_back( ar_rew[idx].GetDouble() );
      idx++;
    }
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
