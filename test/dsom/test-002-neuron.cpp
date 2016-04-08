/* -*- coding: utf-8 -*- */

/** 
 * test-002-neuron.cpp
 *
 * - create DSOM::Neuron
 */
#include <iostream>                     // std::cout
#include <fstream>                      // std::ofstream
#include <dsom/neuron.hpp>

#include "rapidjson/document.h"         // rapidjson's DOM-style API
#include <utils.hpp>                    // various str_xxx
using namespace utils::rj;

// ******************************************************************** Global
#define N_FILE "neuron.data"

/**
 * Create DSOM::Neuron
 */
void tt_create()
{
  Model::DSOM::Neuron n(0, 3);
  std::cout << n.str_dump() << "\n";

  n.add_link( 1 );
  n.add_link( 5 );
  std::cout << n.str_dump() << "\n";

  n.add_neighbor( 1, 1.0 );
  n.add_neighbor( 2, 2.1 );
  std::cout << n.str_dump() << "\n";
}
/**
 * Read/Write DSOM::Neuron
 */
void tt_neur_wr()
{
  Model::DSOM::Neuron n(0, 3);
  n.add_link( 1 );
  n.add_link( 5 );
  std::cout << "*****\n" <<n.str_dump() << "\n";  

  //*** JSON ***
  rapidjson::Document doc;
  rapidjson::Value obj = n.serialize( doc );
  std::cout << str_obj( obj ) << std::endl;
  // Write un a file
  std::ofstream ofile(N_FILE);
  ofile << str_obj( obj ) << std::endl;
  ofile.close();
  
  //*** PERSISTENCE ***
  // std::ofstream outfile( "neurone.sav" );
  // n.write( outfile );
  // outfile.close();

  // Persistence save("neurone.sav" );

  // DSOM::Neuron nr( save );
  // std::cout << "*****\n" <<nr.dumpToString() << "\n";
  
}


// ***************************************************************************
int main(int argc, char *argv[])
{
  std::cout << "__CREATE Neurone" << std::endl;
  tt_create();
  std::cout << "__READ/WRITE neurone" << std::endl;
  tt_neur_wr();
  return 0;
}
