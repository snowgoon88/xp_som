/* -*- coding: utf-8 -*- */

#ifndef RIDGE_REGRESSION_HPP
#define RIDGE_REGRESSION_HPP

/** 
 * A partir des Samples (x,y) ont construit X=(1;x) et Y=y.
 * Les Datas permettent de construire XX^T et YX^T.
 * Une RidgeRegression perment ensuite de trouver les poids optimaux
 */

#include <iostream>                     // std::cout
#include <sstream>                      // std::stringstream
#include <vector>                       // std::vector
#include <limits>                       // std::numeric_limits

#include <gsl/gsl_matrix.h>             // gsl Matrices
#include <gsl/gsl_blas.h>               // gsl matrix . vector multiplication
#include <gsl/gsl_linalg.h>             // gsl Linag . LU Decomposition and inverse

// ***************************************************************************
// ***************************************************************** Exception
// ***************************************************************************
namespace Exception {
  
  class Any : public std::exception {
  private:
    std::string message;
  public:
    Any(const std::string& kind,
	const std::string& msg) {
      message = std::string("GAML-mlp exception : ")
	+ kind + " : " + msg;
    }
    
    virtual ~Any(void) throw () {}
    
    virtual const char * what(void) const throw ()
    {
      return message.c_str();
    }
  };
};

// ***************************************************************************
// *********************************************************** RidgeRegression
// ***************************************************************************
class RidgeRegression
{
public:
  typedef unsigned int Tinput_size;
  typedef std::vector<double> Tinput;
  typedef unsigned int Toutput_size;
  typedef std::vector<double> Toutput;

  typedef gsl_matrix*         TWeightsPtr;
  
  typedef std::pair<Tinput,Toutput> Sample;
  typedef std::vector<Sample> Data;
  // **************************************************************** creation
  RidgeRegression( Tinput_size input_size, Toutput_size output_size,
		   double regul = 1.0 ) :
    _regul(regul),
    _xxt(nullptr), _yxt(nullptr)
  {
    // XX^T Matrix
    _xxt = gsl_matrix_calloc( input_size+1, input_size+1 );
    // YX^T Matrix
    _yxt = gsl_matrix_calloc( output_size, input_size+1 );
  };
  virtual ~RidgeRegression()
  {
    gsl_matrix_free( _xxt );
    gsl_matrix_free( _yxt );
  };
  // ********************************************************************* learn
  void learn( const Data& data, TWeightsPtr w )
  {
    // Check dimensions of w
    if( w->size1 != _yxt->size1 ) {
      std::stringstream msg;
      msg << "w->size1=" << w->size1;
      msg << " DIFF de output_size=" << _yxt->size1;
      throw Exception::Any( "SizeError", msg.str() );
    }
    if( w->size2 != _xxt->size1 ) {
      std::stringstream msg;
      msg << "w->size2=" << w->size2;
      msg << " DIFF de input_size+1=" << _xxt->size1;
      throw Exception::Any( "SizeError", msg.str() );
    }
    
    // Compute XX^T and YX^T
    gsl_matrix* x = gsl_matrix_alloc( _xxt->size1, 1 );
    gsl_matrix* y = gsl_matrix_alloc( _yxt->size1, 1 );
    for( auto& sample: data) {
      // set X
      for( unsigned int i = 0; i < x->size1-1; ++i) {
	gsl_matrix_set( x, i, 0, sample.first[i] );
      }
      gsl_matrix_set( x, x->size1-1, 0, 1.0 );
	    
      // set Y
      for( unsigned int i = 0; i < y->size1; ++i) {
	gsl_matrix_set( y, i, 0, sample.second[i] );
      }

      // Add to XX^T
      // _xxt = x.x^T + 1.0 * _xxt
      gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, x, x, 1.0, _xxt);
      // Add to YX^T
      gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, y, x, 1.0, _yxt);
    }

    // (XX^T + _regul.I)
    gsl_matrix* tmp_xxt = gsl_matrix_alloc( _xxt->size1, _xxt->size2 );
    gsl_permutation* perm = gsl_permutation_alloc( tmp_xxt->size1 );
    gsl_matrix* tmp_inv = gsl_matrix_alloc( _xxt->size1, _xxt->size2 );
    
    // TODO => Chercher le meilleur _regul sur échelle logarithmique.
    double best_regul = -12.0;
    double best_error = std::numeric_limits<double>::infinity();
    for( double regul = best_regul; regul <= -11.9; regul += 0.1 ) {
      // (XX^T + _regul.I)
      gsl_matrix_set_identity( tmp_xxt );
      gsl_matrix_scale( tmp_xxt, exp10(regul) );
      gsl_matrix_add( tmp_xxt, _xxt );
      // LU decomposition for inversion
      int signum;
      gsl_linalg_LU_decomp( tmp_xxt, perm, &signum);
      gsl_linalg_LU_invert( tmp_xxt, perm, tmp_inv);
      // Produit final
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, _yxt, tmp_inv, 0.0, w );
      std::cout << "w=" << str_mat(w) << std::endl;
      
      // Critère d'erreur
      double error = 0;
      std::cout << "Target\tY\tError" << std::endl;
      for( auto& sample: data) {
	// set X
	for( unsigned int i = 0; i < x->size1-1; ++i) {
	  gsl_matrix_set( x, i, 0, sample.first[i] );
	}
	gsl_matrix_set( x, x->size1-1, 0, 1.0 );
	// set Y	
	gsl_blas_dgemm (CblasNoTrans,  CblasNoTrans, 1.0, w, x, 0.0, y);
	// error
	for( unsigned int i = 0; i < y->size1; ++i) {
	  error += pow( (gsl_matrix_get( y, i, 0 ) - sample.second[i]), 2);
	  std::cout << sample.second[i]<<"\t"<< gsl_matrix_get(y,i,0)<<"\t"<<error<< std::endl;
	}
      }
      std::cout << "regul = " << exp10(regul) << " avec err= " << error << std::endl;
      if( error < best_error ) {
	best_regul = exp10(regul);
	best_error = error;
      }
    }
    std::cout << "MEILLEUR REGUL= " << best_regul << " avec err= " << best_error << std::endl;
    gsl_matrix_set_identity( tmp_xxt );
    gsl_matrix_scale( tmp_xxt, best_regul );
    gsl_matrix_add( tmp_xxt, _xxt );
    // LU decomposition for inversion
    int signum;
    gsl_linalg_LU_decomp( tmp_xxt, perm, &signum);
    gsl_linalg_LU_invert( tmp_xxt, perm, tmp_inv);
    // Produit final
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, _yxt, tmp_inv, 0.0, w );

    // Libération mémoire
    gsl_matrix_free( x );
    gsl_matrix_free( y );
    gsl_matrix_free( tmp_xxt );
    gsl_matrix_free( tmp_inv );
    gsl_permutation_free( perm );
  };
  // *************************************************************** attributs
private:
  /** Regulation coefficients */
  double _regul;
  /** internal matrices */
  TWeightsPtr _xxt, _yxt;
};

#endif // RIDGE_REGRESSION_HPP
