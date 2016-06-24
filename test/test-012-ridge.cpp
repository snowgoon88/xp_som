/* -*- coding: utf-8 -*- */

/**
 * test-012-ridge.cpp
 * 
 * Crée un Reservoir+Layer et fait forward, puis regression.
 */

#include <iostream>       // std::cout
#include <fstream>        // std::ofstream

#include <reservoir.hpp>
#include <layer.hpp>
#include <mackeyglass.hpp>
#include <ridge_regression.hpp>

// ******************************************************************** Global
#define DATA_FILE "mackeyglass.data"


// ***************************************************************************
void test_mackey()
{
  Reservoir res( 1, 5, 0.2);
  res.set_spectral_radius( 0.9 );
  std::cout << "***** RESERVOIR **" << "\n";
  std::cout << res.str_dump() << std::endl;

  Layer lay( 5+1, 1 );
  std::cout << "***** LAYER **" << std::endl;
  std::cout << lay.str_dump() << std::endl;

  // Mackeyglass
  MackeyGlass::Data mg_data;
  std::ifstream ifile(DATA_FILE);
  MackeyGlass::read( ifile, mg_data);
  ifile.close();

  // Préparer les données d'apprentissage ([y;1.0], target)
  RidgeRegression::Data learn_data;
  // y = Reservoir( mg_data[i-1]) =?= mg_data[i]
  Reservoir::Tinput in;
  for( unsigned int i = 1; i < mg_data.size(); ++i) {
    // Prépare input
    in.clear();
    in.push_back( mg_data[i-1] );
    // Passe dans réservoir
    auto out_res = res.forward( in );
    // Ajoute 1.0 en bout
    out_res.push_back( 1.0 );
    
    // Prépare target
    RidgeRegression::Toutput target;
    target.push_back( mg_data[i] );

    // Ajoute dans Data
    learn_data.push_back( RidgeRegression::Sample( out_res, target) );
  }
  // Ridge Regression
  RidgeRegression reg( 5+1, 1, 1.0 );
  reg.learn( learn_data, lay.weights() );
  std::cout << "***** REGRESSION **" << std::endl;
  std::cout << lay.str_dump() << std::endl;

  std::cout << "***** RESULTS **" << std::endl;
  // Pour vérifier, on affiche Mackeyglass ESN_après RIDGE
  std::cout << "Mackey \t\tOutput" << std::endl;
  for( auto& sample: learn_data) {
    // Target
    std::cout << sample.second[0];

    // Valeur en sortie de Layer
    auto out_layer = lay.forward( sample.first );
    std::cout << "\t" << out_layer[0] << std::endl;
  }
}
// ***************************************************************************
/**
 * test with data from dbg_learn_data 
 */
void test_learndata()
{
  RidgeRegression::Data data;
  
  // Read data
  std::ifstream ifile( "dbg_learn_data" );
  // Lit en omettant les lignes commençant par '#'
  std::string line = "";
  while (!ifile.eof()) {
    std::getline( ifile, line );
    //DEBUG std::cout << "LINE: " << line << std::endl;
    // remove also the ine beginning win 'in_0'. Should test for header.
    if( line.front() != '#' and line.front() != 'i') { // ADHOC !!!!
      std::stringstream iss( line );
      // skip in_0, read in_1:9 into input and tar_0 into output
      RidgeRegression::Tinput input;
      RidgeRegression::Toutput output;
      auto number = 0.0;
      iss >> number; // in_0
      for( unsigned int i = 0; i < 10; ++i) {
	iss >> number;
	input.push_back( number );
      }
      iss >> number;
      output.push_back( number );
      
      // en testant !eof, on évite de lire le dernier endl comme un float...
      if( !ifile.eof()) {
	data.push_back( RidgeRegression::Sample( input, output ) );
      }
    }
  }
  // DEBUG
  for( const auto& samp:  data) {
    std::cout << utils::str_vec(samp.first) << " -> " << utils::str_vec(samp.second) << std::endl;
  }

  // RIDGE
  auto reg = RidgeRegression( 10, 1, 0);
  RidgeRegression::TWeightsPtr w = gsl_matrix_calloc( 1, 10 );
  reg.center_and_learn( data, w, 1.0 );

  // PRINT OUT
  std::cout << "W:" << std::endl;
  std::cout << utils::gsl::str_mat( w ) << std::endl;

  // Build the proper Layer Weight matrix
  RidgeRegression::TWeightsPtr lay = gsl_matrix_calloc( 1, 10+1 );
  // weight submatrix
  auto lay_w = gsl_matrix_submatrix( lay, 0, 0, 1, 10 ).matrix;
  // lay_w <- copy( w )
  gsl_matrix_memcpy( &lay_w, w);
  // divide every row by vec_sd_x
  for( unsigned int row = 0; row < lay_w.size1; ++row) {
    auto vrlay = gsl_matrix_row( &lay_w, row ).vector;
    gsl_vector_div( &vrlay, reg.sd_x() );
  }
  // multiply every column by vec sd_y
  for( unsigned int col = 0; col < lay_w.size2; ++col) {
    auto vclay = gsl_matrix_column( &lay_w, col ).vector;
    gsl_vector_mul( &vclay, reg.sd_y() );
  }
  
  // PRINT OUT
  std::cout << "RESCALED LAY_w:" << std::endl;
  std::cout << utils::gsl::str_mat( lay ) << std::endl;

  // constant components : lay last column
  auto vbias = gsl_matrix_column( lay, lay->size2-1 ).vector;
  gsl_blas_dgemv( CblasNoTrans, -1.0, &lay_w, reg.mu_x(), 0.0, &vbias );
  gsl_vector_add( &vbias, reg.mu_y() );

  
  // PRINT OUT
  std::cout << "RESCALED LAY WITH BIAS:" << std::endl;
  std::cout << utils::gsl::str_mat( lay ) << std::endl;

  // Compute prediction
  for( auto& sample: data) {
    auto x = sample.first;
    auto vx = gsl_vector_calloc( x.size() + 1 );
    auto vy = gsl_vector_calloc( sample.second.size() );
    auto id_x = 0;
    for (auto it = x.begin(); it != x.end(); ++it, ++id_x) {
      gsl_vector_set( vx, id_x, *it );
    }
    gsl_vector_set( vx, vx->size-1, 1.0 );

    gsl_blas_dgemv(CblasNoTrans, 1.0, lay, vx, 0.0, vy );
    std::cout << utils::gsl::str_vec( vy) << " =?= " << utils::str_vec( sample.second ) << std::endl;

    gsl_vector_free( vx );
    gsl_vector_free( vy );
  }

  gsl_matrix_free( lay );
  gsl_matrix_free( w );
}
//******************************************************************************
int main( int argc, char *argv[] )
{
  test_learndata();
  return 0;
}
