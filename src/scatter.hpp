/* -*- coding: utf-8 -*- */

#ifndef SCATTER_HPP
#define SCATTER_HPP

/** 
 * A special Curve renderer that display a Curve as a scatter plot.
 */

#include <curve.hpp>

// ***************************************************************************
// ************************************************************ ScatterPlotter
// ***************************************************************************
class ScatterPlotter : public Curve 
{
public:
  // ********************************************* ScatterPlotter::constructor
  ScatterPlotter( Curve& curve) : Curve(), 
    _curve(curve), _active(true), _size(0.15f)
  {
  }
  // ***************************************************** ScatterPlotter::set
  void set_size( const GLfloat& size )
  {
    _size = size;
  }
  void set_active( const bool& active )
  {
    _active = active;
  }
  bool is_active()
  {
    return _active;
  }
  // ************************************************** ScatterPlotter::render
  /** Draw diamonds with classic OpenGL */
  virtual void render( float screen_ratio_x = 1.0, float screen_ratio_y = 1.0 )
  {
    if (not _active ) return;
                     
    // Color
    glColor4d( _fg_col.r, _fg_col.g, _fg_col.b, 1.0);
    // -------------------------------------------------------------------------
    //  Rendering using GL_LINE_STRIP
    // -------------------------------------------------------------------------
    glEnable (GL_BLEND);
    glEnable (GL_LINE_SMOOTH);
    glLineWidth( _line_width );

    glBegin(GL_LINES);
    for( auto& pt: _curve.get_samples() ) {
      glVertex3d( pt.x + _size, pt.y, pt.z );
      glVertex3d( pt.x, pt.y + _size, pt.z );

      glVertex3d( pt.x, pt.y + _size, pt.z );
      glVertex3d( pt.x - _size, pt.y, pt.z );

      glVertex3d( pt.x - _size, pt.y, pt.z );
      glVertex3d( pt.x, pt.y - _size, pt.z );

      glVertex3d( pt.x, pt.y - _size, pt.z );
      glVertex3d( pt.x + _size, pt.y, pt.z );
    }
    glEnd();
  }
protected:
  // *********************************************** ScatterPlotter::attributs
  Curve& _curve;
  bool _active;
  GLfloat _size;
};

#endif // SCATTER_HPP
