/* -*- coding: utf-8 -*- */

/** 
 * Tester différent générateur aléatoires.
 */

#include <iostream>                  // std::cout
#include <ctime>                     // std::time std::clock
#include <chrono>                    // std::chrono::steady_clock
#include "utils.hpp"
#include <random>

#include <dsom/neuron.hpp>

// Eigen library for "dense" matrices and vectors
#include <Eigen/Dense>

int main(int argc, char *argv[])
{
  std::cout << "** Random vector  Eigen::VectorXd::Random()" << std::endl;
  auto vec1 = Eigen::VectorXd::Random(2);
  std::cout << vec1 << std::endl;

  
  std::cout << "** DIFFERENT SEED GENERATION" << std::endl;
  auto seed_time1 = (unsigned int) time(0);
  auto seed_time2 = (unsigned int) time(0);
  
  auto seed_clock1 = std::clock();
  auto seed_clock2 = std::clock();
  
  auto ltime1 = std::chrono::steady_clock::now();
  auto ltime2 = std::chrono::steady_clock::now();

  auto high1 = std::chrono::high_resolution_clock::now();
  auto high2 = std::chrono::high_resolution_clock::now();
  
  auto utils1 = utils::random::rnd_int<unsigned int>();
  auto utils2 = utils::random::rnd_int<unsigned int>();

  std::random_device dev;
  auto dev1 = dev();
  auto dev2 = dev();

  
  
  std::cout << "time1 = " << seed_time1 << "\t size = "<< sizeof(seed_time1) << std::endl;
  std::cout << "time2 = " << seed_time2 << "\t size = "<< sizeof(seed_time2) << std::endl;
  
  std::cout << "clock1 =" << seed_clock1 << "\t size = " << sizeof(seed_clock1)<< std::endl;
  std::cout << "clock2 =" << seed_clock2 << "\t size = " << sizeof(seed_clock2)<< std::endl;
  
  unsigned int ltime_us1 = std::chrono::duration_cast<std::chrono::microseconds>(ltime1.time_since_epoch()).count();
  std::cout << "chrono1 = " << ltime_us1 << "\t size = " <<  sizeof(ltime_us1) << std::endl;
  unsigned int ltime_us2 = std::chrono::duration_cast<std::chrono::microseconds>(ltime2.time_since_epoch()).count();
  std::cout << "chrono2 = " << ltime_us2 << "\t size = " <<  sizeof(ltime_us2) << std::endl;

  unsigned int high_us1 = std::chrono::duration_cast<std::chrono::microseconds>(high1.time_since_epoch()).count();
  std::cout << "high1 = " << high_us1 << "\t size = " <<  sizeof(high_us1) << std::endl;
  unsigned int high_us2 = std::chrono::duration_cast<std::chrono::microseconds>(high2.time_since_epoch()).count();
  std::cout << "high2 = " << high_us2 << "\t size = " <<  sizeof(high_us2) << std::endl;

  
  std::cout << "utils1 = " << utils1 << "\t size = " << sizeof(utils1) << std::endl;
  std::cout << "utils2 = " << utils2 << "\t size = " << sizeof(utils2) << std::endl;

  std::cout << "dev1 = " << dev1 << "\t size = "<< sizeof(dev1) << std::endl;
  std::cout << "dev2 = " << dev2 << "\t size = "<< sizeof(dev2) << std::endl;

  
  std::cout << "** Random vector  Eigen::VectorXd::Random()" << std::endl;
  auto vec2 = Eigen::VectorXd::Random(2);
  std::cout << vec2 << std::endl;

// * Random vector  Eigen::VectorXd::Random()
//  0.59688
// 0.823295
// -0.604897
// -0.329554


  
  // auto neur = Model::DSOM::Neuron(0, 3);
  // std::cout << neur.str_dump() << std::endl;
  
  // std::cout << "** Random vector  Eigen::VectorXd::Random()" << std::endl;
  // auto vec1 = Eigen::VectorXd::Random(2);
  // std::cout << vec1 << std::endl;
  // std::srand(seed);
  // auto vec2 = Eigen::VectorXd::Random(2);
  // std::cout << vec2 << std::endl;

  // auto neur2 = Model::DSOM::Neuron(0, 3);
  // std::cout << neur2.str_dump() << std::endl;

  
  // // srand est quand même utilisé souvent, notamment dans Eigen
  // std::cout << "sizeof(uint)=" << sizeof(unsigned int) << std::endl;
  // auto seed = std::clock();
  // std::cout << "seed=" << seed << std::endl;
  // std::cout << "sizeof seed=" << sizeof seed << std::endl;

  // 
  // unsigned int ltime_us = std::chrono::duration_cast<std::chrono::microseconds>(ltime.time_since_epoch()).count();
  // std::cout << "ltime_us=" << ltime_us << std::endl;
  // std::cout << "sizeof ltime_us=" << sizeof ltime_us << std::endl;
  return 0;
}

