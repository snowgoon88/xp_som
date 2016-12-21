/* -*- coding: utf-8 -*- */

#ifndef DSOM_NEURON_HPP
#define DSOM_NEURON_HPP

/** 
 * Neuron for DSOM kind of Networks.
 */

#include <iostream>
#include <sstream>

// Eigen library for "dense" matrices and vectors
#include <Eigen/Dense>
#include <list>
#include <string>

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
// ****************************************************************** NeurDist
// ***************************************************************************
/** Store distance with other Neurone */
struct Neur_Dist {
  unsigned int index;
  double dist;
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
  // ************************************************************* Neuron_TYPE
  typedef double          TNumber;
  typedef Eigen::VectorXd TWeight;
  typedef Eigen::VectorXi TPos;
public:
  // ******************************************************** Neuron::creation
  /** Creation with index and random weights in [w_min,w_max]^dim */
  Neuron( int index, int dim_weights, TNumber w_min=0, TNumber w_max=1) :
    index(index)
  {
    //std::cerr << "Create Neurone " << index << "\n";
    
    // Generate weights between -1 and 1 (Eigen)
    this->weights = Eigen::VectorXd::Random(dim_weights);
    // Scale
    this->weights= (this->weights.array() - -1.0) / (1.0 - -1.0) * (w_max - w_min) + w_min;
  }
  /** Creation with index, position and random weights in [w_min,w_max]^dim */
  Neuron( int index, const Eigen::Ref<const TPos>& pos,
	  int dim_weights, TNumber w_min=0, TNumber w_max=1) : 
    index(index), _pos(pos)
  {
    //std::cerr << "Create Neurone " << index << "\n";

    // Generate weights between -1 and 1 (Eigen)
    this->weights = Eigen::VectorXd::Random(dim_weights);
    // Scale
    this->weights= (this->weights.array() - -1.0) / (1.0 - -1.0) * (w_max - w_min) + w_min;
  }
  /** Creation from Persistence (file). */
  //Neuron( Persistence& save ) {};
  /** Creation with copy */
  Neuron( const Neuron& n ) :
    index(n.index), weights(n.weights),
    l_link(n.l_link), l_neighbors(n.l_neighbors),
    _pos(n._pos)
  {
  }
  /** Creation from assignment */
  Neuron& operator=( const Neuron& n )
  {
    if (this != &n) { // protect against invalid self-assignment
      index = n.index;
      l_link = n.l_link;
      l_neighbors = n.l_neighbors;
      _pos = n._pos;
      weights = n.weights;
    }
    return *this;
  }
  /** Creation from JSON file */
  Neuron( std::istream& is )
  {
	// Wrapper pour lire document
    JSON::IStreamWrapper instream(is);
    // Parse into a document
    rj::Document doc;
    doc.ParseStream( instream );

    // std::cout << "Document read" << std::endl;
    // for (rj::Value::ConstMemberIterator itr = doc.MemberBegin();
	//  itr != doc.MemberEnd(); ++itr) {
    //   std::cout << "Doc has " << itr->name.GetString() << std::endl;
    // }

    unserialize( doc );
  }
  Neuron( const rj::Value& obj )
  {
	unserialize( obj );
  }
  // ********************************************************* Neuron::destroy
  /** Destruction */
  ~Neuron()
  {
  }
  // ************************************************************* Neuron::str
  /** dump to STR */
  std::string str_dump() 
  {
    std::stringstream ss;
    ss << str_display() << "\n";
  
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
  }
  /** display to STR */
  std::string str_display() const
  {
    std::stringstream ss;
    ss << "[" << this->index << "] at (";
    for( unsigned int i = 0; i < _pos.size(); i++) {
      ss<< _pos[i] << ", ";
    }
    ss << ") w=";
    for( int i=0; i < this->weights.size(); i++) {
      ss << this->weights(i) << " ";
    }
  
    return ss.str();
  }
  // ************************************************************ Neuron::JSON
  rj::Value serialize( rj::Document& doc )
  {
    // rj::Object qui contient les données
    rj::Value rj_node;
    rj_node.SetObject();
    rj_node.AddMember( "id", rj::Value(index), doc.GetAllocator() );
    
    // rj::Array qui contient les Weights
    rj::Value rj_w;
    rj_w.SetArray();
    for( unsigned int i = 0; i < this->weights.size(); ++i) {
      rj_w.PushBack( this->weights(i), doc.GetAllocator());
    }
    rj_node.AddMember( "weights", rj_w, doc.GetAllocator() );

    // Links
    rj::Value rj_link;
    rj_link.SetArray();
    for( auto& link: l_link) {
      rj_link.PushBack( link, doc.GetAllocator() );
    }
    rj_node.AddMember( "link", rj_link, doc.GetAllocator());
    
    // Neighbors
    rj::Value rj_neigh;
    rj_neigh.SetArray();
    for( auto& neigh: l_neighbors) {
      rj::Value rj_cpl;
      rj_cpl.SetArray();
      rj_cpl.PushBack( neigh.index, doc.GetAllocator() );
      rj_cpl.PushBack( neigh.dist, doc.GetAllocator() );
      rj_neigh.PushBack( rj_cpl, doc.GetAllocator() );
    }
    rj_node.AddMember( "neighbors", rj_neigh, doc.GetAllocator() );

    // Pos
    rj::Value rj_pos;
    rj_pos.SetArray();
    for( unsigned int i = 0; i < _pos.size(); ++i) {
      rj_pos.PushBack( _pos(i), doc.GetAllocator());
    }
    rj_node.AddMember( "pos", rj_pos, doc.GetAllocator() );
    
    return rj_node;
  }
  void unserialize( const rj::Value& obj)
  {
	// id
	index = obj["id"].GetInt();

	// Weights
	const rj::Value& w = obj["weights"];
	assert( w.IsArray() );
	this->weights.resize( w.Size() );
	for( unsigned int i = 0; i < w.Size(); ++i) {
	  this->weights(i) = w[i].GetDouble();
	}

	// Links
	l_link.clear();
	const rj::Value& l = obj["link"];
	assert(l.IsArray() );
	for( unsigned int i = 0; i < l.Size(); ++i) {
	  l_link.push_back( l[i].GetUint() );
	}

	// Neighbors
	l_neighbors.clear();
	const rj::Value& n = obj["neighbors"];
	for( unsigned int i = 0; i < n.Size(); ++i) {
	  add_neighbor( n[i][0].GetUint(), n[i][1].GetDouble() );
	}

	// Position
	const rj::Value& p = obj["pos"];
	_pos.resize( p.Size() );
	for( unsigned int i = 0; i < p.Size(); ++i) {
	  _pos(i) = p[i].GetInt();
	}
  }
  // ***************************************************** Neuron::Persistence
  /** write to an ouput stream */
  // void write( std::ostream& out ) {};
  // out << "##### DSOM:Neuron\n";
  // out << "DSOM_Neuron.id: " << index << "\n";
  
