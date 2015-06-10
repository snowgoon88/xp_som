/* -*- coding: utf-8 -*- */

/**
 * test-011-esn.cpp
 * 
 * Crée un Reservoir+Layer et fait forward
 */

#include <iostream>       // std::cout
#include <fstream>        // std::ofstream
#include <reservoir.hpp>
#include <layer.hpp>

// ******************************************************************** Global
//******************************************************************************
int main( int argc, char *argv[] )
{
  Reservoir res( 2, 3, 0.2);
  res.set_spectral_radius( 0.1 );
  std::cout << "***** RESERVOIR **" << "\n";
  std::cout << res.str_dump() << std::endl;

  Layer lay( 3, 1 );
  std::cout << "***** LAYER **" << std::endl;
  std::cout << lay.str_dump() << std::endl;

  // try Input
  Reservoir::Tinput in = {1.2, -2.0};
  auto out_res = res.forward( in );
  std::cout << "out_res = ";
  for( auto& v: out_res) {
    std::cout << v << "; ";
  }
  std::cout << std::endl;
  
  auto out = lay.forward( out_res );
  std::cout << "out = ";
  for( auto& v: out) {
    std::cout << v << "; ";
  }
  std::cout << std::endl;

  // TODO rassembler les sorties de Réservoir
  // TODO mettre les sorties désirées (Makeyglass)
  // TODO apprendre avec RIDGE
  
  return 0;
}
