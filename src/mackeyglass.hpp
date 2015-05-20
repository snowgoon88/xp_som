/* -*- coding: utf-8 -*- */

# pragma once

/** 
 * Generate a MackeyGlass discrete serie. Either from a set of initial values or by
 * specifying the delay length (mem_size).
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

#include <ctime>                        // std::time
#include <vector>                       // std::vector
#include <gsl/gsl_rng.h>                // gsl random generator
#include <gsl/gsl_randist.h>            // gsl random distribution

#include <math.h>                       // pow, sqrt

namespace MackeyGlass {
  /** Type of data */
  typedef std::vector<double>     Data;               // A sequence of values

  /** Generate a sequence with an initial random vector of size mem_size */
  Data create_sequence( unsigned int nb_pt, double level,
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
}

