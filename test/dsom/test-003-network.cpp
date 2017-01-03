/* -*- coding: utf-8 -*- */

/** 
 * test-003-network.cpp
 *
 */
#include <iostream>                     // std::cout
#include <fstream>                      // std::ofstream
#include <dsom/network.hpp>

#include "rapidjson/document.h"         // rapidjson's DOM-style API
#include <utils.hpp>                    // various str_xxx
using namespace utils::rj;

// ******************************************************************** Global
#define N_FILE "dsom.data"

void tt_network_step()
{
  // 2D input, 10 neurons, 1D output
  Model::DSOM::Network net(2, 10, 1);
  auto max_dist = net.computeAllDist();
  std::cout << "max dist=" << max_dist << "\n***********\n" << net.str_dump();

  // one input
  Eigen::VectorXd v1(2);
  v1 << 0.5, 0.5;
  std::cout << "v1 = " << v1 << "\n";

  auto out = net.forward( v1 );
  std::cout << "out=\n" << out << "\n";
  std::cout << "Winner is " << net.get_winner() << " at " << net.get_winner_dist() << "\n";

  auto win_dist = net.computeWinner( v1 );
  std::cout << "Winner is " << net.get_winner() << " at " << win_dist << "\n";

  net.forward( v1 );
  net.deltaW( v1, 1.0, 1.0, true);
}

/** Test regularly setting input weigths in 2d space */
void tt_network_regular_weights()
{
  Model::DSOM::Network net(2, 9, 1);
  std::cout << "***********\n" << net.str_dump();
  net.set_regular_weights();
  std::cout << "***********\n" << net.str_dump();
}
/** Create Model::DSOM::Network */
void tt_network_create()
{
  Model::DSOM::Network net(3, 25, 2);
  std::cout << "***********\n" << net.str_dump();

  net.computeDist( 0 );
  std::cout << "***********\n" << net.str_dump();

  net.computeDist( 1 );
  std::cout << "***********\n" << net.str_dump();

  double max_dist = net.computeAllDist();
  std::cout << "***********\n" << net.str_dump();
  std::cout << "\n MAX = " << max_dist << "\n";
}
/** Read/TODO Write Model::DSOM::Network */
void tt_net_wr()
{
  Model::DSOM::Network net(2, 10, 2);
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
  Model::DSOM::Network net_read( ifile );
  std::cout << "***** NETWORK_READ ***" << "\n";
  std::cout << net_read.str_dump() << std::endl;
  std::cout << "***** NETWORK_end ****" << "\n";
  
  // Persistence save("network.sav" );

  // DSOM::Network netr( save );
  // std::cout << "*****\n" <<netr.dumpToString() << "\n";  

}


// ***************************************************************************
int main(int argc, char *argv[])
{
  std::cout << "__CREATE DSOM" << std::endl;
  tt_network_create();
  std::cout << "__READ/WRITE DSOM" << std::endl;
  tt_net_wr();
  std::cout << "__REGULAR WEIGHTS DSOM" << std::endl;
  tt_network_regular_weights();
  std::cout << "__STEP DSOM" << std::endl;
  tt_network_step();
  return 0;
}
