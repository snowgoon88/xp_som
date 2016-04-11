/* -*- coding: utf-8 -*- */

/** 
 * Tester différent générateur aléatoires.
 */

#include <iostream>                  // std::cout
#include <ctime>                     // std::time std::clock
#include <chrono>                    // std::chrono::steady_clock

int main(int argc, char *argv[])
{
  // srand est quand même utilisé souvent, notamment dans Eigen
  std::cout << "sizeof(uint)=" << sizeof(unsigned int) << std::endl;
  auto seed = std::clock();
  std::cout << "seed=" << seed << std::endl;
  std::cout << "sizeof seed=" << sizeof seed << std::endl;

  auto ltime = std::chrono::steady_clock::now();
  unsigned int ltime_us = std::chrono::duration_cast<std::chrono::microseconds>(ltime.time_since_epoch()).count();
  std::cout << "ltime_us=" << ltime_us << std::endl;
  std::cout << "sizeof ltime_us=" << sizeof ltime_us << std::endl;
  return 0;
}

