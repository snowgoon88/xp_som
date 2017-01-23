/* -*- coding: utf-8 -*- */

#pragma once
#include <list>
#include <iostream>
#include <math.h>

/** 
 * Trace une courbe en OpenGL.
 */
const unsigned int _nb_data = 100;


class Curve
{
public:
  /** Un Sample est un triplet */
  using Sample = struct {
    double x;
    double y;
    double z;
  };

  /** Bounding box around data */
  using BoundingBox = struct {
    double x_min, x_max, y_min, y_max;
  };

  /** Color */
  using Color = struct {
    double r,g,b;
  };

public:
  // ********************************************************* Curve::creation
  /** Creation */
  Curve() : _fg_col{1,0,0}, _line_width(1.f) // red,thin
  {}
  ~Curve() {std::cout << "Curve destroyed" << std::endl;};

  // ************************************************************ Curve::clear
  void clear()
  {
    _data.clear();
  }
  // ******************************************************* Curve::add_sample
  template<typename Itr>
  void add_sample( const Itr& x_data_begin, const Itr& x_data_end,
		   const Itr& y_data_begin, const Itr& y_data_end )
  {
    //std::cout << "Curve::add_sample_with_Itr" << std::endl;
    auto it_x = x_data_begin;
    auto it_y = y_data_begin;
    
    for (; it_x != x_data_end and it_y != y_data_end; ++it_x, ++it_y) {
      //std::cout << "add " << *it_x << ", " << *it_y << std::endl;
      add_sample( {*it_x, *it_y, 0.0} );
    }
  }
  /** Add a point to the Curve and adjust _bbox */
  void add_sample( const Sample& sample)
  {
    //std::cout << "Curve::add_sample" << std::endl;
    // First sample ?
    if (_data.size() == 0 ) {
      //std::cout << "add_sample : NEW" << std::endl;
      _data.push_back( sample );
      _bbox.x_min = sample.x;
      _bbox.x_max = sample.x;
      _bbox.y_min = sample.y;
      _bbox.y_max = sample.y;
    }
    else {
      //std::cout << "add_sample : ADD" << std::endl;
      // Check it is after the last point
      if (sample.x > get_bbox().x_max) {
	//std::cout << "add_sample : OK"  << std::endl;
	_data.push_back( sample );
	_bbox.x_max = sample.x;
	if (sample.y > get_bbox().y_max) _bbox.y_max = sample.y;
	if (sample.y < get_bbox().y_min) _bbox.y_min = sample.y;
      }
    }
  }
  // *************************************************** Curve::add_time_serie
  template<typename Itr>
  void add_time_serie( const Itr& data_begin, const Itr& data_end )
  {
    auto it = data_begin;
    for (auto t_data=0.0; it != data_end; ++it, t_data+=1.0) {
      add_sample( Sample{t_data, it, 0.0} );
    }
  }

  // ******************************************************** Curve::set_color
  void set_color( const Color& col )
  {
    _fg_col = col;
  }
  void set_width( const GLfloat& width )
  {
	_line_width = width;
  }
  
  /** Draw curve with OpenGL */
  void render()
  {
    // for( auto& pt : _data) {
    //   std::cout << "[ " << pt.x << "; " << pt.y << "; " << pt.z << "]" << std::endl;
    // }

    // Couleur rouge
    glColor4d( _fg_col.r, _fg_col.g, _fg_col.b, 1.0);
    // -------------------------------------------------------------------------
    //  Rendering using GL_LINE_STRIP
    // -------------------------------------------------------------------------
    glEnable (GL_BLEND);
    glEnable (GL_LINE_SMOOTH);
    glLineWidth( _line_width );

    glBegin(GL_LINE_STRIP);
    for( auto& pt: _data) {
      glVertex3d( pt.x, pt.y, pt.z );
    }
    glEnd();
    
  }

  /** get BoundingBox */
  const BoundingBox& get_bbox() const {return _bbox;}
  std::list<Sample> get_samples() const { return _data; }

private:
  /** Data are a list of Samples*/
  std::list<Sample> _data;
  /** Bounding box around Data */
  BoundingBox _bbox;
  /** Color for the Curve */
  Color _fg_col;
  /** Line Width */
  GLfloat _line_width;
  
public:
  /** Create artificial data y=sin(x) pour x=[0,2PI[ */
  void create_data()
  {
    std::cout << "Create_data nb=" << _nb_data << std::endl;
    for( unsigned int i=0; i < _nb_data; ++i) {
      Sample pt;
      pt.x = 2.0 * M_PI * i / _nb_data;
      pt.y = sin( pt.x );
      pt.z = 0.0;      
      _data.push_back( pt );
    }
    _bbox = {0.0, 2.0 * M_PI, -1.0, 1.0};
    std::cout << "Curve  size(data)=" << _data.size() << std::endl;
    std::cout << "bbox = {" << get_bbox().x_min <<"; " << get_bbox().x_max << "; ";
    std::cout << get_bbox().y_min << "; " << get_bbox().y_max << "}" << std::endl;
  }
};
