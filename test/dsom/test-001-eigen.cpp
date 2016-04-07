/* -*- coding: utf-8 -*- */

/** 
 * test-001-eigen.cpp
 *
 * Check that Eigen is usable to do some vector computation.
 */
#include <iostream>       // std::cout
#include <Eigen/Dense>

void tt_vector()
{
  Eigen::Vector3f v1(1.0, 0.5, 0.8);
  Eigen::Vector3f v2(1.0, 0.9, 0.2);

  std::cout << "v1-v2 = " << v1-v2 << "\n";
  std::cout << "POW v1-v2 = " << (v1-v2).cwiseProduct(v1-v2) << "\n";
  std::cout << "SUM POW v1-v2 = " << (v1-v2).cwiseProduct(v1-v2).sum() << "\n";
  std::cout << "SQRT SUM POW v1-v2 = " << sqrt((v1-v2).cwiseProduct(v1-v2).sum()) << "\n";
}

// ***************************************************************************
int main(int argc, char *argv[])
{
  tt_vector();
  return 0;
}
