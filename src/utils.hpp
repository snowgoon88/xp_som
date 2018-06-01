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
#include <sstream>                    // std::stringdtream
#include <vector>                     // std::vector

#include <chrono>                     // std::chrono::steady_clock
#include <random>                     // std::uniform_int...

#include <memory>                     // std::unique_ptr, std::shared_ptr
#include <utility>                    // std::forward
// ***************************************************************************
// **************************************************************** unique_ptr
// ***************************************************************************
/** A Template function to replace make_unique (not in C++11)
 * auto ptr(make_unique<Truc>());
 */
template<typename T, typename... Ts>
std::unique_ptr<T> make_unique(Ts&&... params)
{
  return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}

// ***************************************************************************
// ********************************************************************* Range
// ***************************************************************************
// for (int num : make_range(vector, 3, 7))
//     std::cout << num << ", ";      // 4, 5, 6, 7,
// http://stackoverflow.com/questions/30540101/iterator-for-a-subset-of-a-vector
namespace utils
{
template <class Iter>
class Range {
  Iter b;
  Iter e;
public:
  
    Range(Iter b, Iter e) : b(b), e(e) {}

    Iter begin() { return b; }
    Iter end() { return e; }
};

template <class Container>
Range<typename Container::iterator> 
make_range(Container& c, size_t b, size_t e) {
    return Range<typename Container::iterator> (c.begin()+b, c.begin()+e);
}
};
// ***************************************************************************
// ******************************************************************** RANDOM
// ***************************************************************************
namespace utils
{
  namespace random
  {
    /**
     * Generate a random TInt with a first seed taken from std::steady_clock
     * and then using std::random.
     */
    template<class TInt>
    TInt rnd_int()
    {
      // first seed
      // auto ltime = std::chrono::steady_clock::now();
      // unsigned int first_seed = std::chrono::duration_cast<std::chrono::microseconds>(ltime.time_since_epoch()).count();
      std::random_device dev;
      auto first_seed = dev();
      std::default_random_engine generator(first_seed);
      
      std::uniform_int_distribution<TInt> gen;
      TInt seed = gen(generator);

      return seed;
    };
  };
};

// ***************************************************************************
// *********************************************************************** STR
// ***************************************************************************
namespace utils
{
  template<class T>
  std::string str_vec( const std::vector<T>& vec )
  {
    std::stringstream str;
    for( auto& item: vec) {
      str << item << " ";
    }
    return str.str();
  }
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
