/* -*- coding: utf-8 -*- */

#pragma once

/** 
 * Création d'un réservoir random, seed depends on std::time()
 */

#include <iostream>          // std::cout
#include <sstream>           // std::stringstream

#include <gsl/gsl_rng.h>     // gsl random generator
#include <gsl/gsl_matrix.h>  // gsl Matrices
#include <gsl/gsl_eigen.h>   // gsl EigenValues
#include <gsl/gsl_complex_math.h>// gsl complex
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
    gsl_rng_set( _rnd, std::time( NULL ) );

    // Matrix _output_size lines of _input_size columns
    _w_in = gsl_matrix_alloc( output_size, input_size);
    for( unsigned int i = 0; i < _w_in->size1; ++i) {
      for( unsigned int j = 0; j < _w_in->size2; ++j) {
	gsl_matrix_set( _w_in, i, j, gsl_rng_uniform_pos(_rnd)-0.5 );
      }
    }
    // Matrix _output_size lines of _output_size columns
    _w_res = gsl_matrix_alloc( output_size, output_size);
    for( unsigned int i = 0; i < _w_res->size1; ++i) {
      for( unsigned int j = 0; j < _w_res->size2; ++j) {
	gsl_matrix_set( _w_res, i, j, gsl_rng_uniform_pos(_rnd)-0.5 );
      }
    }
  }
  /** Destruction */
  ~Reservoir()
  {
    gsl_rng_free( _rnd );
    gsl_matrix_free( _w_res );
  }

  // ********************************************************************** INIT
  void set_spectral_radius( double radius )
  {
    // Compute spectral radius
    // Copier la matrice des poids
    gsl_matrix* _w_tmp = gsl_matrix_alloc( _w_res->size1, _w_res->size2 );
    gsl_matrix_memcpy( _w_tmp, _w_res);
    // Espace pour valeurs propres
    gsl_vector_complex* eval = gsl_vector_complex_alloc(_w_res->size1);
    // if( eval == NULL ) {
    //   std::cerr << "set_spectral_radius: Allocation of 'eval' failed" << "\n";
    // }
    // else {
    //   std::cout << "set_spectral_radius: 'eval' allocated" << "\n";
    // }
    // Espace pour calcul valeur propres
    gsl_eigen_nonsymm_workspace* work = gsl_eigen_nonsymm_alloc(_w_res->size1);
    // if( work == NULL ) {
    //   std::cerr << "set_spectral_radius: Allocation of 'work' Failed" << "\n";
    // }
    // else {
    //   std::cout << "set_spectral_radius: 'work' allocated" << "\n";
    // }
    gsl_eigen_nonsymm( _w_tmp, eval, work);

    // Liste les valeurs pour trouver le max
    double abs_max = 0.0;
    for( unsigned int i = 0; i < _w_res->size1; ++i) {
      // double real_eval_i = GSL_REAL(gsl_vector_complex_get (eval, i));
      // double imag_eval_i = GSL_IMAG(gsl_vector_complex_get (eval, i));
      double mag = gsl_complex_abs( gsl_vector_complex_get (eval, i));
      // std::cout << "EigenVal[" << i << "]= " << real_eval_i << " +i " << imag_eval_i << " ||=" << mag << std::endl;
      if( mag > abs_max ) {
	abs_max = mag;
      }
    }
    // Et divise tous les éléements de _w_res
    gsl_matrix_scale( _w_res, radius/abs_max );
    // Libère l'espace
    gsl_eigen_nonsymm_free( work );
    gsl_vector_complex_free( eval );
    gsl_matrix_free( _w_tmp );
  }
  
  // ******************************************************************* DISPLAY
  /** display string */
  std::string str_dump()
  {
    std::stringstream dump;
    dump << "__WEIGHTS_IN__" << std::endl;
    for( unsigned int i = 0; i < _w_in->size1; ++i) {
      for( unsigned int j = 0; j < _w_in->size2; ++j) {
	dump << gsl_matrix_get( _w_in, i, j ) << "; ";
      }
      dump << std::endl;
    }
    dump << "__WEIGHTS_RESERVOIR__" << std::endl;
    for( unsigned int i = 0; i < _w_res->size1; ++i) {
      for( unsigned int j = 0; j < _w_res->size2; ++j) {
	dump << gsl_matrix_get( _w_res, i, j ) << "; ";
      }
      dump << std::endl;
    }
    return dump.str();
  };
  
  
  
private:
  //Tinput_size _input_size;
  //Toutput_size _output_size;
  /** Input weights */
  Tweights _w_in;
  /** Reservoir weights */
  Tweights _w_res;
  
  /** Parameters */
  double _input_scaling;
  double _spectral_radius;
  double _leaking_rate;
  /** Random generator */
  gsl_rng* _rnd;
};
