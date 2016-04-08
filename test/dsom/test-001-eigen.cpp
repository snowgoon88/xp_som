/* -*- coding: utf-8 -*- */

/** 
 * test-001-eigen.cpp
 *
 * Check that Eigen is usable to do some vector computation.
 */
#include <iostream>       // std::cout
#include <Eigen/Dense>

/** Create and manipulate Vectors */
void tt_vector()
{
  Eigen::Vector3f v1(1.0, 0.5, 0.8);
  Eigen::Vector3f v2(1.0, 0.9, 0.2);

  std::cout << "v1-v2 = " << v1-v2 << "\n";
  std::cout << "POW v1-v2 = " << (v1-v2).cwiseProduct(v1-v2) << "\n";
  std::cout << "SUM POW v1-v2 = " << (v1-v2).cwiseProduct(v1-v2).sum() << "\n";
  std::cout << "SQRT SUM POW v1-v2 = " << sqrt((v1-v2).cwiseProduct(v1-v2).sum()) << "\n";
}
/** Create Random Matrices */
void tt_matrix()
{
  for(int i = 0; i < 5; i++) {   
    std::srand((unsigned int) time(0));
    // Random matrix between -1 and 1
    Eigen::VectorXd V = Eigen::VectorXd::Random(3);
    std::cout << "vec=\n" << V << std::endl;
    const Eigen::VectorXd& VP = Eigen::VectorXd::Random(3);
    std::cout << "vec=\n" << VP << std::endl;
    Eigen::MatrixXf A = Eigen::MatrixXf::Random(3, 3);
    std::cout << "mat=\n" << A << std::endl;
    // Scale between 10 and 20
    A = (A.array() - -1.f) / (1.f - -1.f) * (20.f - 10.f) + 10.f;
    std::cout << "[10,20]=\n" << A << std::endl;
  }
}

// ***************************************************************************
int main(int argc, char *argv[])
{
  std::cout << "__VECTOR" << std::endl;
  tt_vector();
  std::cout << "__MATRIX" << std::endl;
  tt_matrix();
  return 0;
}
