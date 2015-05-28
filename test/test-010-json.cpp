/* -*- coding: utf-8 -*- */

/**
 * test-010-json.cpp
 * 
 * Crée un Reservoir et le serialise.
 */

#include <iostream>       // std::cout

#include <reservoir.hpp>

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

  // TODO Write un a file
  // TODO Read from file
  
  return 0;
}
