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
  using TNumber = RNeuron::TNumber;
  using container_type = std::vector<DSOM::RNeuron *>;
  using Similarities = std::vector<TNumber>;
public:
  // ****************************************************** RNetwork::creation
  /** 
   * Creation with dim of input and dsom neurons.
   * nb_link : One Neurone is directly connected to nb_link others
   * nb_link < 0 : mean regular grid of dimension |nb_link|
   */
  RNetwork( int dim_input, int nb_neur, int nb_link=5,
	    float w_min=0.0, float w_max=1.0 ) :
    _winner_neur(0), _old_winner_neur(0), _pred_winner(0),
    _winner_dist(std::numeric_limits<double>::max()),
    _winner_dist_input(std::numeric_limits<double>::max()),
    _winner_dist_rec(std::numeric_limits<double>::max()),
    _winner_dist_pred(std::numeric_limits<double>::max()),
    _max_dist_neurone(0.0), _max_dist_input(0.0), _max_dist_rec(0.0),
    _sim_w(nb_neur,0.0), _sim_rec(nb_neur,0.0), _sim_merged(nb_neur,0.0),
    _sim_convol(nb_neur,0.0),
	_sim_hn_dist(nb_neur,0.0), _sim_hn_rec(nb_neur,0.0),
    _winner_similarity(0.0)
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
		  n->r_pos << (RNeuron::TNumber) i / (RNeuron::TNumber) _size_grid;
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
	//std::cout << "max_dist_neurone=" << _max_dist_neurone << std::endl;
  }
  /** Creation from JSON file */
  RNetwork( std::istream& is ) : 
    _winner_neur(0), _old_winner_neur(0),
    _winner_dist(std::numeric_limits<double>::max()),
    _winner_dist_input(std::numeric_limits<double>::max()),
    _winner_dist_rec(std::numeric_limits<double>::max()),
    _max_dist_neurone(0.0), _max_dist_input(0.0), _max_dist_rec(0.0),
    _sim_w(0,0.0), _sim_rec(0,0.0), _sim_merged(0,0.0),
    _sim_convol(0,0.0),
    _sim_hn_dist(0,0.0), _sim_hn_rec(0,0.0),
    _winner_similarity(0.0)
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
  // ********************************************************** RNetwork::play
  TNumber computeWinner( RNeuron::TWeight &input,
						 const RNeuron::TNumber& beta=1.0,
						 const TNumber& sig_input = 1.0,
						 const RNeuron::TNumber& sig_recur = 1.0,
						 const RNeuron::TNumber& sig_conv  = 1.0 )
  {
    //std::cout << "    _computeWinner " << std::endl;
	// compute both similarities ==> merged
	RNeuron::TWeight sim( v_neur.size() );
	_sim_w.clear();
	_sim_rec.clear();
	_sim_merged.clear();
	for( unsigned int i=0; i<v_neur.size(); ++i) {
	  // std::cout << "      with i=" << i;
	  // std::cout << " in=" << input << " sig_in=" << sig_input << std::endl;
	  auto simw = v_neur[i]->similaritiesInput( input, sig_input );
	  _sim_w.push_back( simw );

	  // std::cout << "      with i=" << i;
	  // std::cout << " in=" << input << " old=" << _old_winner_neur << " sig_rec=" << sig_recur << std::endl;
	  auto simrec = v_neur[i]->similaritiesRecurrent( v_neur[_old_winner_neur]->r_pos, sig_recur );
	  _sim_rec.push_back( simrec );
	  // std::cout << "      merging simw=" << simw << " simrec=" << simrec << std::endl;
	  _sim_merged.push_back( sqrt( _sim_w[i] * (beta+(1-beta) * _sim_rec[i] )) );
	  //LINEAR _sim_merged.push_back( _sim_w[i] * beta + (1.0 - beta) * _sim_rec[i] );
	}
	//std::cout << "     _convolution" << std::endl;
	// // Convolution with gaussian
	// integral of a.exp(-x^2/(2c^2)) = ac.sqrt(2.PI)
	_sim_convol.clear();
	for( int i=0; i< (int) v_neur.size(); i++) {
	  TNumber val = 0.0;
	  auto pos_i = v_neur[i]->r_pos(0);
	  for( int j=i-((int)v_neur.size())/2; j<i+(int)v_neur.size()/2; j++) {
		auto pos_j = 0.0 + (TNumber) j / (TNumber) v_neur.size();
		// TODO specific to 1D grid network
		auto k = ((j < 0) ? j+v_neur.size() : j);
		k = (k >= v_neur.size() ? k-v_neur.size() : k);

		auto dist = (pos_i - pos_j) / 1.0 ;
		// std::cout << "i,j,k=" << i<<", "<<j<<", "<<k<<" => "<<pos_i << " - " << pos_j;
		// std::cout << " : " <<  exp( - (dist*dist) / (2.0 * sig_conv * sig_conv)) << " * " << _sim_merged[k];
		val += _sim_merged[k] * exp( - (dist*dist) / (2.0 * sig_conv * sig_conv));
		// std::cout << " --> " << val << " (" << (val / (sig_conv * sqrt(2*M_PI))) << ")" << std::endl;
	  }
	  //std::cout << "convol[" << i << "]=" << val << std::endl;
	  _sim_convol.push_back( val / (double) v_neur.size() ); /// (sig_conv * sqrt(2*M_PI)) );
	}
	// TODO : normalize convolution ??
	
	// max and argmax
	auto it_max = std::max_element( _sim_convol.begin(), _sim_convol.end() );
	_winner_similarity = *it_max;
	_winner_neur = std::distance( _sim_convol.begin(), it_max );
	_winner_dist_input = v_neur[_winner_neur]->computeDistanceInput( input );
	_winner_dist_rec = v_neur[_winner_neur]->computeDistanceRPos( v_neur[_old_winner_neur]->r_pos );
	// Compare with the neuron what was predicted
	_winner_dist_pred = v_neur[_pred_winner]->computeDistanceInput( input );
	// best prediction will be the one with maximum _sim_rec
	auto it_pred = std::max_element( _sim_rec.begin(), _sim_rec.end());
	_pred_winner = std::distance( _sim_rec.begin(), it_pred );

	
	return _winner_similarity;
  }
  void forward( Eigen::VectorXd &input,
				const RNeuron::TNumber& beta=1.0,
				const TNumber& sig_input = 1.0,
				const RNeuron::TNumber& sig_recur = 1.0,
				const RNeuron::TNumber& sig_conv  = 1.0,
				bool verb = false)
  {
    // Compute the winner, this will update similarities
    if( verb ) {
      std::cout << "__FORWARD" << std::endl;
	  std::cout << "  => pred is " << _pred_winner;
      std::cout << "  " << v_neur[_pred_winner]->str_display() << std::endl;
	}
    computeWinner( input, beta, sig_input, sig_recur, sig_conv );
    if( verb ) {
      std::cout << "  in=" << input;
      std::cout << " old_win=" << _old_winner_neur << " at(" << v_neur[_old_winner_neur]->r_pos(0) << ")" << std::endl; 
      std::cout << "  => win is " << _winner_neur;
      std::cout << "  " << v_neur[_winner_neur]->str_display() << std::endl;
	  std::cout << "  => diff with old predicted is " << _winner_dist_pred << std::endl;
    }
    // and then, compute distances and update max_distances
    for( unsigned int i = 0; i < v_neur.size(); ++i) {
      // input
      auto dist_input = v_neur[i]->computeDistanceInput( input );
      _max_dist_input = std::max( _max_dist_input, dist_input);
      // rec
      auto dist_rec = v_neur[i]->computeDistanceRPos( v_neur[_old_winner_neur]->r_pos );
      _max_dist_rec = std::max( _max_dist_rec, dist_rec);
      //std::cout << "  n[" << i << "] din=" << dist_input << "; dr=" << dist_rec << std::endl;
    }
    //std::cout << "  MAX din=" << _max_dist_input << "; dr=" << _max_dist_rec << std::endl;
  }
  // ******************************************************* Network::backward
  double hnDistance( double dist_neur_win, double win_dist, double ela)
  {
	//std::cout << "HN: dnw=" << dist_neur_win << "; wd=" << win_dist << "; ela=" << ela << std::endl;
	
	if( win_dist < 0.000001 ) win_dist = 0.000001;
  
	return exp( -1.0 * (dist_neur_win*dist_neur_win)/( ela * ela * win_dist * win_dist ) );
  }
  void deltaW( Eigen::VectorXd &input, double eps, double ela,
	       double ela_rec = 1.0, double verb=false)
  {
	if( verb ) 
	  std::cout << "__DeltaW" << std::endl;
    // TODO NON-Regular GRID
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
	  _sim_hn_dist.clear();
	  _sim_hn_rec.clear();
	  if( verb ) {
	    std::cout << "  ** max_dist_neur= " << _max_dist_neurone;
	    std::cout << " max_dist_input= " << _max_dist_input;
	    std::cout << " max_dist_rec= " << _max_dist_rec;
	    std::cout << std::endl;
	    std::cout << "  ** d_win_input=" << _winner_dist_input / _max_dist_input;
	    std::cout << " d_win_rec= " << _winner_dist_rec / _max_dist_rec;
	    std::cout << std::endl;
	  }
	  
      auto old_win_rpos = v_neur[_old_winner_neur]->r_pos;
	  //std::cout <<  "  old_win is " << _old_winner_neur << " at " << old_win_rpos << std::endl;
      
      // All neurones will be adapted, difference betwenn weights and r_weights
      for( unsigned int indn = 0; indn < v_neur.size(); indn++ ) {
	// normalized distance to input (ie with weights)
	auto dnorm_in = v_neur[indn]->computeDistanceInput( input ) / _max_dist_input;
	// normalized distance to previous winner (ie with r_weights)
	auto dnorm_rec = v_neur[indn]->computeDistanceRPos( old_win_rpos ) / _max_dist_rec;

      // hn_distance
	auto hn_input = hnDistance( v_neur[indn]->computeDistancePos( *(v_neur[_winner_neur]) ) /_max_dist_neurone, _winner_dist_input / _max_dist_input, ela );
	_sim_hn_dist.push_back( hn_input );
	
	auto hn_rec = hnDistance( v_neur[indn]->computeDistancePos( *(v_neur[_winner_neur]) ) /_max_dist_neurone, _winner_dist_rec / _max_dist_rec, ela_rec );
	_sim_hn_rec.push_back( hn_rec );
	
	// Delta W / RecWeights
	auto delta_w = eps * dnorm_in * hn_input * (input - v_neur[indn]->weights);
	auto delta_rw = eps * dnorm_rec * hn_rec * (old_win_rpos - v_neur[indn]->r_weights);
	
	if( verb and (abs((int)indn - (int)_winner_neur) < 3) ) {
	    std::cout << "  " << v_neur[indn]->str_display() << "\n";
	    std::cout << "    dist_pos_win=" <<  v_neur[indn]->computeDistancePos( *(v_neur[_winner_neur])) << std::endl;
	  std::cout << "    INPUT: dnorm= " << dnorm_in << "; hn=" << hn_input << " => delta=" << delta_w << std::endl;
	  std::cout << "    REC  : dnorm= " << dnorm_rec << "; hn=" << hn_rec << " =>  delta=" << delta_rw << std::endl;
	}      
	v_neur[indn]->add_to_weights( delta_w );
	v_neur[indn]->add_to_r_weights( delta_rw );
      }
    }

    _old_winner_neur = _winner_neur;
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
  double get_winner_dist_input() const { return _winner_dist_input; }
  double get_winner_dist_rec() const { return _winner_dist_rec; }
  double get_winner_dist_pred() const { return _winner_dist_pred; }
  double get_max_dist_neurone() { return _max_dist_neurone; }
