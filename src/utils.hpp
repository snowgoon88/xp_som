/* -*- coding: utf-8 -*- */

#ifndef UTILS_HPP
#define UTILS_HPP

/** 
 * Various functions for nice output of GSL matrices and vectors.
 */

#include <gsl/gsl_matrix.h>           // gsl Matrices

#include "rapidjson/document.h"       // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h"   // rapidjson
#include "rapidjson/stringbuffer.h"   // rapidjson

#include <string>                     // std::string

namespace utils
{
  namespace gsl
  {
  // ************************************************************ gsl::str_vec  
  std::string str_vec(const gsl_vector* v)
  {
    std::stringstream str;
    for( unsigned int i = 0; i< v->size; ++i) {
      str << gsl_vector_get(v, i) << "; ";
    }
    return str.str();
  }
  // ************************************************************ gsl::str_mat
  std::string str_mat(const gsl_matrix* m)
  {
    std::stringstream str;
    for( unsigned int i = 0; i < m->size1; ++i) {
      for( unsigned int j = 0; j < m->size2; ++j) {
	str << gsl_matrix_get( m, i, j ) << "; ";
      }
      str << std::endl;
    }
    return str.str();
  }
  }; // namespace gls

  namespace rj // alias rapidjson
  {
  // ************************************************************* rj::str_obj
  std::string str_obj( const rapidjson::Value& obj )
  {
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( buffer );
    obj.Accept( writer );

    return buffer.GetString();
  }
  }; // namespace
  
}; // namespace utils

#endif // UTILS_HPP
