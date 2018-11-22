/* -*- coding: utf-8 -*- */

#ifndef PLOTTER_HPP
#define PLOTTER_HPP

/** 
 * Basic characteristics of Plotter
 * _bbox
 * render( ratio_x, ratio_y )
 */

// ***************************************************************************
// ******************************************************************* Plotter
// ***************************************************************************
class Plotter
{
public:
  // ********************************************************** Plotter::TYPES
  /** Bounding box around data */
  using BoundingBox = struct {
    double x_min, x_max, y_min, y_max;
  };
  // ****************************************************** Plotter::creation
  Plotter() : _bbox{0.0, 1.0, 0.0, 1.0}
  {
  }
  Plotter( double x_min, double x_max, double y_min, double y_max) :
    _bbox{x_min, x_max, y_min, y_max}
  {
  }
  virtual ~Plotter()
  {
  }
  // ********************************************************* Plotter::render
  virtual void render( float screen_ratio_x = 1.0, float screen_ratio_y = 1.0 )
  {
  }
  // ****************************************************** Plotter::attributs
  /** get BoundingBox */
  const BoundingBox& get_bbox() const {return _bbox;}

  BoundingBox _bbox;
  
}; // Plotter

#endif // PLOTTER_HPP
