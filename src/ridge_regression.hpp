/* -*- coding: utf-8 -*- */

#ifndef RIDGE_REGRESSION_HPP
#define RIDGE_REGRESSION_HPP

/** 
 * A partir des Samples (x,y) ont construit X=(1;x) et Y=y.
 * Les Datas permettent de construire XX^T et YX^T.
 * Une RidgeRegression perment ensuite de trouver les poids optimaux
 *
 * ATTENTION : on peut ne pas pénaliser le poids associé à l'intercept.
 */

#include <iostream>                     // std::cout
#include <sstream>                      // std::stringstream
#include <vector>                       // std::vector
#include <limits>                       // std::numeric_limits

#include <gsl/gsl_matrix.h>             // gsl Matrices
#include <gsl/gsl_blas.h>               // gsl matrix . vector multiplication
#include <gsl/gsl_linalg.h>             // gsl Linag . LU Decomposition and inverse

// DEBUG
#include <utils.hpp>
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
      message = std::string("RidgeReg exception : ")
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
  using Tsize = unsigned int;
  typedef std::vector<double> Tinput;
  typedef std::vector<double> Toutput;

  typedef gsl_matrix*         TWeightsPtr;
  using TVectorPtr = gsl_vector*;
  
  typedef std::pair<Tinput,Toutput> Sample;
  typedef std::vector<Sample> Data;
  // **************************************************************** creation
  /**
   * @param int idx_intercept : index of weight for intercept. (-1 if none)
   */
  RidgeRegression( Tsize input_size, Tsize output_size,
		   int idx_intercept = -1 ) :
    _dim_x( input_size ), _dim_y( output_size ),
    _idx_intercept(idx_intercept),
    _xxt(nullptr), _yxt(nullptr),
    _mu_x(nullptr), _sd_x(nullptr), _mu_y(nullptr), _sd_y(nullptr)
  {
    // XX^T Matrix
    _xxt = gsl_matrix_calloc( input_size, input_size );
    // YX^T Matrix
    _yxt = gsl_matrix_calloc( output_size, input_size );
  };
  virtual ~RidgeRegression()
  {
    gsl_matrix_free( _xxt );
    gsl_matrix_free( _yxt );

    if( _mu_x ) {
      gsl_vector_free( _mu_x );
      gsl_vector_free( _sd_x );
      gsl_vector_free( _mu_y );
      gsl_vector_free( _sd_y );
    }
  };
  // *************************************************** RidgeReegression::learn
  /**
   * Given a Matrix M of N columns of vector, center every column.
   * col <- (col - mean(col) / sd(col)
   *
   * @return mu : vector of mean for each row
   * @return sd : vector of standard deviation for each row
   */
  void center_colmatrix( gsl_matrix* X, gsl_vector* mu, gsl_vector* sd )
  {
    // temporary vector, same size as column
    gsl_vector* tmp_sd = gsl_vector_calloc( X->size1 );
    
    // build X, mu and sd
    for( unsigned int col = 0; col < X->size2; ++col) {
      // Vector view of col
      auto vx = gsl_matrix_const_column( X, col );  // vector view
      //DEBUG std::cout << "VX.size=" << vx.vector.size << std::endl;
      // add to mu
      gsl_vector_add( mu, &(vx.vector) );              // addr of vector of vectorview
      // add square to sd
      gsl_vector_memcpy( tmp_sd, &(vx.vector) );
      gsl_vector_mul( tmp_sd, tmp_sd );
      gsl_vector_add( sd, tmp_sd );			
    }
    // now compute actual mu = 1/N sum( x_i )
    //DEBUG
    // std::cout << "SUM_mu= " << utils::gsl::str_vec( mu ) << std::endl;
    gsl_vector_scale( mu, 1.0 / (double) X->size2 );
    // and population sd = 1/(N-1) [sum (x_i * x_i ) - mu * N]
    gsl_vector_memcpy( tmp_sd, mu );
    gsl_vector_mul( tmp_sd, tmp_sd );
    gsl_vector_scale( tmp_sd, (double) X->size2 );
    gsl_vector_sub( sd, tmp_sd );
    gsl_vector_scale( sd, 1.0 / (double) (X->size2-1.0) );
    // and square root
    for( unsigned int i = 0; i < sd->size; ++i) {
      gsl_vector_set( sd, i, sqrt( gsl_vector_get( sd, i)));
    }

    // DEBUG
    // std::cout << "MEAN = " << utils::gsl::str_vec( mu ) << std::endl;
    // std::cout << "SD   = " << utils::gsl::str_vec( sd ) << std::endl;

    // Center X: every col <- (col-mu) / sd
    for( unsigned int col = 0; col < X->size2; ++col) {
      auto vx = gsl_matrix_column( X, col ).vector;
      gsl_vector_sub( &vx, mu );
      gsl_vector_div( &vx, sd );
    }
    
    // free temporary
    gsl_vector_free( tmp_sd );
  }
  /**
   * Hyp: Y = W.X where
   *   - Y : dim_y row x N col
   *   - X : dim_x row x N col
   *   - W : dim_y row x dim-x col
   *
   * @param data : a sequence of (x,y) learning data
   * @param w : ptr to a matrix with dim_y row x dim_x cols.
   * @param regul : regularization value
   *
   * @return : w
   */
  void center_and_learn( const Data& data,
			 TWeightsPtr w,
			 double regul )
  {
    // Memory allocation for Matrix
    gsl_matrix* X = gsl_matrix_calloc( _dim_x, data.size() );
    if( _mu_x ) gsl_vector_free( _mu_x );
    _mu_x = gsl_vector_calloc( _dim_x );
    if( _sd_x ) gsl_vector_free( _sd_x );
    _sd_x = gsl_vector_calloc( _dim_x );
    gsl_matrix* Y = gsl_matrix_calloc( _dim_y, data.size() );
    if( _mu_y ) gsl_vector_free( _mu_y );
    _mu_y = gsl_vector_calloc( _dim_y );
    if( _sd_y ) gsl_vector_free( _sd_y );
    _sd_y = gsl_vector_calloc( _dim_y );
    

    // Build X and Y
    auto id_sample = 0;
    for (auto it_s = data.begin(); it_s != data.end(); ++it_s, ++id_sample) {
      const auto x = it_s->first;
      const auto y = it_s->second;
      // set X
      auto id_x = 0;
      for (auto it_x = x.begin(); it_x != x.end(); ++it_x, ++id_x) {
    	gsl_matrix_set( X, id_x, id_sample, *it_x );
      }
      // set Y
      auto id_y = 0;
      for (auto it_y = y.begin(); it_y != y.end(); ++it_y, ++id_y) {
    	gsl_matrix_set( Y, id_y, id_sample, *it_y );
      }
    }
    // center X and Y
    center_colmatrix( X, _mu_x, _sd_x );
    center_colmatrix( Y, _mu_y, _sd_y );

    //DEBUG
    // std::cout << "X" << std::endl;
    // std::cout << utils::gsl::str_mat( X ) << std::endl;

    // Compute XX^T and YX^T
    // _xxt <- 1.0 * X * X^T + 0.0 * _xxt
    gsl_blas_dgemm( CblasNoTrans, CblasTrans, 1.0, X, X, 0.0, _xxt );
    // _yxt <- 1.0 * Y * Y^T + 0.0 * _yxt
    gsl_blas_dgemm( CblasNoTrans, CblasTrans, 1.0, Y, X, 0.0, _yxt );

    // (XX^T + regul.I)
    gsl_matrix* tmp_xxt = gsl_matrix_alloc( _xxt->size1, _xxt->size2 );
    gsl_permutation* perm = gsl_permutation_alloc( tmp_xxt->size1 );
    gsl_matrix* tmp_inv = gsl_matrix_alloc( _xxt->size1, _xxt->size2 );
    gsl_matrix_set_identity( tmp_xxt );
    gsl_matrix_scale( tmp_xxt, regul );
    gsl_matrix_add( tmp_xxt, _xxt );

    // LU decomposition for inversion
    int signum;
    gsl_linalg_LU_decomp( tmp_xxt, perm, &signum);
    gsl_linalg_LU_invert( tmp_xxt, perm, tmp_inv);
    // Produit final : w = YX^T * (XX^T + regul.I)^{-1}
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, _yxt, tmp_inv, 0.0, w );
    //std::cout << "w=" << str_mat(w) << std::endl;
    
    // Free memory
    gsl_matrix_free( X );
    gsl_matrix_free( Y );
    gsl_matrix_free( tmp_xxt );
    gsl_matrix_free( tmp_inv );
    gsl_permutation_free( perm );
  }
  // ********************************************************************* learn
  /**
   * Ridge regression with a given regul parameter.
   *
   * Returns IN w : best weights with given regul.
   */
  double learn( const Data& data, TWeightsPtr w, double regul )
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
      msg << " DIFF de input_size=" << _xxt->size1;
      throw Exception::Any( "SizeError", msg.str() );
    }

    // Compute XX^T and YX^T
    gsl_matrix* x = gsl_matrix_alloc( _xxt->size1, 1 );
    gsl_matrix* y = gsl_matrix_alloc( _yxt->size1, 1 );
    for( auto& sample: data) {
      // set X
      for( unsigned int i = 0; i < x->size1; ++i) {
	gsl_matrix_set( x, i, 0, sample.first[i] );
      }
	    
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

    // (XX^T + regul.I)
    gsl_matrix* tmp_xxt = gsl_matrix_alloc( _xxt->size1, _xxt->size2 );
    gsl_permutation* perm = gsl_permutation_alloc( tmp_xxt->size1 );
    gsl_matrix* tmp_inv = gsl_matrix_alloc( _xxt->size1, _xxt->size2 );
    gsl_matrix_set_identity( tmp_xxt );
    gsl_matrix_scale( tmp_xxt, regul );
    // Set regul to 0 for intercept weight
    if( _idx_intercept >= 0 ) {
      gsl_matrix_set( tmp_xxt, _idx_intercept, _idx_intercept, 0.0 );
    }
    gsl_matrix_add( tmp_xxt, _xxt );
    // LU decomposition for inversion

    int signum;
    gsl_linalg_LU_decomp( tmp_xxt, perm, &signum);
    gsl_linalg_LU_invert( tmp_xxt, perm, tmp_inv);
    // Produit final : w = YX^T * (XX^T + regul.I)^{-1}
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, _yxt, tmp_inv, 0.0, w );
    //std::cout << "w=" << str_mat(w) << std::endl;

    // Critère d'erreur
    double error = 0;
    // regul * ||w||^2
    for( unsigned row = 0; row < w->size1; ++row ) {
      
      gsl_vector_view vrow = gsl_matrix_row (w, row);
      // Set error to 0 for intercept weight
      if( _idx_intercept >= 0 ) {
	gsl_vector_set( &vrow.vector, _idx_intercept, 0.0 );
      }

      error += gsl_blas_dnrm2( &vrow.vector );
    }
    error *= regul;
    // std::cout << "regul*||W||^2=" << error << std::endl;
    // std::cout << "Target\tY\tError" << std::endl;
    for( auto& sample: data) {
      // set X
      for( unsigned int i = 0; i < x->size1; ++i) {
	gsl_matrix_set( x, i, 0, sample.first[i] );
      }
      // set Y	
      gsl_blas_dgemm (CblasNoTrans,  CblasNoTrans, 1.0, w, x, 0.0, y);
      // error
      for( unsigned int i = 0; i < y->size1; ++i) {
	error += pow( (gsl_matrix_get( y, i, 0 ) - sample.second[i]), 2);
	// std::cout << sample.second[i]<<"\t"<< gsl_matrix_get(y,i,0)<<"\t"<<error<< std::endl;
      }
    }
    // std::cout << "___ err=" << error << " avec regul=" << regul << std::endl;
    
    // Libération mémoire
    gsl_matrix_free( x );
    gsl_matrix_free( y );
    gsl_matrix_free( tmp_xxt );
    gsl_matrix_free( tmp_inv );
    gsl_permutation_free( perm );

    return error;
  }
  /** 
   * Optimize _regul parameters by minimizing regularized Risk.
   */
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
      msg << " DIFF de input_size=" << _xxt->size1;
      throw Exception::Any( "SizeError", msg.str() );
    }
    
    // Compute XX^T and YX^T
    gsl_matrix* x = gsl_matrix_alloc( _xxt->size1, 1 );
    gsl_matrix* y = gsl_matrix_alloc( _yxt->size1, 1 );
    for( auto& sample: data) {
      // set X
      for( unsigned int i = 0; i < x->size1; ++i) {
	gsl_matrix_set( x, i, 0, sample.first[i] );
      }
	    
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
    for( double regul = best_regul; regul <= 2.0; regul += 0.1 ) {
      // (XX^T + _regul.I)
      gsl_matrix_set_identity( tmp_xxt );
      gsl_matrix_scale( tmp_xxt, exp10(regul) );
      // Set regul to 0 for intercept weight
      if( _idx_intercept >= 0 ) {
	gsl_matrix_set( tmp_xxt, _idx_intercept, _idx_intercept, 0.0 );
      }

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
      // regul * ||w||^2
      for( unsigned row = 0; row < w->size1; ++row ) {
	
	gsl_vector_view vrow = gsl_matrix_row (w, row);
	// Set error to 0 for intercept weight
	if( _idx_intercept >= 0 ) {
	  gsl_vector_set( &vrow.vector, _idx_intercept, 0.0 );
	}
	error += gsl_blas_dnrm2( &vrow.vector );
      }
      error *= exp10(regul);
      // std::cout << "regul*||W||^2=" << error << std::endl;
      // std::cout << "Target\tY\tError" << std::endl;
      for( auto& sample: data) {
	// set X
	for( unsigned int i = 0; i < x->size1; ++i) {
	  gsl_matrix_set( x, i, 0, sample.first[i] );
	}
	// set Y	
	gsl_blas_dgemm (CblasNoTrans,  CblasNoTrans, 1.0, w, x, 0.0, y);
	// error
	for( unsigned int i = 0; i < y->size1; ++i) {
	  error += pow( (gsl_matrix_get( y, i, 0 ) - sample.second[i]), 2);
	  // std::cout << sample.second[i]<<"\t"<< gsl_matrix_get(y,i,0)<<"\t"<<error<< std::endl;
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
public:
  TVectorPtr mu_x() { return _mu_x; };
  TVectorPtr sd_x() { return _sd_x; };
  TVectorPtr mu_y() { return _mu_y; };
  TVectorPtr sd_y() { return _sd_y; };
private:
  /** Data dimensions */
  Tsize _dim_x, _dim_y;
  /** Possible index of the weight of the 'intercept' */
  int _idx_intercept;
  /** internal matrices */
  TWeightsPtr _xxt, _yxt;
  TVectorPtr _mu_x, _sd_x, _mu_y, _sd_y;
};

#endif // RIDGE_REGRESSION_HPP
