/* -*- coding: utf-8 -*- */

/** 
 * test-002-neuron.cpp
 *
 * - create DSOM::Neuron
 * - Read/Write DSOM::Neuron
 */
#include <iostream>                     // std::cout
#include <fstream>                      // std::ofstream
#include <dsom/neuron.hpp>

#include "rapidjson/document.h"         // rapidjson's DOM-style API
#include <json_wrapper.hpp>          // JSON::OStreamWrapper, JSON::IStreamWrapper
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
  typedef Eigen::Vector2i Vec;
  Model::DSOM::Neuron n(0, Vec(2,3), 3);
  n.add_link( 1 );
  n.add_link( 5 );
  n.add_neighbor( 1, 1.0 );
  n.add_neighbor( 2, 2.1 );
  std::cout << "*****\n" <<n.str_dump() << "\n";  

  //*** JSON ***
  rapidjson::Document doc;
  rapidjson::Value obj = n.serialize( doc );
  std::cout << str_obj( obj ) << std::endl;
  // Write in a file
  std::ofstream ofile(N_FILE);
  ofile << str_obj( obj ) << std::endl;
  ofile.close();

  // Read from file
  std::cout << "UNSERIALZE" << std::endl;
  std::ifstream ifile( N_FILE );
  // Wrapper pour lire document
  JSON::IStreamWrapper instream(ifile);
  // Parse into a document
  rj::Document read_doc;
  read_doc.ParseStream( instream );
  ifile.close();

  Model::DSOM::Neuron read_n( read_doc );
  std::cout << "READ NEUR *******" << std::endl << read_n.str_dump() << std::endl;
  
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
