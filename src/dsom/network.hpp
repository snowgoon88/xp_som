/* -*- coding: utf-8 -*- */

#ifndef DSOM_NETWORK_HPP
#define DSOM_NETWORK_HPP

/** 
 * DSOM Network.
 */
#include <iostream>
#include <sstream>
#include <random>                   // std::uniform_int...
#include <limits>                   // max dbl

#include <dsom/neuron.hpp>

#include "rapidjson/prettywriter.h" // rapidjson
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include <json_wrapper.hpp>         // JSON::OStreamWrapper et IStreamWrapper
namespace rj = rapidjson;

// ********************************************************************* Model
namespace Model
{
// ********************************************************************** DSOM
namespace DSOM
{
// ***************************************************************************
// ******************************************************************* Network
// ***************************************************************************
class Network
{
public:
  // ******************************************************* Network::creation
  /** 
   * Creation with dim of input and dsom neurons.
   * nb_link : One Neurone is directly connected to nb_link others
   * nb_link < 0 : mean regular grid of dimension |nb_link|
   */
  Network( int dim_input, int nb_neur, int nb_link=5,
		   float w_min=0.0, float w_max=1.0 )
  {
	// Init Random Engine
	std::random_device rnd_seeder;
	_rnd = std::default_random_engine( rnd_seeder() );

	// NON-Regular GRID
	if( nb_link > 0 ) {
	  _nb_link = nb_link;
	  _size_grid = 0;
    
	  // Create all the neurones
	  for( int i=0; i < nb_neur; i++) {
		Neuron *n = new Neuron( i, dim_input, w_min, w_max );
		v_neur.push_back(n);
	  }

	  // Create Random links
	  auto unif = std::uniform_int_distribution<>(0, nb_neur-1 );
	  // Each Neuron
	  for( unsigned int i=0; i<v_neur.size(); i++) {
		// Must have nb_link links at least
		while( (int) v_neur[i]->l_link.size() < nb_link ) {
		  int ind_other = unif(_rnd);
		  // but not a link to itself or an existing link
		  while( ind_other == (int) i || v_neur[i]->has_link( ind_other ) ) {
			ind_other = unif(_rnd);
		  }
		  v_neur[i]->add_link( ind_other );
		  v_neur[ind_other]->add_link( i );
		}
	  }
	  _max_dist_neurone = 1.0;
	  computeAllDist();
	}
	// Regular GRID
	else if( nb_link < 0 ) {
	  // Check dimension
	  float size_grid = powf( (float) nb_neur, 1.0 / (float) -nb_link);
	  _size_grid = abs( floor( size_grid));
	  if( pow( _size_grid, -nb_link) != nb_neur) {
		std::cerr << "Incompatible size nb_neur=" << nb_neur << ", size=" << _size_grid << ", dim=" << nb_link << "\n"; 
		exit(1);
	  }
	  _nb_link = nb_link;

	  // Create all the neurones
	  if( _nb_link == -2 ) {
		for( int i=0; i < _size_grid; i++) {
		  for( int j=0; j < _size_grid; j++) {
			Eigen::VectorXi v(-_nb_link);
			v << i, j;
			Neuron *n = new Neuron( i*_size_grid+j, v, dim_input, w_min, w_max );
			v_neur.push_back(n);
		  }
		}
	  }
	  else {
		std::cerr << "dim=" << _nb_link << " not implemented yet\n";
		exit(1);
	  }
    
      // Create links
	  if( _nb_link == -2 ) {
		for( int i=0; i < _size_grid; i++) {
		  for( int j=0; j < _size_grid; j++) {
			if( (i-1) >= 0 ) v_neur[i*_size_grid+j]->add_link( (i-1)*_size_grid+j);
			if( (i+1) < _size_grid ) v_neur[i*_size_grid+j]->add_link( (i+1)*_size_grid+j);
			if( (j-1) >= 0) v_neur[i*_size_grid+j]->add_link( i*_size_grid+(j-1));
			if( (j+1) < _size_grid) v_neur[i*_size_grid+j]->add_link( i*_size_grid+(j+1));
		  }
		}
	  }

	  _max_dist_neurone = v_neur[0]->computeDistance( *(v_neur[nb_neur-1]) );
	}
  }
  // ************************************************************ Network::str
  std::string str_dump()
  {
	std::stringstream ss;
	ss << "Net " << _size_grid << "^" << _nb_link << "\n";

	for( unsigned int i=0; i<v_neur.size(); i++) {
	  ss << (*v_neur[i]).str_dump() << "\n";
	}
	return ss.str();
  }
  // *********************************************************** Network::DIST
  double computeAllDist()
  {
	_max_dist_neurone = 0;
  
	for( unsigned int i=0; i<v_neur.size(); i++ ) {
	  float alt = this->computeDist( i );
	  if( alt > _max_dist_neurone ) _max_dist_neurone = alt;
	}
  
	return _max_dist_neurone;
  }
  double computeDist( unsigned int ind_neur)
  {
	unsigned int cur_neur = ind_neur;

	// Add all neurones to evaluation list
	std::list<unsigned int> l_eval;
	for( unsigned int i=0; i<v_neur.size(); i++ ) {
	  l_eval.push_front( i );
	}

	// Initialize distance array
	double dist[v_neur.size()];
	for( unsigned int i=0; i<v_neur.size(); i++ ) {
	  dist[i] = std::numeric_limits<double>::max();
	}
	dist[cur_neur] = 0;
	// using also existing distances
	std::list<Neur_Dist>::iterator i_neigh;
	for( i_neigh=v_neur[cur_neur]->l_neighbors.begin();
		 i_neigh != v_neur[cur_neur]->l_neighbors.end();
		 i_neigh++) {
	  dist[(*i_neigh).index] = (*i_neigh).dist;
	}

	// While new neurone to check
	while( l_eval.empty() == false ) {
	  // Look for neurone with min distance
	  double min_dist = std::numeric_limits<double>::max();
	  unsigned int next_neur = 0;
	  std::list<unsigned int>::iterator i_eval;
	  for( i_eval = l_eval.begin(); i_eval != l_eval.end(); i_eval++) {
		if( dist[(*i_eval)] < min_dist ) {
		  min_dist = dist[(*i_eval)];
		  next_neur = (*i_eval);
		}
	  }

	  if( min_dist == std::numeric_limits<double>::max() ) {
		// No more interesting Neurones in l_eval
		break;
	  }
    
	  // Remove next_neur from l_eval
	  l_eval.remove( next_neur );

	  // Update the distance to the direct links of next_neur
	  // if their are still in l_eval
	  std::list<unsigned int>::iterator i_link;
	  for( i_link=v_neur[next_neur]->l_link.begin();
		   i_link != v_neur[next_neur]->l_link.end(); i_link++) {
		// if it is still in l_eval
		if( this->is_in( (*i_link), l_eval) ) {
		  double alt = dist[next_neur] + 1.0;
		  // and new best distance --> update
		  if( alt < dist[(*i_link)] ) {
			dist[(*i_link)] = alt;
		  }
		}
	  }
	}

	double max_dist = 0;
	// update neighbors of cur_neur
	for( unsigned int i=0; i<v_neur.size(); i++ ) {
	  if( dist[i] != std::numeric_limits<double>::max() ) {
		v_neur[cur_neur]->update_neighbor( i, dist[i] );
		v_neur[i]->update_neighbor( cur_neur, dist[i] );
		if( dist[i] > max_dist ) max_dist = dist[i];
	  }
	}

	return max_dist;
  }
  // ********************************************************** Network::is_in
  bool is_in( unsigned int elem,  std::list<unsigned int> ll )
  {
	std::list<unsigned int>::iterator i;
	for( i = ll.begin(); i != ll.end(); i++) {
	  if( elem == (*i) ) return true;
	}
	return false;
  }

private:
  // ****************************************************** Network::attributs
  /** Random engine */
  std::default_random_engine _rnd;
  
  /** All the neurons */
  std::vector<DSOM::Neuron *> v_neur;
  
  /** Dimension of the grid */
  int _nb_link;
  /** Size of the regular grid */
  int _size_grid;

  /** The current winner neurone. */
  unsigned int _winner_neur;
  /** The current winner distance */
  double _winner_dist;
  /** The maximum distance between neurones */
  double _max_dist_neurone;
}; // class Network
}; // namespace DSOM
}; // namespace Model

#endif // DSOM_NETWORK_HPP
