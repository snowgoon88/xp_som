/* -*- coding: utf-8 -*- */

#ifndef DSOM_NEURON_HPP
#define DSOM_NEURON_HPP

/** 
 * Neuron for DSOM kinf of Networks.
 */

#include <iostream>
#include <sstream>

// Eigen library for "dense" matrices and vectors
#include <Eigen/Dense>
#include <list>
#include <string>

// ********************************************************************* Model
namespace Model
{
// ********************************************************************** DSOM
namespace DSOM
{
// ***************************************************************************
// ****************************************************************** NeurDist
// ***************************************************************************
/** Store distance with other Neurone */
struct Neur_Dist {
  unsigned int index;
float dist;
};
// ***************************************************************************
// ******************************************************************** Neuron
// ***************************************************************************
/** Neuron for DSOM Net.
 * Can have a list of neighbors with distance.
 * or use the position of the other neurone to compute the distance.
 */
class Neuron
{
public:
  // ******************************************************** Neuron::creation
  /** Creation with index and random weights in [w_min,w_max]^dim */
  Neuron( int index, int dim_weights, float w_min=0, float w_max=1) :
    index(index), weights(nullptr)
  {
    //std::cerr << "Create Neurone " << index << "\n";
    
    // Generate weights
    Random rr;
    this->weights = new Eigen::VectorXf(dim_weights);
    
    for( int i=0; i < this->weights->size(); i++ ) {
      (*this->weights)(i) = w_min + rr.rndUniFloat() * (w_max - w_min);
    }
  };
  /** Creation with index, position and random weights in [w_min,w_max]^dim */
  Neuron( int index, Eigen::VectorXi pos,
	  int dim_weights, float w_min=0, float w_max=1) : 
    index(index), weights(nullptr), _pos(pos)
  {
    //std::cerr << "Create Neurone " << index << "\n";

    // Generate weights
    Random rr;
    this->weights = new Eigen::VectorXf(dim_weights);

    for( int i=0; i < this->weights->size(); i++ ) {
      (*this->weights)(i) = w_min + rr.rndUniFloat() * (w_max - w_min);
    }
  };
  /** Creation from Persistence (file). */
  //Neuron( Persistence& save ) {};
  /** Creation with copy */
  Neuron( const Neuron& n ) :
    index(n.index), weights(nullptr),
    l_link(n.l_link), l_neighbors(n.l_neighbors),
    _pos(n._pos)
  {
    // std::cerr << "CreateCopy Neurone " << this->index << "\n";
    if( n.weights ) {
      this->weights = new Eigen::VectorXf(*(n.weights));
    }
  };
  /** Creation from assignment */
  Neuron& operator=( const Neuron& n )
  {
    if (this != &n) { // protect against invalid self-assignment
      index = n.index;
      l_link = n.l_link;
      l_neighbors = n.l_neighbors;
      _pos = n._pos;
      if( n.weights ) {
	this->weights = new Eigen::VectorXf(*(n.weights));
      }
    }
  };
  // ********************************************************* Neuron::destroy
  /** Destruction */
  ~Neuron()
  {
    //std::cerr << "Delete Neurone " << this->index << "\n";
    if( this->weights != nullptr ) delete this->weights;
  };

  // ************************************************************* Neuron::str
  /** dump to STR */
  std::string str_dump()
  {
    std::stringstream ss;
    ss << toString() << "\n";
  
    ss << "    link=";
    std::list<unsigned int>::iterator i_link;
    for( i_link=this->l_link.begin(); i_link != this->l_link.end(); i_link++) {
      ss << "(" << (*i_link) << ") ";
    }

    ss << "\n    neig=";
    std::list<Neur_Dist>::iterator i_neigh;
    for( i_neigh=this->l_neighbors.begin(); i_neigh != this->l_neighbors.end(); i_neigh++) {
      ss << "(" << (*i_neigh).index << ", " << (*i_neigh).dist << ") ";
    }
    return ss.str();
  };
  /** display to STR */
  std::string str_display()
  {
    std::stringstream ss;
    ss << "[" << this->index << "] at (";
    for( unsigned int i = 0; i < _pos.size(); i++) {
      ss<< _pos[i] << ", ";
    }
    ss << ") w=";
    for( int i=0; i < this->weights->size(); i++) {
      ss << (*this->weights)(i) << " ";
    }
  
    return ss.str();
  };
  // ***************************************************** Neuron::Persistence
  /** write to an ouput stream */
  void write( std::ostream& out ) {};

  // ******************************************************* Neuron::neighbors
  /** add a direct neighbor */
  void add_link( unsigned int n_ind )
  {
    this->l_link.push_front( n_ind );
  };
  /** check if already has a link */
  bool has_link( unsigned int n_ind )
  {
    std::list<unsigned int>::iterator i_link;
    for( i_link=this->l_link.begin(); i_link != this->l_link.end(); i_link++) {
      if( n_ind == (*i_link) ) return true;
    }
    return false;
  };
  
  /** add a neighbor with distance */
  void add_neighbor( unsigned int n_ind, float n_dist)
  {
    Neur_Dist elem;
    elem.index = n_ind;
    elem.dist = n_dist;
    this->l_neighbors.push_front( elem );
  };  
  /** update existing distance if inferior or add new distance */
  void update_neighbor( unsigned int n_ind, float n_dist)
  {
    std::list<Neur_Dist>::iterator i_neigh;
    for( i_neigh=this->l_neighbors.begin(); i_neigh != this->l_neighbors.end(); i_neigh++) {
      // Same element => update if needed
      if( (*i_neigh).index == n_ind ) {
	if ( (*i_neigh).dist > n_dist ) {
	  l_neighbors.erase( i_neigh );
	  this->add_neighbor( n_ind, n_dist );
	}
	return;
      }
    }
    this->add_neighbor( n_ind, n_dist );
  };
  // ******************************************************** Neuron::distance
  /** compute distance to another neurone */
  float computeDistance( Neuron &neur) {};

  // ********************************************************* Neuron::forward
  /** compute distance from a given input */
  float computeDistance( Eigen::VectorXf &input ) {};
  /** compute normed distance from a given input (ie. between 0 and 1 */
  float computeDistanceNormed( Eigen::VectorXf &input )
  {
    float dim = input.size();
    return sqrt((*(this->weights) - input).cwiseProduct( *(this->weights) - input).sum()) /
      sqrt( dim );
  };
  // ******************************************************** Neuron::backward
  /** Add to current weights */
  void add_to_weights( Eigen::VectorXf &delta_weight )
  {
    *(this->weights) = *(this->weights) +  delta_weight;
  }

  // ****************************************************** Neuron::attributes
  /** Index */
  int index;
  /** Weights */
  Eigen::VectorXf *weights;
  /** List of Direct Neighbors */
  std::list<unsigned int> l_link;
  /** List of Neighbors */
  std::list<Neur_Dist> l_neighbors;
  /** Position on grid */
  Eigen::VectorXi _pos;
};
// ******************************************************************** Neuron
// ***************************************************************************


}; // DSOM

}; // Model

#endif // DSOM_NEURON_HPP
