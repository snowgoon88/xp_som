/* -*- coding: utf-8 -*- */

/** 
 * simu-000-rob.cpp
 *
 * Simulateur simple avec le robot qui avance.
 */

#include <iostream>       // std::cout

#include <simul.hpp>

//******************************************************************************
int main( int argc, char *argv[] )
{
  Simul sim;
  std::cout << "__CREATION" << std::endl << sim.str_dump() << std::endl;
  sim._rob.set_speed( 1, 0 );
  
  for( unsigned int i = 0; i< 10; ++i) {
    sim.update( 0.1 );
    std::cout << sim.str_dump() << std::endl;
  }

  return 0;
}
