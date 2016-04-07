/* -*- coding: utf-8 -*- */

/** 
 * test-002-neuron.cpp
 *
 * - create DSOM::Neuron
 */
#include <iostream>       // std::cout
#include <dsom/neuron.hpp>

/**
 * Create DSOM::Neuron
 */
void tt_create()
{
  Model::DSOM::Neuron n(0, 3);
  std::cout << n.dumpToString() << "\n";

  n.add_link( 1 );
  n.add_link( 5 );
  std::cout << n.dumpToString() << "\n";

  n.add_neighbor( 1, 1.0 );
  n.add_neighbor( 2, 2.1 );
  std::cout << n.dumpToString() << "\n";
}

// ***************************************************************************
int main(int argc, char *argv[])
{
  tt_create();
  return 0;
}
