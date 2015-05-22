/* -*- coding: utf-8 -*- */

#ifndef SIMUL_HPP
#define SIMUL_HPP

/** 
 * Simulateur simple avec un Robot.
 * Le Robot a une position TVec2 _pos et une orientation TAng _ang.
 * Il a une vitesse linéaire TPos _lin_spd et une vitesse de rotation TAng _ang_spd.
 *
 * update(delta_t en secondes) permet de faire évoluer les éléments du Simulateur.
 */

#include <sstream>                 // std::stringstream
#include <Eigen/Dense>             // Eigen::Vector2d
#include <list>                    // std::list

// ********************************************************************* TYPES
typedef double                     Pos;
typedef double                     Ang;
typedef double                     Time;
typedef Eigen::Matrix<Pos, 2, 1>  Vec2;

std::ostream& operator<<( std::ostream& os, const Vec2& v)
{
  return os << "(" << v[0] << "; " << v[1] << ")";
}
// ***************************************************************************
// ********************************************************************* ROBOT
// ***************************************************************************
class Robot
{
public:
  // ****************************************************************** creation
  Robot() : _pos(0,0), _ang(0), _spd_lin(0), _spd_ang(0)
  {};
  // *********************************************************************** str
  std::string str_dump()
  {
    std::stringstream str;
    str << "_pos=" << _pos << " _ang=" << _ang;
    str << " _spd=(" << _spd_lin << "; " << _spd_ang << ")";

    return str.str();
  };
  // *********************************************************************** set
  void set_pose( Pos x, Pos y, Ang angle )
  { _pos(0) = x; _pos(1) = y; _ang = angle; };
  void set_speed( Pos linear, Ang angular )
  { _spd_lin = linear; _spd_ang = angular; };
  // ******************************************************************** update
  void update( Time delta_t )
  {
    // Angle
    _ang += _spd_ang * delta_t;
    // Position
    Vec2 dep( cos(_ang), sin(_ang));
    _pos += _spd_lin * dep * delta_t;
  };
  
  // ***************************************************************** attributs
  Vec2 _pos;
  Ang  _ang;
  Pos  _spd_lin;
  Ang  _spd_ang;
};
// ***************************************************************************
// ****************************************************************** OBSTACLE
// ***************************************************************************
class Obstacle
{
public:
  // ****************************************************************** creation
  Obstacle(Vec2 pos, Pos radius) : _pos(pos), _radius(radius) {};
  // *********************************************************************** str
  std::string str_dump()
  {
    std::stringstream str;
    str << "_pos=" << _pos << " _radius=" << _radius;

    return str.str();
  };
  // ***************************************************************** intersect
  /**
   * Does a half-line starting from 'pt' going to 'dir' intersect this Obstacle ?
   * => No : return false
   * => Yes : nearest_pt is the intersection point nearest to pt,
   *          lambda is its abcisse along the half-line.
   */
  bool intersect( const Vec2& pt, const Vec2& dir, Vec2& nearest_pt, double& lambda )
  {
    // vector along dir, but of length one
    Vec2 v = dir / sqrt(dir.dot(dir));
    
    // Distance between half-line and Obstacle (as a circle)
    double coord = v.dot(_pos - pt);
    std::cout << "coord = " << coord << std::endl;
    // if not positive, no intersection of the half-line
    if( coord < 0.0 ) return false;

    // Distance is the norm of vecteur (pt+coord*v - _pos)
    nearest_pt = (pt+coord*v) - _pos;
    double dist = sqrt(nearest_pt.dot(nearest_pt));
    std::cout << "nearest = " << nearest_pt << " dist=" << dist << std::endl;
    // if outside of circle => no intersection
    if( dist > _radius ) return false;

    // Nearest point
    lambda = coord - sqrt( _radius*_radius - dist*dist );
    nearest_pt = pt + lambda * v;
    
    return true;
  }
  // ***************************************************************** attributs
  Vec2 _pos;
  Pos  _radius;
};
// ***************************************************************************
// ********************************************************************* SIMUL
// ***************************************************************************
class Simul
{
public:
  // ****************************************************************** creation
  Simul( Pos width=10, Pos height=10) :
    _width(width), _height(height), _rob()
  {};
  // *********************************************************************** str
  std::string str_dump()
  {
    std::stringstream str;

    str << "ROBOT : " << _rob.str_dump();
    for( auto& obst: _l_obst) {
      str << "OBST : " << obst.str_dump() << std::endl;
    }
    return str.str();
  };
  // ******************************************************************** update
  void update( Time delta_t )
  {
    _rob.update( delta_t );
  };
  // ***************************************************************** attributs
  /** Size of the world */
  Pos _width, _height;
  /** A Robot */
  Robot _rob;
  /** Obstacles list */
  std::list<Obstacle> _l_obst;
};

#endif // SIMUL_HPP
