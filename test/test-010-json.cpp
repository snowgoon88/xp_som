/* -*- coding: utf-8 -*- */

/**
 * test-010-json.cpp
 * 
 * Crée un Reservoir et le serialise.
 */

#include <iostream>       // std::cout
#include <fstream>        // std::ofstream
#include <reservoir.hpp>

// ******************************************************************** Global
#define RES_FILE "reservoir.data"

//******************************************************************************
int main( int argc, char *argv[] )
{
  Reservoir res( 2, 3, 0.2);
  res.set_spectral_radius( 0.1 );
  std::cout << "***** RESERVOIR **" << "\n";
  std::cout << res.str_dump() << std::endl;

  std::cout << "***** JSON *******" << std::endl;
  res.serialize( std::cout );
  std::cout << std::endl;

  // Write un a file
  std::ofstream ofile(RES_FILE);
  res.serialize( ofile );
  ofile.close();

  // TODO Read from file
  std::ifstream ifile(RES_FILE);
  Reservoir res_read( ifile );
  std::cout << "***** RES_READ ***" << "\n";
  std::cout << res_read.str_dump() << std::endl;
  std::cout << "***** RES_end ****" << "\n";
  
  return 0;
}
