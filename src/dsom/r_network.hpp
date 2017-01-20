/* -*- coding: utf-8 -*- */

#ifndef DSOM_R_NETWORK_HPP
#define DSOM_R_NETWORK_HPP

/** 
 * DSOM recurrent Network.
 */
#include <iostream>
#include <sstream>
#include <random>                   // std::uniform_int...
#include <limits>                   // max dbl
#include <algorithm>    // std::max

#include <dsom/r_neuron.hpp>
#include <dsom/utils.hpp>

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
  class RNetwork
{
public:
  // ****************************************************** RNetwork::creation
  /** 
   * Creation with dim of input and dsom neurons.
   * nb_link : One Neurone is directly connected to nb_link others
   * nb_link < 0 : mean regular grid of dimension |nb_link|
   */
  RNetwork( int dim_input, int nb_neur, int nb_link=5,
	    float w_min=0.0, float w_max=1.0 ) :
    _max_dist_neurone(0.0), _max_dist_input(0.0) 
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
	RNeuron *n = new RNeuron( i, dim_input, w_min, w_max );
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
	    RNeuron *n = new RNeuron( i*_size_grid+j, v, dim_input, w_min, w_max );
	    v_neur.push_back(n);
	  }
	}
      }
      else if( _nb_link == -1 ) {
	for( int i=0; i < _size_grid; i++) {
	  Eigen::VectorXi v(-_nb_link);
	  v << i;
	  RNeuron *n = new RNeuron( i, v, dim_input, w_min, w_max );
	  v_neur.push_back(n);
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
      if( _nb_link == -1 ) {
	for( int i=0; i < _size_grid; i++) {
	  if( (i-1) >= 0 ) v_neur[i]->add_link( (i-1) );
	  if( (i+1) < _size_grid ) v_neur[i]->add_link( (i+1) );
	}
      }
      
      _max_dist_neurone = v_neur[0]->computeDistancePos( *(v_neur[nb_neur-1]) );
    }
  }
  /** Creation from JSON file */
  RNetwork( std::istream& is )
  {
    // Wrapper pour lire document
    JSON::IStreamWrapper instream(is);
    // Parse into a document
    rj::Document doc;
    doc.ParseStream( instream );

    unserialize( doc );
  }
  // *********************************************************** RNetwork::str
  std::string str_dump()
  {
	std::stringstream ss;
	ss << "Net " << _size_grid << "^" << _nb_link;
	ss << " max_d_input=" << _max_dist_input << "\n";

	for( unsigned int i=0; i<v_neur.size(); i++) {
	  ss << (*v_neur[i]).str_dump() << "\n";
	}
	return ss.str();
  }
  // ******************************************* RNetwork::set_regular_weights
  void set_regular_weights()
  {
    int dim_weights = v_neur[0]->weights.size();

    // si _nb_link > 0, must check dimensions and nb_neur
    if( _nb_link > 0 ) {
      // Check dimension
      auto size_grid = pow( (double) v_neur.size(), 1.0 / (double) dim_weights );
      _size_grid = abs( floor( size_grid));
      if( pow( _size_grid, dim_weights) != v_neur.size()) {
	std::cerr << "Incompatible size nb_neur=" << v_neur.size() << ", size=" << _size_grid << ", dim=" << dim_weights << "\n"; 
	return;
      }
    }
    else if( -_nb_link != dim_weights ) {
      std::cerr << "dim_weights=" << dim_weights << " is not compatible with internal regular grid dimension _nb_link=" << _nb_link << "\n";
      return;
    }
  
    // Initialize weights
    if( dim_weights == 2 ) {
      for( int i=0; i < _size_grid; i++) {
	for( int j=0; j < _size_grid; j++) {
	  Eigen::VectorXd v(dim_weights);
	  v << (double) i / (double) (_size_grid-1), (double) j / (double) (_size_grid-1);
	  v_neur[i*_size_grid+j]->weights = v;
	}
      }
    }
    else {
      std::cerr << "TODO initialise with dim_weight != 2 ....\n";
    }
  }
  // *********************************************************** Network::DIST
  /** 
   * Compute distance between all Neurones
   * Useful only for nb_link > 0
   * Return : distance max between neurones
   */
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
  // *********************************************************** Network::play
  double computeWinner( Eigen::VectorXd &input )
  {
    _winner_dist = std::numeric_limits<double>::max();
    
    for( unsigned int i=0; i<v_neur.size(); i++) {
      auto tmp = v_neur[i]->computeDistanceInput( input );
      //std::cout << "win_dist[" << i << "] = " << tmp << "\n";
      if( tmp < _winner_dist ) {
	_winner_dist = tmp;
	_winner_neur = i;
      }
    }
    return _winner_dist;
  }
  Eigen::VectorXd forward( Eigen::VectorXd &input )
  {
	Eigen::VectorXd output(v_neur.size());
  
	for( unsigned int i=0; i<v_neur.size(); i++) {
	  output[i] = v_neur[i]->computeDistanceInput( input );
	}
	_winner_dist = output.minCoeff( &_winner_neur );

	// And update max_distance
	auto max_dist_sample = output.maxCoeff();
	_max_dist_input = std::max( _max_dist_input, max_dist_sample);
	
	return output;
  }
  // ******************************************************* Network::backward
  double hnDistance( double dist_neur_win, double win_dist, double ela)
  {
	if( win_dist < 0.000001 ) win_dist = 0.000001;
  
	return exp( -1.0 * (dist_neur_win*dist_neur_win)/( ela * ela * win_dist * win_dist ) );
  }
  void deltaW( Eigen::VectorXd &input, double eps, double ela, double verb=false)
  {
    // NON-Regular GRID
    if( _nb_link > 0 ) {
      // All neigbors of the winner will be adapted
      Neuron *win = v_neur[_winner_neur];
      if( verb ) {
	std::cout << "Network::deltaW for Winner Neurone\n";
	std::cout << win->str_dump() << "\n";
	std::cout << "At winning distance of " << _winner_dist;
	std::cout << " from input " << utils::eigen::str_vec(input) << "\n";
	
	std::cout << "N[#]\t dp\t dw\t k\t (ratio)\t delta\t dW\n";
      }
      std::list<Neur_Dist>::iterator i_neigh;
      for( i_neigh= win->l_neighbors.begin();
	   i_neigh != win->l_neighbors.end();
	   i_neigh++) {
	auto delta = eps * v_neur[(*i_neigh).index]->computeDistanceInput( input ) / _max_dist_input * this->hnDistance( (*i_neigh).dist/_max_dist_neurone, _winner_dist/_max_dist_input, ela);
	auto delta_weight = delta * (input - v_neur[(*i_neigh).index]->weights);
		
	if( verb ) {
	  std::cout << "N[" << (*i_neigh).index << "]\t";
	  std::cout << " " << (*i_neigh).dist << " / " << _max_dist_neurone << "\t";
	  std::cout << " " << v_neur[(*i_neigh).index]->computeDistanceInput( input ) / _max_dist_input << "\t";
	  std::cout << " " << hnDistance( (*i_neigh).dist/_max_dist_neurone, _winner_dist/_max_dist_input, ela) << " (" << ((*i_neigh).dist/_max_dist_neurone/_winner_dist*_max_dist_input)*((*i_neigh).dist/_max_dist_neurone/_winner_dist*_max_dist_input) << ")\t";
	  std::cout << " " << delta << "\tdW=" << utils::eigen::str_vec(delta_weight) << "\n";
	}      
	v_neur[(*i_neigh).index]->add_to_weights( delta_weight );
      }
      if( verb ) {
	std::cout << "********\n";
      }
    }
    // REGULAR GRID
    else if (_nb_link < 0 ) {
      // All neurones will be adapted
      for( unsigned int indn = 0; indn < v_neur.size(); indn++ ) {
	auto delta = eps * v_neur[indn]->computeDistanceInput( input ) / _max_dist_input * this->hnDistance( v_neur[indn]->computeDistancePos( *(v_neur[_winner_neur]) ) /_max_dist_neurone, _winner_dist/_max_dist_input, ela);
	auto delta_weight = delta * (input - v_neur[indn]->weights);
      
	if( verb ) {
	  std::cout << "Neurone " << indn << "\n";
	  std::cout << "Distance to winner " << v_neur[indn]->computeDistancePos( *(v_neur[_winner_neur]) ) /_max_dist_neurone << "\n";
	  std::cout << "hnDist = " << this->hnDistance( v_neur[indn]->computeDistancePos( *(v_neur[_winner_neur]) ) /_max_dist_neurone, _winner_dist/_max_dist_input, ela) << "\n";
	  std::cout << "delta=" << delta << "\ndW=" << delta_weight << "\n";
	}      
	v_neur[indn]->add_to_weights( delta_weight );
      }
    }
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
  // *********************************************************** Network::JSON
  rj::Value serialize( rj::Document& doc )
  {
	// rj::Object that holds the data
	rj::Value rj_node;
	rj_node.SetObject();
	
	// global data
	rj_node.AddMember( "nb_neur", rj::Value(v_neur.size()), doc.GetAllocator());
	rj_node.AddMember( "nb_link", rj::Value(_nb_link), doc.GetAllocator());
	rj_node.AddMember( "size_grid", rj::Value(_size_grid), doc.GetAllocator());
	rj_node.AddMember( "max_dist_input", rj::Value(_max_dist_input), doc.GetAllocator());

	// Array of neurons
	rj::Value rj_neur;
	rj_neur.SetArray();
	for( auto& n: v_neur) {
	  rj_neur.PushBack( n->serialize(doc), doc.GetAllocator());
	}
	rj_node.AddMember( "r_neurons", rj_neur, doc.GetAllocator());
	
	return rj_node;
  }
  void unserialize( const rj::Value& obj )
  {
	// global data
	int nb_neur = obj["nb_neur"].GetInt();
	_nb_link = obj["nb_link"].GetInt();
	_size_grid = obj["size_grid"].GetInt();
	_max_dist_input = obj["max_dist_input"].GetDouble();

	// Neurons
	v_neur.clear();
	const rj::Value& n = obj["r_neurons"];
	assert( n.IsArray() );
	for( unsigned int i = 0; i < n.Size(); ++i) {
	  RNeuron *neur = new RNeuron( n[i] );
	  v_neur.push_back(neur);
	}

	// Now, eventually links and distance
	// NON-Regular GRID
	if( _nb_link > 0 ) {
	  _max_dist_neurone = 1.0;
	  computeAllDist();
	}
	// Regular GRID
	else if( _nb_link < 0 ) {
	  // Check dimension
	  if( pow( _size_grid, -_nb_link) != nb_neur) {
		std::cerr << "Incompatible size nb_neur=" << nb_neur << ", size=" << _size_grid << ", dim=" << _nb_link << "\n"; 
		exit(1);
	  }

	  _max_dist_neurone = v_neur[0]->computeDistancePos( *(v_neur[nb_neur-1]) );
	}
  }
  // ****************************************************** Network::attributs
public:
  unsigned int get_winner() const { return _winner_neur; }
  double get_winner_dist() const { return _winner_dist; }
  double get_max_dist_neurone() { return _max_dist_neurone; }
private:
  /** Random engine */
  std::default_random_engine _rnd;
  
  /** All the neurons */
  std::vector<DSOM::RNeuron *> v_neur;
  
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
  /** The maximum distance between inputs */
  double _max_dist_input;
}; // class Network
}; // namespace DSOM
}; // namespace Model

#endif // DSOM_NETWORK_HPP
