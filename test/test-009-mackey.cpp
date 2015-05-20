/* -*- coding: utf-8 -*- */

/** 
 * test-009-mackey.cpp
 *
 * Generate and print Mackeyglass sequence.
 */

#include <iostream>       // std::cout

#include <mackeyglass.hpp>

//******************************************************************************
int main( int argc, char *argv[] )
{
  MackeyGlass::Data data = MackeyGlass::create_sequence( 20, 1.0, 0.1, 0.5, 2.0, 5 );

  std::cout << "SEQ={ ";
  for( auto& x: data) {
    std::cout << x << "; ";
  }
  std::cout << "}" << std::endl;
  
  return 0;
}

