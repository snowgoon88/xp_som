/* -*- coding: utf-8 -*- */

/**
 * test-008-esn.cpp
 * 
 * Matrice aléatoire
 */

#include <iostream>       // std::cout

#include <reservoir.hpp>

//******************************************************************************
int main( int argc, char *argv[] )
{
  Reservoir res( 2, 3, 0.2);
  std::cout << res.str_dump() << std::endl;
  res.set_spectral_radius( 0.1 );
  std::cout << "** APRES CALCUL **" << "\n";
  std::cout << res.str_dump() << std::endl;
  res.set_spectral_radius( 0.1 );
  std::cout << "** APRES 2eme CALCUL **" << "\n";
  std::cout << res.str_dump() << std::endl;

  // try Input
  Reservoir::Tinput in = {1.2, -2.0};
  res.forward( in );
  res.forward( in );
  
  
  return 0;
}
