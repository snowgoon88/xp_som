/* -*- coding: utf-8 -*- */

#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP

/** 
 * Genereate various 2D-sample according to distributions:
 * - uniform bouned
 * - normal bounded
 * - ring
 */

#include <Eigen/Dense>
#include <vector>
#include <random>                     // std::uniform_int...
#include <iostream>                   // std::cout, std::istream

// ***************************************************************************
// ************************************************************** Distribution
// ***************************************************************************
class Distribution
{
public:
  // ***************************************************** Distribution::types
  using Sample = Eigen::Vector2d;
  using Data = std::vector<Sample>;

  // *************************************************** Distribution::uniform
  const Data& uniform( unsigned int nb_data=100,
		       double min_x=-1.0, double max_x=1.0,
		       double min_y = -1.0, double max_y = 1.0 )
  {
    // generate seed
    std::random_device random_seeder;
    std::default_random_engine rnd_engine( random_seeder() );
    // and random distribution
    std::uniform_real_distribution<double> rnd_x( min_x, max_x );
    std::uniform_real_distribution<double> rnd_y( min_y, max_y );

    // Generate samples
    _data.clear();
    for( unsigned int i = 0; i < nb_data; ++i) {
      Sample sample { rnd_x(rnd_engine), rnd_y(rnd_engine) };
      _data.push_back( sample );
    }

    return _data;
  }
  // **************************************************** Distribution::normal
  const Data& normal( unsigned int nb_data=100,
					  double center_x = 0.0, double center_y = 0.0,
					  double sigma_x = 1.0, double sigma_y = 1.0,
					  double min_x = -1.0, double max_x = 1.0,
					  double min_y = -1.0, double max_y = 1.0 )
  {
	 // generate seed
    std::random_device random_seeder;
    std::default_random_engine rnd_engine( random_seeder() );
    // and random distribution
    std::normal_distribution<double> rnd_x( center_x, sigma_x );
    std::normal_distribution<double> rnd_y( center_y, sigma_y );

	// Generate samples
	_data.clear();
    for( unsigned int i = 0; i < nb_data; ++i) {
      Sample sample { rnd_x(rnd_engine), rnd_y(rnd_engine) };
	  if( sample[0] < min_x ) sample[0] = min_x;
	  if( sample[0] > max_x ) sample[0] = max_x;
	  if( sample[1] < min_y ) sample[1] = min_y;
	  if( sample[1] > max_y ) sample[1] = max_y;
      _data.push_back( sample );
    }

    return _data;
  }
  // ******************************************************** Disibution::ring
  /** ring from [0,1] uniforms distributions */
  const Data& ring( unsigned int nb_data=100,
					double center_x = 0.0, double center_y = 0.0,
					double r_min = 0.0, double r_max = 0.5 )
  {
		 // generate seed
    std::random_device random_seeder;
    std::default_random_engine rnd_engine( random_seeder() );
    // and random distribution
    std::uniform_real_distribution<double> rnd_x( 0.0, 1.0 );
    std::uniform_real_distribution<double> rnd_y( 0.0, 1.0 );

	// Generate samples
	_data.clear();
    for( unsigned int i = 0; i < nb_data; ++i) {
	  Sample sample { rnd_x(rnd_engine), rnd_y(rnd_engine) };
	  double dist = sqrt( sample[0]*sample[0] + sample[1] * sample[1] );
	  while( (dist < r_min) or (dist > r_max) ) {
		sample = { rnd_x(rnd_engine), rnd_y(rnd_engine) };
		dist = sqrt( sample[0]*sample[0] + sample[1] * sample[1] );
	  }
	  sample[0] += center_x;
	  sample[1] += center_y;
      _data.push_back( sample );
    }

    return _data;
  }
  // ******************************************************* Distribution::R/W
  const Data& read(std::istream& is)
  {
    double x,y;
    _data.clear();
    std::string line;
    while (std::getline(is, line)) {
      // Avoid lines beginning with '#'
      if( line.front() != '#' ) {
	std::istringstream iss(line);
	iss >> x >> y; // 
	_data.push_back( {x,y} );
      }
    }

    return _data;
  }
  void write(std::ostream& os) const
  {
    for( auto& v: _data) {
      os << v[0] << "\t" << v[1] << std::endl;
    }
  }
  // ************************************************* Distribution::attributs
  const Data& data() const { return _data; }
private:
  Data _data;
}; // class Distribution

#endif // DISTRIBUTION_HPP
