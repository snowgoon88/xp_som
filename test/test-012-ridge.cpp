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

//******************************************************************************
int main( int argc, char *argv[] )
{
  Reservoir res( 1, 5, 0.2);
  res.set_spectral_radius( 0.9 );
  std::cout << "***** RESERVOIR **" << "\n";
  std::cout << res.str_dump() << std::endl;

  Layer lay( 5, 1 );
  std::cout << "***** LAYER **" << std::endl;
  std::cout << lay.str_dump() << std::endl;

  // Mackeyglass
  MackeyGlass::Data mg_data;
  std::ifstream ifile(DATA_FILE);
  MackeyGlass::read( ifile, mg_data);
  ifile.close();

  // Préparer les données d'apprentissage (y, target)
  RidgeRegression::Data learn_data;
  // y = Reservoir( mg_data[i-1]) =?= mg_data[i]
  Reservoir::Tinput in;
  for( unsigned int i = 1; i < mg_data.size(); ++i) {
    // Prépare input
    in.clear();
    in.push_back( mg_data[i-1] );
    // Passe dans réservoir
    auto out_res = res.forward( in );
    // Prépare target
    RidgeRegression::Toutput target;
    target.push_back( mg_data[i] );

    // Ajoute dans Data
    learn_data.push_back( RidgeRegression::Sample( out_res, target) );
  }
  // Ridge Regression
  RidgeRegression reg( 5, 1, 1.0 );
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
  
  return 0;
}
