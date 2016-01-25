/* -*- coding: utf-8 -*- */

#ifndef NOISE_HPP
#define NOISE_HPP

/** 
 * Generate a multi-dimension noise.
 * Can save into json.
 * can Read/Write into file.
 */

#include <iostream>                   // std::cout, std::istream
#include <sstream>                   // std::istringstream
#include <ctime>                      // std::time
#include <vector>                     // std::vector
#include <gsl/gsl_rng.h>              // gsl random generator
#include <gsl/gsl_randist.h>          // gsl random distribution

#include "rapidjson/document.h"       // rapidjson's DOM-style API
namespace rj = rapidjson;

class WNoise
{
public:
  // ************************************************************ WNoise::Data
  typedef std::vector<std::vector<double>> Data;
  // ******************************************************** WNoise::creation
  WNoise( const unsigned int nb_pt,
	  const double level = 1.0,
	  const unsigned int dim = 1,
	  unsigned long int seed = std::time(NULL) ) :
    _nb_pt(nb_pt), _level(level), _dim(dim),
    _seed(seed)
  {
  };
  // ********************************************************** WNoise::create
  static Data create_sequence( const unsigned int nb_pt,
			       const double level = 1.0,
			       const unsigned int dim = 1,
			       const unsigned long int seed = std::time(NULL) )
  {
    // Random Engine with seed 
    const gsl_rng* rnd = gsl_rng_alloc( gsl_rng_taus );
    gsl_rng_set( rnd, seed );

    Data v_noise;
    for( unsigned int i = 0; i < nb_pt; ++i) {
      std::vector<double> element;
      for( unsigned int j = 0; j < dim; ++j) {
	element.push_back( (gsl_rng_uniform_pos(rnd)-0.5) * level );
      }
      v_noise.push_back( element );
    }
    return v_noise;
  }
  const Data& create_sequence()
  {
    _seq = create_sequence( _nb_pt, _level, _dim, _seed );
    return _seq;
  }
  // ******************************************************* WNoise::serialize
  rj::Value serialize( rj::Document& doc )
  {
    // rj::Object qui contient les données
    rj::Value obj;
    obj.SetObject();

    // Ajoute les paramètres
    obj.AddMember( "nb_pt", rj::Value().SetUint(_nb_pt),
		   doc.GetAllocator() );
    obj.AddMember( "level", rj::Value().SetDouble(_level),
		   doc.GetAllocator() );
    obj.AddMember( "dim", rj::Value().SetUint(_dim),
		   doc.GetAllocator() );
    obj.AddMember( "seed", rj::Value().SetUint(_seed),
		   doc.GetAllocator() );

    return obj;
  }
  // ********************************************************** WNoise::parser
  /** read/write for Mackeyglass */
  static void read(std::istream& is, Data& data)
  {
    double x;
    data.clear();
    std::string line;
    while (std::getline(is, line)) {
      std::vector<double> vec;
      std::istringstream iss(line);
      while( !iss.eof()) {
	iss >> x;
	vec.push_back( x );
      }
      data.push_back( vec );
    }
  }
  static void write(std::ostream& os, const Data& data)
  {
    for( auto& vec: data) {
      for( auto& val: vec) {
	os << val << "\t";
      }
      os << std::endl;
    }
  }
  // ******************************************************* WNoise::attributs
  const Data& data() const { return _seq; };
private:
  /** nb_pt generated */
  unsigned int _nb_pt;
  /** noise level */
  double _level;
  /** dimension */
  unsigned int _dim;
  /** seed */
  unsigned long int _seed;
  /** Sequence generated */
  Data _seq;
};

#endif // NOISE_HPP
