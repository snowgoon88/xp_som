/* -*- coding: utf-8 -*- */

/** 
 * Test Distribution.
 */

#include <distribution.hpp>
#include <iostream>
#include <fstream>

// ************************************************************** test_uniform
void test_uniform()
{
  std::string filename = "unif.data";
  
  Distribution dist;
  // generate
  auto data = dist.uniform( 30, 0, 1.0, -1, 0);
  std::cout << "__CREATE" << std::endl;
  dist.write( std::cout );

  std::cout << "__WRITE and READ" << std::endl;
  std::ofstream ofile( filename );
  dist.write( ofile );
  ofile.close();

  Distribution dist_read;
  std::ifstream ifile( filename );
  dist_read.read( ifile );
  ifile.close();
  dist_read.write( std::cout );
}
// ***************************************************************************
// ********************************************************************** main
int main(int argc, char *argv[])
{
  test_uniform();
  return 0;
}

