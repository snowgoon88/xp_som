/* -*- coding: utf-8 -*- */

#ifndef DSOM_UTILS_HPP
#define DSOM_UTILS_HPP

/** 
 * Various functions for nice output of Eigen matrices and vectors.
 */
// Eigen library for "dense" matrices and vectors
#include <Eigen/Dense>

#include <sstream>
#include <string>

namespace utils
{
namespace eigen
{
  template<typename T>
  std::string str_vec( const T& vector )
  {
     std::stringstream ss;

	 for( int i=0; i < vector.size(); i++) {
	   ss << (vector)(i) << " ";
	 }
	 ss << ";";
	 
	  return ss.str();
  }
}; // namespace eigen
}; // namespace utils

#endif // DSOM_UTILS_HPP
