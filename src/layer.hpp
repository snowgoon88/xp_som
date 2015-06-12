/* -*- coding: utf-8 -*- */

#ifndef LAYER_HPP
#define LAYER_HPP

/** 
 * Une couche (de sortie pour le Reservoir).
 */

#include <iostream>                     // std::cout
#include <sstream>                      // std::stringstream

#include <gsl/gsl_matrix.h>             // gsl Matrices
#include <gsl/gsl_blas.h>               // gsl matrix . vector multiplication

#include <utils_gsl.hpp>                // utils::str_vec, utils::str_mat ...
using namespace utils;
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
    _w(nullptr)
  {
    // Weights
    _w = gsl_matrix_calloc( output_size, input_size+1);
    // Output
    _y_out = gsl_vector_calloc( output_size );
  }
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
    if( (in.size()+1) != _w->size2 ) {
      std::cerr << "Layer.forward() : Wrong input size !" << std::endl;
      std::cerr << "                  in.size=" << in.size() << " != " << _w->size2 << std::endl;
    }

    // output
    Toutput result;
    
    // Into GSL : colomn matrix of in.size()+& row (for bias)
    TstatePtr v_input = gsl_vector_alloc(in.size()+1 );
    for( unsigned int i = 0; i< in.size(); ++i) {
      gsl_vector_set( v_input, i, in[i]);
    }
    gsl_vector_set( v_input, in.size(), 1.0);

    // W.X (_w * v_input);
    gsl_blas_dgemv(CblasNoTrans, 1.0, _w, v_input, 1.0, _y_out );

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
  // ************************************************************** attributes
  TweightsPtr weights() { return _w; };
private:
  /** Weights */
  TweightsPtr _w;
  /** State (output) */
  TstatePtr _y_out;
  
};

#endif // LAYER_HPP
