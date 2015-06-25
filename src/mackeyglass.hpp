/* -*- coding: utf-8 -*- */

# pragma once

/** 
 * Generate a MackeyGlass discrete serie. 
 * Either from a set of initial values or by
 * specifying the delay length (mem_size).
 * Only the sequence is returned, not the values uses for boostrapping.
 *
 * x(i+1)=x(i)+ax(i-s)/(1+x(i-s)^c)-bx(i)
 *
 * @param nb_pt : size of the generated sequence
 * @param level : level is noise of standard deviation divided by standard deviation
 *                of the noise free serie. Gaussian noise with zero mean
 * @param x_ini : vector of initial values
 * @param mem_size : 's' in the equation
 * @param seed : for random generator
 */

#include <ctime>                      // std::time
#include <vector>                     // std::vector
#include <gsl/gsl_rng.h>              // gsl random generator
#include <gsl/gsl_randist.h>          // gsl random distribution

#include "rapidjson/document.h"       // rapidjson's DOM-style API
namespace rj = rapidjson;

#include <math.h>                     // pow, sqrt
// ***************************************************************************
// *************************************************************** MackeyGlass
// ***************************************************************************
class MackeyGlass {
public:
  // ******************************************************************** Data
  /** Type of data */
  typedef std::vector<double>     Data;               // A sequence of values

  // **************************************************************** creation
  MackeyGlass( unsigned int nb_pt, double level,
	       double a, double b, double c,
	       unsigned int mem_size,
	       unsigned long int seed = std::time(NULL) ) :
    _nb_pt(nb_pt), _level(level),
    _a(a), _b(b), _c(c),
    _mem_size(mem_size), _seed(seed)
  {
  };
  // ********************************************************* create_sequence
  /** 
   * Generate a sequence with an initial random vector of size mem_size
   * Only the sequence is returned, not the initialization value.
   *
   * @param out_param : a stream for json formatted parameters serialization
   */
  static Data create_sequence( unsigned int nb_pt, double level,
			double a, double b, double c,
			unsigned int mem_size,
			unsigned long int seed = std::time(NULL) )
  {
    // Random Engine with seed 
    const gsl_rng* rnd = gsl_rng_alloc( gsl_rng_taus );
    gsl_rng_set( rnd, seed );
    
    // Generate initial values x0 in [-1, 1]
    Data x0;
    for( unsigned int i = 0; i< mem_size; ++i) {
      x0.push_back( (gsl_rng_uniform_pos(rnd)-0.5) * 2.0 );
    }

    // Generate sequence and compute mean
    Data seq;
    double x = x0[mem_size-1] + a * x0[0] / (1.0 + pow( x0[0], c)) - b * x0[mem_size-1];
    double mean = x;
    seq.push_back( x );
    for( unsigned int i = 1; i< mem_size; ++i) {
      x = seq[i-1] + a*x0[i] / (1.0 + pow( x0[i], c)) - b * seq[i-1];
      mean += x;
      seq.push_back( x );
    }
    for( unsigned int i = mem_size; i< nb_pt; ++i) {
      x = seq[i-1] + a*seq[i-mem_size] / (1.0 + pow( seq[i-mem_size], c)) - b * seq[i-1];
      mean += x;
      seq.push_back( x ); 
    }
    mean /= (double) nb_pt;
    
    // Compute standard deviation of sequence
    double dev = 0;
    for( auto& x: seq ) {
      dev += (x-mean)*(x-mean);
    }
    dev = sqrt( dev / (double) (nb_pt-1) );

    // Add normal white noise
    for( unsigned int i = 0; i < seq.size(); ++i) {
      seq[i] += gsl_ran_gaussian (rnd, level * dev);
    }

    return seq;
  }
  Data create_sequence()
  {
    _seq  = create_sequence( _nb_pt, _level, _a, _b, _c,
			    _mem_size, _seed );
    return _seq;
  }
  // *************************************************************** serialize
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
    // sous forme d'array
    rj::Value ar;
    ar.SetArray();
    ar.PushBack(_a, doc.GetAllocator());
    ar.PushBack(_b, doc.GetAllocator());
    ar.PushBack(_c, doc.GetAllocator()); 
    obj.AddMember( "abc", ar, doc.GetAllocator() );
    // la suite
    obj.AddMember( "mem_size", rj::Value().SetUint(_mem_size),
		   doc.GetAllocator() );
    obj.AddMember( "seed", rj::Value().SetUint(_seed),
		   doc.GetAllocator() );
    
    return obj;
  };
  // ****************************************************************** parser
  /** read/write for Mackeyglass */
  static void read(std::istream& is, Data& data)
  {
    double x;
    while( !is.eof() ) {
      is >> x;
      data.push_back( x );
    }
  }
  static void write(std::ostream& os, Data& data)
  {
    for( auto& x: data) {
      os << x << std::endl;
    }
  }
  // *************************************************************** attributs
  const Data& data() const { return _seq; };
private:
  /** nb point generated */
  unsigned int _nb_pt;
  /** noise level */
  double _level;
  /** equ param */
  double _a, _b, _c;
  /** memory size needed */
  unsigned int _mem_size;
  /** seed */
  unsigned long int _seed;
  /** Sequence generated */
  Data _seq;
  // ********************************************************************* end
};

