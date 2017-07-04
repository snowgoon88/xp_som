/* -*- coding: utf-8 -*- */

/** 
 * Test searching in an array/vector using a comparison function.
 */

#include <iostream>
#include <vector>
#include <algorithm>    // std::nth_element
#include <limits>

#include <utils.hpp>

// **************************************************************** comparison
int mydist( int i, int j)
{
  return (i-j)*(i-j);
}
// ********************************************************************** main
int main(int argc, char *argv[])
{
  unsigned int vec_size = 10000000;
  // New random seed
  unsigned long int seed = utils::random::rnd_int<unsigned long int>();
  std::default_random_engine generator(seed);
  //std::uniform_int_distribution<int> gen(1,10);
  std::uniform_int_distribution<int> gen;
  
  // Init vector
  std::vector<int> v_in;
  for( unsigned int i = 0; i < vec_size; ++i) {
    v_in.push_back( gen(generator) );
  }

  std::cout << "__INIT" << std::endl;
  //std::cout << utils::str_vec( v_in ) << std::endl;

  // Select target
  int target = gen(generator);
  
  // Loop
  auto start_time = std::chrono::steady_clock::now();
  int minval = std::numeric_limits<int>::max();
  int bestval = -1;
  for( unsigned int i = 0; i < v_in.size(); ++i) {
    if( mydist( target, v_in[i]) < minval ) {
	  minval = mydist( target, v_in[i]);
	  bestval = v_in[i];
	}
  }
  auto end_time = std::chrono::steady_clock::now();
  std::chrono::duration<double> duration = end_time - start_time; 
  std::cout << "__BASE LOOP in "  << duration.count() << std::endl;
  std::cout << "  target=" << target << " best="<< bestval << std::endl;

  // iterator
  start_time = std::chrono::steady_clock::now();
  minval = std::numeric_limits<int>::max();
  bestval = -1;
  for (auto it = v_in.begin(); it != v_in.end(); ++it) {
    if( mydist( target, *it) < minval ) {
	  minval = mydist( target, *it);
	  bestval = *it;
	}
  }
  end_time = std::chrono::steady_clock::now();
  duration = end_time - start_time; 
  std::cout << "__ITERATOR LOOP in "  << duration.count() << std::endl;
  std::cout << "  target=" << target << " best="<< bestval << std::endl;

  
  // advanced loop
  start_time = std::chrono::steady_clock::now();
  minval = std::numeric_limits<int>::max();
  bestval = -1;
  for( auto& v: v_in) {
	if( mydist( target, v) < minval ) {
	  minval = mydist( target, v);
	  bestval = v;
	}
  }
  end_time = std::chrono::steady_clock::now();
  duration = end_time - start_time; 
  std::cout << "__ADVANCED LOOP in "  << duration.count() << std::endl;
  std::cout << "  target=" << target << " best="<< bestval << std::endl;

  // nth_element
  start_time = std::chrono::steady_clock::now();
  std::nth_element (v_in.begin(), v_in.begin()+0, v_in.end(),
					// lambda expression
					[=](int a, int b) {
					  return mydist(target, a) < mydist(target,b);
					});
  end_time = std::chrono::steady_clock::now();
  duration = end_time - start_time; 
  std::cout << "__Nth ELEMENT in "  << duration.count() << std::endl;
  std::cout << "  target=" << target << " best=" << v_in[0] << std::endl;
  

  
  return 0;
}
