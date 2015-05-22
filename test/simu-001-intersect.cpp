/* -*- coding: utf-8 -*- */

/** 
 * simu-001-intersect.cpp
 *
 * Test l'intersection entre une demi-droite et un cercle,
 * pour les obstacles.
 */

#include <iostream>       // std::cout

#include <simul.hpp>
#include <assert.h>

//******************************************************************************
int main( int argc, char *argv[] )
{
  Obstacle obst( Vec2(3,1), 1.0);
  std::cout << "__CREATION" << obst.str_dump() << std::endl;

  Vec2 pt_res;
  double dist;
  
  std::cout << "Pas intersection à cause direction" << std::endl;
  assert( obst.intersect( Vec2(1,0), Vec2(-1,1), pt_res, dist ) == false); 

  std::cout << "Pas intersection car trop loin" << std::endl;
  assert( obst.intersect( Vec2(1,0), Vec2(0,1), pt_res, dist ) == false);

  std::cout << "Intersection en 3,0" << std::endl;
  assert( obst.intersect( Vec2(1,0), Vec2(1,0), pt_res, dist ) == true);
  assert( pt_res == Vec2(3,0) );
  assert( dist == 2.0 );
  std::cout << "===> pt=" << pt_res << "d=" << dist << std::endl;

  std::cout << "Intersection en 3,0" << std::endl;
  assert( obst.intersect( Vec2(1,0), Vec2(1.3,0), pt_res, dist ) == true);
  assert( pt_res == Vec2(3,0) );
  assert( dist == 2.0 );
  std::cout << "===> pt=" << pt_res << "d=" << dist << std::endl;

  std::cout << "Double Intersection => (2,1), d=1.41" << std::endl;
  assert( obst.intersect( Vec2(1,0), Vec2(1,1), pt_res, dist ) == true);
  std::cout << "===> pt=" << pt_res << "d=" << dist << std::endl;

  std::cout << "Double Intersection" << std::endl;
  assert( obst.intersect( Vec2(1,0), Vec2(2,1), pt_res, dist ) == true);
  std::cout << "===> pt=" << pt_res << "d=" << dist << std::endl;

  return 0;
}
