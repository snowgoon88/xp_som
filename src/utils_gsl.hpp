/* -*- coding: utf-8 -*- */

#ifndef UTILS_GSL_HPP
#define UTILS_GSL_HPP

/** 
 * Various functions for nice output of GSL matrices and vectors.
 */

namespace utils
{
  // ***************************************************************** str_vec
  std::string str_vec(const gsl_vector* v)
  {
    std::stringstream str;
    for( unsigned int i = 0; i< v->size; ++i) {
      str << gsl_vector_get(v, i) << "; ";
    }
    return str.str();
  }
  // ***************************************************************** str_mat
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
};



#endif // UTILS_GSL_HPP
