/* -*- coding: utf-8 -*- */

/** 
 * Test std::map<std::vector<unsigned int>,int>.
 */

#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <utils.hpp>
using namespace utils;

std::vector<unsigned int> _v0 = {1,2,3};
std::vector<unsigned int> _v1 = {1,2,3};
std::vector<unsigned int> _v2 = {1,2,4};
std::vector<unsigned int> _v3 = {3,2,1};

std::map<std::vector<unsigned int>,int> _map;

// ***************************************************************** print_map
void print_map()
{
  for (auto it = _map.begin(); it != _map.end(); ++it) {
    std::cout << str_vec(it->first) << "-> " << it->second << '\n';
  }
}
  
int main(int argc, char *argv[])
{
  // Test operator<
  std::cout << str_vec(_v0) << "< " << str_vec(_v0) << ": " << (_v0 < _v0) << std::endl;
  std::cout << str_vec(_v0) << "< " << str_vec(_v1) << ": " << (_v0 < _v1) << std::endl;
  std::cout << str_vec(_v1) << "< " << str_vec(_v0) << ": " << (_v1 < _v0) << std::endl;
  std::cout << str_vec(_v0) << "< " << str_vec(_v2) << ": " << (_v0 < _v2) << std::endl;
  std::cout << str_vec(_v1) << "< " << str_vec(_v2) << ": " << (_v1 < _v2) << std::endl;

  // Test map
  std::cout << "__MAP CREATION" << std::endl;
  _map[_v0] = 2;
  print_map();
  std::cout << "__INCREMENT" << std::endl;
  _map[_v0]++;
  _map[_v2]++;
  print_map();
  std::cout << "__AGAIN" << std::endl;
  _map[_v1]++;
  print_map();

  std::cout << "__ADD elements" << std::endl;
  _map[_v2]++;
  _map[_v2]++;
  _map[_v2]++;
  _map[_v2]++;
  _map[_v3]++;
  print_map();

  std::cout << "__SORT" << std::endl;
  std::vector<std::pair<std::vector<unsigned int>, int>> pairs;
  for (auto itr = _map.begin(); itr != _map.end(); ++itr)
    pairs.push_back(*itr);

  std::sort(pairs.begin(), pairs.end(),
	    [](std::pair<std::vector<unsigned int>, int>& a,
	       std::pair<std::vector<unsigned int>, int>& b)
	    {
	      return a.second < b.second;
	    }
	    );
  for (auto it = pairs.begin(); it != pairs.end(); ++it) {
    std::cout << str_vec(it->first) << " -> " << it->second << std::endl;
  }

}

