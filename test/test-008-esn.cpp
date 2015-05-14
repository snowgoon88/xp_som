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
  Reservoir res( 2, 3 );
  std::cout << res.str_dump() << std::endl;
  
  return 0;
}
