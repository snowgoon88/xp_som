/* -*- coding: utf-8 -*- */

#ifndef SIMUL_HPP
#define SIMUL_HPP

/** 
 * Simulateur simple avec un Robot.
 * Le Robot a une position TVec2 _pos et une orientation TAng _ang.
 * Il a une vitesse linéaire TPos _lin_spd et une vitesse de rotation TAng _ang_spd.
 */

#include <sstream>                 // std::stringstream
#include <Eigen/Dense>             // Eigen::Vector2d


// ********************************************************************* TYPES
typedef double                     Pos;
typedef double                     Ang;
typedef double                     Time;
typedef Eigen::Matrix<Pos, 2, 1>  Vec2;

std::ostream& operator<<( std::ostream& os, const Vec2& v)
{
  return os << "(" << v[0] << "; " << v[1] << ")";
}
// ********************************************************************* ROBOT
class Robot
{
public:
  // ****************************************************************** CREATION
  Robot() : _pos(0,0), _ang(0), _spd_lin(0), _spd_ang(0)
  {};
  // *********************************************************************** STR
  std::string str_dump()
  {
    std::stringstream str;
    str << "_pos=" << _pos << " _ang=" << _ang;
    str << " _spd=(" << _spd_lin << "; " << _spd_ang << ")";

    return str.str();
  };
  // *********************************************************************** SET
  void set_pose( Pos x, Pos y, Ang angle )
  { _pos(0) = x; _pos(1) = y; _ang = angle; };
  void set_speed( Pos linear, Ang angular )
  { _spd_lin = linear; _spd_ang = angular; };
  // ******************************************************************** UPDATE
  void update( Time delta_t )
  {
    // Angle
    _ang += _spd_ang * delta_t;
    // Position
    Vec2 dep( cos(_ang), sin(_ang));
    _pos += _spd_lin * dep * delta_t;
  };
  
  // ***************************************************************** ATTRIBUTS
  Vec2 _pos;
  Ang  _ang;
  Pos  _spd_lin;
  Ang  _spd_ang;
};

// ********************************************************************* SIMUL
class Simul
{
public:
  // ****************************************************************** CREATION
  Simul( Pos width=10, Pos height=10) :
    _width(width), _height(height), _rob()
  {};
  // *********************************************************************** STR
  std::string str_dump()
  {
    std::stringstream str;

    str << "ROBOT : " << _rob.str_dump();

    return str.str();
  };
  // ******************************************************************** UPDATE
  void update( Time delta_t )
  {
    _rob.update( delta_t );
  };
  // ***************************************************************** ATTRIBUTS
  /** Size of the world */
  Pos _width, _height;
  /** A Robot */
  Robot _rob;
};


#endif // SIMUL_HPP
