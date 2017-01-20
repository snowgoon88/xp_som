/* -*- coding: utf-8 -*- */

#ifndef DSOM_R_NEURON_HPP
#define DSOM_R_NEURON_HPP

/** 
 * Recurrent Neuron for DSOM kind of Networks.
 */
#include <dsom/neuron.hpp>

// ********************************************************************* Model
namespace Model
{
// ********************************************************************** DSOM
namespace DSOM
{
// ***************************************************************************
// ******************************************************************** Neuron
// ***************************************************************************
/** Recurrent Neuron for DSOM Net.
 * Derived from DSOM::Neuron
 *   - Can have a list of neighbors with distance.
 *   - or use the position of the other neurone to compute the distance.
 * + rweights, in the position space => TODO dim=1
 */
  class RNeuron : public Neuron
{
public:
  // ************************************************************ RNeuron_TYPE
  using TNumber  = double;
  using TWeight  = Eigen::VectorXd;
  using TRWeight = Eigen::VectorXd;
  using TPos     = Eigen::VectorXi;
public:
  // ******************************************************** Neuron::creation
  /** 
   * Creation with index and random weights in [w_min,w_max]^dim
   * RWeights are in [0,1]^dim
   */
  RNeuron( int index, int dim_weights, TNumber w_min=0, TNumber w_max=1) :
    Neuron(index,dim_weights,w_min,w_max)
  {
    std::cerr << "Create RNeurone " << index << "\n";
    
    // Generate r_weights between 0 and 1 (Eigen) ^ dim (TODO=1)
    this->r_weights = Eigen::VectorXd::Random(1);
    // Scale (TODO dim=1)
    this->r_weights= (this->r_weights.array() - -1.0) / (1.0 - -1.0) * (1.0 - 0.0) + 0.0;
  }
  /** 
   * Creation with index, position and random weights in [w_min,w_max]^dim
   * RWeights in [0,1]^dimension
   */
  RNeuron( int index, const Eigen::Ref<const TPos>& pos,
	  int dim_weights, TNumber w_min=0, TNumber w_max=1) :
    Neuron( index, pos, dim_weights, w_min, w_max)
  {
    std::cerr << "Create RNeurone " << index << "\n";

    auto dim = _pos.size();
    // Generate r_weights between 0 and 1 (Eigen) ^ dim_pos
    this->r_weights = Eigen::VectorXd::Random(dim);
    // Scale
    this->r_weights= (this->r_weights.array() - -1.0) / (1.0 - -1.0) * (1.0 - 0.0) + 0.0;
    
  };
  /** Creation with copy */
  RNeuron( const RNeuron& n ) :
    Neuron( n ), r_weights( n.r_weights )
  {
  }
  /** Creation from assignment */
  RNeuron& operator=( const RNeuron& n )
  {
    if (this != &n) { // protect against invalid self-assignment
      index = n.index;
      l_link = n.l_link;
      l_neighbors = n.l_neighbors;
      _pos = n._pos;
      weights = n.weights;
      r_weights = n.r_weights;
    }
    return *this;
  }
  /** Creation from JSON doc */
  RNeuron( const rj::Value& obj ) : Neuron(0,1)
  {
    // decode d'après obj
    this->unserialize( obj );
  }
  /** Creation from JSON file */
  RNeuron( std::istream& is ) : Neuron(0,1)
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
  // ******************************************************** RNeuron::destroy
  /** Destruction */
  ~RNeuron()
  {
  }
  // ************************************************************ RNeuron::str
  /** dump to STR */
  std::string str_dump() 
  {
    std::stringstream ss;
    ss << this->str_display() << "\n";
  
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
    ss << "RNeuron ";
    ss << Neuron::str_display();

    ss << " rw=";
    for( int i=0; i < this->r_weights.size(); i++) {
      ss << this->r_weights(i) << " ";
    }
  
    return ss.str();
  }
  // *********************************************************** RNeuron::JSON
  rj::Value serialize( rj::Document& doc )
  {
    // rj::Object qui contient les données
    rj::Value rj_node = Neuron::serialize( doc );
    
    // rj::Array with RWeights
    rj::Value rj_rw;
    rj_rw.SetArray();
    for( unsigned int i = 0; i < this->r_weights.size(); ++i) {
      rj_rw.PushBack( this->r_weights(i), doc.GetAllocator());
    }
    rj_node.AddMember( "r_weights", rj_rw, doc.GetAllocator() );

    return rj_node;
  }
  void unserialize( const rj::Value& obj)
  {
    Neuron::unserialize( obj );

    // RWeights
    const rj::Value& rw = obj["r_weights"];
    assert( rw.IsArray() );
    this->r_weights.resize( rw.Size() );
    for( unsigned int i = 0; i < rw.Size(); ++i) {
      this->r_weights(i) = rw[i].GetDouble();
    }
  }
  // ****************************************************** Neuron::attributes
  /** RWeights */
  TRWeight r_weights;
};
// ******************************************************************* RNeuron
// ***************************************************************************
}; // namespace DSOM

}; // namespace Model

#endif // DSOM_R_NEURON_HPP
