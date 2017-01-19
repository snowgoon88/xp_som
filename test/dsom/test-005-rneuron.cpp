/* -*- coding: utf-8 -*- */

/** 
 * test-005-rneuron.cpp
 *
 * - create DSOM::RNeuron
 */
#include <iostream>                     // std::cout
#include <fstream>                      // std::ofstream
#include <dsom/r_neuron.hpp>

#include "rapidjson/document.h"         // rapidjson's DOM-style API
#include <json_wrapper.hpp>          // JSON::OStreamWrapper, JSON::IStreamWrapper
#include <utils.hpp>                    // various str_xxx
using namespace utils::rj;

// ******************************************************************** Global
#define N_FILE "r_neuron.data"

/**
 * Create DSOM::RNeuron with no pos
 */
void tt_create()
{
  Model::DSOM::RNeuron n(0, 3);
  std::cout << n.str_display() << std::endl;
  std::cout << n.str_dump() << "\n";

  n.add_link( 1 );
  n.add_link( 5 );
  std::cout << n.str_dump() << "\n";

  n.add_neighbor( 1, 1.0 );
  n.add_neighbor( 2, 2.1 );
  std::cout << n.str_dump() << "\n";
}
/**
 * Copy and Assignment
 */
void tt_copy_assign()
{
  Model::DSOM::RNeuron n1(0, 3);
  n1.add_link( 1 );
  n1.add_neighbor( 1, 1.0 );
  std::cout << "N1 = " << n1.str_dump() << std::endl;

  Model::DSOM::RNeuron n2(1, 2);
  n2.add_link( 5 );
  n2.add_neighbor( 2, 2.1 );
  std::cout << "N2 = " << n2.str_dump() << std::endl;

  Model::DSOM::RNeuron n3( n2 );
  std::cout << "N3 = " << n3.str_dump() << std::endl;
  
  n2 = n1;
  std::cout << "N2=N1" << n2.str_dump() << std::endl;
  std::cout << "N3 = " << n3.str_dump() << std::endl;
  
  
}
/**
 * Read/Write DSOM::RNeuron
 */
void tt_neur_wr()
{
  typedef Eigen::Vector2i Vec;
  Model::DSOM::RNeuron n(0, Vec(2,3), 3);
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

  // Read from JSON
  std::ifstream ifile(N_FILE);
  Model::DSOM::RNeuron n_read( ifile );
  std::cout << "***** N_READ ***" << "\n";
  std::cout << n_read.str_dump() << std::endl;
  std::cout << "***** N_end ****" << "\n";

}


// ***************************************************************************
int main(int argc, char *argv[])
{
  std::cout << "__CREATE RNeurone" << std::endl;
  tt_create();
  std::cout << "__COPY/ASSIGN RNeurone" << std::endl;
  tt_copy_assign();
  
  std::cout << "__READ/WRITE neurone" << std::endl;
  tt_neur_wr();
  return 0;
}
