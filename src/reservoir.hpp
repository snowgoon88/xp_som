/* -*- coding: utf-8 -*- */

#pragma once

/** 
 * Création d'un réservoir random et uniform, seed depends on std::time().
 *
 * @param _input_scaling : input weights in [-input_scaling,input_sclaing]
 */

#include <iostream>                     // std::cout
#include <sstream>                      // std::stringstream

#include <gsl/gsl_rng.h>                // gsl random generator
#include <gsl/gsl_matrix.h>             // gsl Matrices
#include <gsl/gsl_eigen.h>              // gsl EigenValues
#include <gsl/gsl_complex_math.h>       // gsl complex
#include <gsl/gsl_blas.h>               // gsl matrix . vector multiplication

#include <ctime>                        // std::time
#include <vector>                       // std::vector
#include <math.h>                       // tanh
// ***************************************************************************
// ***************************************************************** RESERVOIR
// ***************************************************************************
class Reservoir
{
public:
  typedef std::vector<double> Tinput;
  typedef unsigned int        Tinput_size;
  //typedef std::vector<double> Toutput;
  typedef unsigned int        Toutput_size;
  typedef gsl_matrix*         Tweights;
  typedef gsl_vector*         Tstate;

// ****************************************************************** CREATION
/** Creation */
  Reservoir( Tinput_size input_size, Toutput_size output_size,
	     double input_scaling = 1.0,
	     double spectral_radius= 0.99,
	     double leaking_rate = 0.1 ) :
    _input_scaling(input_scaling), _spectral_radius(spectral_radius),
    _leaking_rate(leaking_rate)
  {
    // Random generator with seed = time
    _rnd = gsl_rng_alloc( gsl_rng_taus );
    gsl_rng_set( _rnd, std::time( NULL ) );
    
    // INPUT_WEIGHTS : Matrix _output_size lines of _input_size+1 columns
    // in [- _input_scaling, _input_scaling]
    _w_in = gsl_matrix_alloc( output_size, input_size+1);
    for( unsigned int i = 0; i < _w_in->size1; ++i) {
      for( unsigned int j = 0; j < _w_in->size2; ++j) {
	gsl_matrix_set( _w_in, i, j, (gsl_rng_uniform_pos(_rnd)-0.5) * _input_scaling / 0.5 );
      }
    }
    // RESERVOIR_WEIGHTS Matrix _output_size lines of _output_size columns
    // in [-0.5, 0.5] before spectral radius
    _w_res = gsl_matrix_alloc( output_size, output_size);
    for( unsigned int i = 0; i < _w_res->size1; ++i) {
      for( unsigned int j = 0; j < _w_res->size2; ++j) {
	gsl_matrix_set( _w_res, i, j, gsl_rng_uniform_pos(_rnd)-0.5 );
      }
    }
    set_spectral_radius( _spectral_radius );

    // RESERVOIR STATE : initialisé à 0
    _x_res = gsl_vector_calloc( output_size );
  }
  /** Destruction */
  ~Reservoir()
  {
    gsl_rng_free( _rnd );
    gsl_matrix_free( _w_res );
    gsl_matrix_free( _w_in );
    gsl_vector_free( _x_res );
  }
  // ***************************************************************** OPERATION
  void forward( const Tinput& in )
  {
    // Verifie bonne taille
    if( (in.size()+1) != _w_in->size2 ) {
      std::cerr << "Reservoir.forward() : Wrong input size !" << std::endl;
      std::cerr << "                      in.size=" << in.size() << " != " << _w_in->size2 << std::endl;
    }
    // Into GSL : colomn matrix of in.size()+& row (for bias)
    Tstate v_input = gsl_vector_alloc(in.size()+1 );
    for( unsigned int i = 0; i< in.size(); ++i) {
      gsl_vector_set( v_input, i, in[i]);
    }
    gsl_vector_set( v_input, in.size(), 1.0);

    std::cout<< "IN= {" << str_vec(v_input) << std::endl;
    
    // Computation
    Tstate v_tmp = gsl_vector_calloc( _x_res->size );
    std::cout<< "TMP= {" << str_vec(v_tmp) << std::endl;
    // _w_in * input + _w_res * _x_res
    gsl_blas_dgemv(CblasNoTrans, 1.0, _w_res, _x_res, 0.0, v_tmp);  // tmp <- w_res * x_res
    std::cout<< "TMP= {" << str_vec(v_tmp) << std::endl;
    gsl_blas_dgemv(CblasNoTrans, 1.0, _w_in, v_input, 1.0, v_tmp); // tmp <- _w_in * v_in + tmp
    std::cout<< "XN= {" << str_vec(v_tmp) << std::endl;
    // tanh
    for( unsigned int i = 0; i< v_tmp->size; ++i) {
      gsl_vector_set( v_tmp, i, tanh( gsl_vector_get( v_tmp, i)));
    }
    std::cout<< "tanh(XN)= {" << str_vec(v_tmp) << std::endl;
    // x = (1-alpha) x + alpha xtilde
    gsl_vector_scale( v_tmp, _leaking_rate );
    gsl_vector_scale( _x_res, 1.0 - _leaking_rate );
    gsl_vector_add( _x_res, v_tmp);
    std::cout<< "X= {" << str_vec(_x_res) << std::endl;

    // Libère la mémoire
    gsl_vector_free(v_input);
    gsl_vector_free(v_tmp);
    
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
    dump << str_mat( _w_in );

    dump << "__WEIGHTS_RESERVOIR__" << std::endl;
    dump << str_mat( _w_res );

    return dump.str();
  };
  /** display vector */
  std::string str_vec(const Tstate v)
  {
    std::stringstream str;
    for( unsigned int i = 0; i< v->size; ++i) {
      str << gsl_vector_get(v, i) << "; ";
    }
    str << "}";
    return str.str();
  }
  /** display matrix */
  std::string str_mat(const Tweights m)
  {
    std::stringstream str;
    for( unsigned int i = 0; i < m->size1; ++i) {
      for( unsigned int j = 0; j < m->size2; ++j) {
	str << gsl_matrix_get( m, i, j ) << "; ";
      }
      str << std::endl;
    }
    return str.str();
  }
  
  // **************************************************************** ATTRIBUTES
private:
  //Tinput_size _input_size;
  //Toutput_size _output_size;
  /** Input weights */
  Tweights _w_in;
  /** Reservoir weights */
  Tweights _w_res;
  /** Reservoir state */
  Tstate _x_res;
  
  /** Parameters */
  double _input_scaling;
  double _spectral_radius;
  double _leaking_rate;
  /** Random generator */
  gsl_rng* _rnd;
};
