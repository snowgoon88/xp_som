/* -*- coding: utf-8 -*- */

/** 
 * Basics to use REC_DSOM.
 *   o create 1D regular REC_DSOM
 *   o save/relaod REC_DSOM
 *   o forward
 *   o backward
 */
#include <iostream>                     // std::cout
#include <fstream>                      // std::ofstream
#include <dsom/r_network.hpp>

#include "rapidjson/document.h"         // rapidjson's DOM-style API
#include <utils.hpp>                    // various str_xxx
using namespace utils::rj;

// ******************************************************************** Global
#define N_FILE "rec_dsom2D.json"

/** Create Model::DSOM::Network */
void tt_network_create()
{
  Model::DSOM::RNetwork net(2, 9, -1); //regular 1D grid
  std::cout << "***********\n" << net.str_dump();

  auto max_dist = net.get_max_dist_neurone();
  std::cout << "***********\n" << net.str_dump();
  std::cout << "\n MAX = " << max_dist << "\n";
}
/** Read/TODO Write Model::DSOM::Network */
void tt_net_wr()
{
  Model::DSOM::RNetwork net(2, 16, -1);
  std::cout << "***********\n" << net.str_dump();

  //*** JSON ***
  rapidjson::Document doc;
  rapidjson::Value obj = net.serialize( doc );
  std::cout << str_obj( obj ) << std::endl;
  // Write un a file
  std::ofstream ofile(N_FILE);
  ofile << str_obj( obj ) << std::endl;
  ofile.close();

  // Read from JSON
  std::ifstream ifile(N_FILE);
  Model::DSOM::RNetwork net_read( ifile );
  std::cout << "***** REC_NETWORK_READ ***" << "\n";
  std::cout << net_read.str_dump() << std::endl;
  std::cout << "***** REC_NETWORK_end ****" << "\n";
  
}

/** Step forward and backward */
void tt_network_step()
{
  // 1D input, 7 neurons, on a 1D grid */
  Model::DSOM::Network net(1, 7, -1);

  auto max_dist = net.computeAllDist();
  std::cout << "max dist=" << max_dist << "\n***********\n" << net.str_dump();

  // one input
  Eigen::VectorXd v1(1);
  v1 << 0.57;
  std::cout << "v1 = " << v1 << "\n";

  auto win_dist = net.computeWinner( v1 );
  std::cout << "Winner is " << net.get_winner() << " at " << win_dist << "\n";

  // net.forward( v1 );
  // net.deltaW( v1, 1.0, 1.0, true);
  // std::cout << "__AFTER" << std::endl << net.str_dump();
}
// ***************************************************************************
int main(int argc, char *argv[])
{
  std::cout << "__CREATE 1D REC_SOM" << std::endl;
  tt_network_create();
  std::cout << "__READ:WRITE REC_SOM" << std::endl;
  tt_net_wr();
  
  return 0;
}
