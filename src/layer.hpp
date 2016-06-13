/* -*- coding: utf-8 -*- */

#ifndef LAYER_HPP
#define LAYER_HPP

/** 
 * Une couche (de sortie pour le Reservoir).
 * out = W.input
 * W.size1 = output_size
 * W.size2 = input_size
 */

#include <iostream>                 // std::cout
#include <sstream>                  // std::stringstream

#include <gsl/gsl_matrix.h>         // gsl Matrices
#include <gsl/gsl_blas.h>           // gsl matrix . vector multiplication

#include "rapidjson/prettywriter.h" // rapidjson
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include <json_wrapper.hpp>         // JSON::OStreamWrapper et IStreamWrapper
namespace rj = rapidjson;

#include <utils.hpp>                // utils::str_vec, utils::str_mat ...
using namespace utils::gsl;
// ***************************************************************************
// ********************************************************************* Layer
// ***************************************************************************
class Layer
{
public:
  typedef std::vector<double> Tinput;
  typedef unsigned int        Tinput_size;
  typedef std::vector<double> Toutput;
  typedef unsigned int        Toutput_size;
  typedef gsl_matrix*         TweightsPtr;
  typedef gsl_vector*         TstatePtr;
  // **************************************************************** creation
  Layer( Tinput_size input_size, Toutput_size output_size ) :
    _w(nullptr), _y_out(nullptr)
  {
    // Weights
    _w = gsl_matrix_calloc( output_size, input_size);
    // Output
    _y_out = gsl_vector_calloc( output_size );
  }
  /** 
   * Creation à partir d'un fichier contenant uniquement JSON format
ema   * of ONE layer.
   */
  Layer( std::istream& is ) :
    _w(nullptr), _y_out(nullptr)
  {
    // Wrapper pour lire document
    JSON::IStreamWrapper instream(is);
    // Parse into a document
    rapidjson::Document doc;
    doc.ParseStream( instream );

    std::cout << "Document read" << std::endl;

    unserialize( doc );
  }
  /** Creation from a piece of JSON in a Document */
  Layer( const rapidjson::Value& obj ) :
    _w(nullptr), _y_out(nullptr)
  {
    unserialize( obj );
  }
  // ************************************************************* Layer::copy
  Layer(const Layer& other) :
    _w(nullptr), _y_out(nullptr)
  { 
    // copy
    _w = gsl_matrix_alloc( other._w->size1, other._w->size2 );
    gsl_matrix_memcpy( _w, other._w );
    _y_out = gsl_vector_calloc( other._y_out->size );
    gsl_vector_memcpy( _y_out, other._y_out );
  }
  Layer& operator=(const Layer& other) 
  {
    if (this != &other) { // protect against invalid self-assignment
      if( _w ) gsl_matrix_free( _w );
      if( _y_out ) gsl_vector_free( _y_out );
      // copy
      _w = gsl_matrix_alloc( other._w->size1, other._w->size2 );
      gsl_matrix_memcpy( _w, other._w );
      _y_out = gsl_vector_calloc( other._y_out->size );
      gsl_vector_memcpy( _y_out, other._y_out );
    }
    return *this;
  }
  // ****************************************************** Layer::destructeur
  /** Destruction */
  virtual ~Layer()
  {
    gsl_matrix_free( _w );
    gsl_vector_free( _y_out );
  };
  // *************************************************************** operation
  Toutput forward( const Tinput& in )
  {
    // Verifie bonne taille
    if( (in.size()) != _w->size2 ) {
      std::cerr << "Layer.forward() : Wrong input size !" << std::endl;
      std::cerr << "                  in.size=" << in.size() << " != " << _w->size2 << std::endl;
    }

    // output
    Toutput result;
    
    // Into GSL : colomn matrix of in.size()+& row (for bias)
    TstatePtr v_input = gsl_vector_alloc(in.size() );
    for( unsigned int i = 0; i< in.size(); ++i) {
      gsl_vector_set( v_input, i, in[i]);
    }

    // W.X (y = 1.0 * _w * v_input + 0.0 * y);
    gsl_blas_dgemv(CblasNoTrans, 1.0, _w, v_input, 0.0, _y_out );

    // Libère la mémoire
    gsl_vector_free(v_input);

     // Et prépare output
    result.clear();
    for( unsigned int i = 0; i < _y_out->size; ++i) {
      result.push_back( gsl_vector_get( _y_out, i ));
    }
    return result;
  };
  // ***************************************************************** display
  std::string str_dump()
  {
    std::stringstream dump;
    dump << "__WEIGHTS__OUT__" << std::endl;
    dump << str_mat( _w );
    
    dump << "__STATE_OUT__" << std::endl;
    dump << str_vec( _y_out );
    
    return dump.str();
  };
  /** display */
  std::string str_display()
  {
    std::stringstream disp;
    disp << _w->size2 << " --> ";
    disp << _w->size1;

    return disp.str();
  };
  // *************************************************************** serialize
  rj::Value serialize( rj::Document& doc )
  {
    // rj::Object qui contient les données
    rj::Value obj;
    obj.SetObject();

    // Ajoute les paramètres
    // nb_in, nb_out
    obj.AddMember( "nb_input", rj::Value(_w->size2), doc.GetAllocator() );
    obj.AddMember( "nb_output", rj::Value(_w->size1), doc.GetAllocator() );
    // w sous forme d'array
    rj::Value ar;
    ar.SetArray();
    for( unsigned int i = 0; i < _w->size1; ++i) {
      for( unsigned int j = 0; j < _w->size2; ++j) {
	ar.PushBack( gsl_matrix_get(_w, i,j), doc.GetAllocator());
      }
    }
    obj.AddMember( "w", ar, doc.GetAllocator() );

    return obj;
  }
  void unserialize( const rapidjson::Value& obj )
  {
    // size
    Tinput_size nb_in =  obj["nb_input"].GetUint();
    Toutput_size nb_out =  obj["nb_output"].GetUint();
    // Matrix
    _w = gsl_matrix_calloc( nb_out, nb_in);
    _y_out = gsl_vector_calloc( nb_out );

    // Read Weights
    const rapidjson::Value& w = obj["w"];
    assert( w.IsArray() );
    rapidjson::SizeType idx = 0;
    for( unsigned int i = 0; i < _w->size1; ++i) {
      for( unsigned int j = 0; j < _w->size2; ++j) {
	assert(w[idx].IsNumber());
	gsl_matrix_set( _w, i, j, w[idx].GetDouble() );
	idx++;
      }
    }
  }
  // ******************************************************* Layer::read/write
  void write(std::ostream& os )
  {
    for( unsigned int i = 0; i < _w->size1; ++i) {
      for( unsigned int j = 0; j < _w->size2; ++j) {
	os << gsl_matrix_get(_w, i,j) << "\t";
      }
      os << std::endl;
    }
  }
  // ************************************************************** attributes
  TweightsPtr weights() { return _w; };
  Tinput_size input_size() const { return (Tinput_size) _w->size2; };
  Toutput_size output_size() const { return (Toutput_size) _w->size1; };
private:
  /** Weights */
  TweightsPtr _w;
  /** State (output) */
  TstatePtr _y_out;
  
};

#endif // LAYER_HPP