  // out << "DSOM_Neuron.dim_pos: " << _pos.size() << "\n";
  // for( unsigned int i=0; i<_pos.size(); i++ ) 
  //     out << "DSOM_Neuron.pos: " << _pos[i] << "\n";

  // out << "DSOM_Neuron.nb_weights: " << weights->size() << "\n";
  // for( unsigned int i=0; i<weights->size(); i++ ) 
  //   out << "DSOM_Neuron.weight: " << (*weights)[i] << "\n";

  // out << "DSOM_Neuron.nb_link: " << l_link.size() << "\n";
  // std::list<unsigned int>::iterator i_link;
  // for( i_link=this->l_link.begin(); i_link != this->l_link.end(); i_link++)
  //   out << "DSOM_Neuron.link: " << (*i_link) << "\n";

  // ******************************************************* Neuron::neighbors
  /** add a direct neighbor */
  void add_link( unsigned int n_ind )
  {
    this->l_link.push_front( n_ind );
  }
  /** check if already has a link */
  bool has_link( unsigned int n_ind ) 
  {
    std::list<unsigned int>::iterator i_link;
    for( i_link=this->l_link.begin(); i_link != this->l_link.end(); i_link++) {
      if( n_ind == (*i_link) ) return true;
    }
    return false;
  }
  /** add a neighbor with distance */
  void add_neighbor( unsigned int n_ind, TNumber n_dist)
  {
    Neur_Dist elem;
    elem.index = n_ind;
    elem.dist = n_dist;
    this->l_neighbors.push_front( elem );
  }  
  /** update existing distance if inferior or add new distance */
  void update_neighbor( unsigned int n_ind, TNumber n_dist)
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
  }
  // ******************************************************** Neuron::distance
  /** compute distance to another neurone */
  TNumber computeDistance( Neuron &neur)
  {
	return sqrt((_pos - neur._pos).cwiseProduct( _pos - neur._pos).sum());
  }

  // ********************************************************* Neuron::forward
  /** compute distance from a given input */
  //TNumber computeDistance( TWeight &input ) {};
  /** compute normed distance from a given input (ie. between 0 and 1 */
  TNumber computeDistanceNormed( const TWeight& input ) const
  {
    TNumber dim = input.size();
    return sqrt((this->weights - input).cwiseProduct(this->weights - input).sum()) /
      sqrt( dim );
  };
  // ******************************************************** Neuron::backward
  /** Add to current weights */
  void add_to_weights( const TWeight& delta_weight )
  {
    this->weights = this->weights +  delta_weight;
  }
  // ****************************************************** Neuron::attributes
  /** Index */
  int index;
  /** Weights */
  TWeight weights;
  /** List of Direct Neighbors */
  std::list<unsigned int> l_link;
  /** List of Neighbors */
  std::list<Neur_Dist> l_neighbors;
  /** Position on grid */
  TPos _pos;
};
// ******************************************************************** Neuron
// ***************************************************************************


}; // DSOM

}; // Model

#endif // DSOM_NEURON_HPP
