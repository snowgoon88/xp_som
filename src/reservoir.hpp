/* -*- coding: utf-8 -*- */

#pragma once

/** 
 * Création d'un réservoir random, seed depends on std::time()
 */

#include <iostream>          // std::cout
#include <sstream>           // std::stringstream

#include <gsl/gsl_rng.h>     // gsl random generator
#include <gsl/gsl_matrix.h>  // gsl Matrices
#include <ctime>             // std::time

class Reservoir
{
public:
  //typedef std::vector<double> Tinput;
  typedef unsigned int        Tinput_size;
  //typedef std::vector<double> Toutput;
  typedef unsigned int        Toutput_size;
  typedef gsl_matrix*          Tweights;

  // ****************************************************************** CREATION
  /** Creation */
  Reservoir( Tinput_size input_size, Toutput_size output_size ) :
    _input_scaling( 1.0 ), _spectral_radius (1.0 ), _leaking_rate( 1.0 )
  {
    // Random generator with seed = time
    _rnd = gsl_rng_alloc( gsl_rng_taus );
    
    // Matrix _output_size lines of _input_size columns
    _weights = gsl_matrix_alloc( output_size, input_size);
    for( unsigned int i = 0; i < _weights->size1; ++i) {
      for( unsigned int j = 0; j < _weights->size2; ++j) {
	gsl_matrix_set( _weights, i, j, gsl_rng_uniform_pos(_rnd)-0.5 );
      }
    }
  }
  /** Destruction */
  ~Reservoir()
  {
    gsl_rng_free( _rnd );
    gsl_matrix_free( _weights );
  }

  // ********************************************************************** INIT
  void set_spectral_radius()
  {
    // Compute spectral radius
    
  }
  
  // ******************************************************************* DISPLAY
  /** display string */
  std::string str_dump()
  {
    std::stringstream dump;
    for( unsigned int i = 0; i < _weights->size1; ++i) {
      for( unsigned int j = 0; j < _weights->size2; ++j) {
	dump << gsl_matrix_get( _weights, i, j ) << "; ";
      }
      dump << std::endl;
    }
    return dump.str();
  };
  
  
  
private:
  Tinput_size _input_size;
  Toutput_size _output_size;
  Tweights _weights;
  
  /** Parameters */
  double _input_scaling;
  double _spectral_radius;
  double _leaking_rate;
  /** Random generator */
  gsl_rng* _rnd;
};