private:
  /** Random engine */
  std::default_random_engine _rnd;
public:
  /** All the neurons */
  std::vector<DSOM::RNeuron *> v_neur;
  
  /** Dimension of the grid */
  int _nb_link;
  /** Size of the regular grid */
  int _size_grid;

  /** The current, old and predicted winner neurone. */
  unsigned int _winner_neur;
  unsigned int _old_winner_neur;
  unsigned int _pred_winner;
  /** The current winner distance */
  double _winner_dist;
  double _winner_dist_input, _winner_dist_rec, _winner_dist_pred;
  /** The maximum distance between neurones */
  double _max_dist_neurone;
  /** The maximum distance between inputs */
  double _max_dist_input;
  double _max_dist_rec;

  /** Similarities */
  std::vector<RNeuron::TNumber> _sim_w;
  std::vector<RNeuron::TNumber> _sim_rec;
  std::vector<RNeuron::TNumber> _sim_merged;
  std::vector<RNeuron::TNumber> _sim_convol;
  std::vector<RNeuron::TNumber> _sim_hn_dist;
  std::vector<RNeuron::TNumber> _sim_hn_rec;
  /** Similarity with the Winner */
  double _winner_similarity;
}; // class RNetwork
}; // namespace DSOM
}; // namespace Model

#endif // DSOM_NETWORK_HPP
