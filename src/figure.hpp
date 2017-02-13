/* -*- coding: utf-8 -*- */

#ifndef FIGURE_HPP
#define FIGURE_HPP

/** 
 * A Window with a Figure inside to plot several curves.
 */
#include <GLFW/glfw3.h>
#include <iostream>                  // std::cout
#include <string>                    // std::string

#include <curve.hpp>
#include <axis.hpp>

#include <list>
#include <memory>

static void error_callback(int error, const char* description );
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// ***************************************************************************
// ******************************************************************** Figure
// ***************************************************************************
class Figure
{
public:
  // *********************************************************** Figure::Types
  using CurvePtr = Curve* ;
  using CurveList = std::list<CurvePtr>;
public:
  // ******************************************************** Figure::creation
  Figure( std::string title = "Figure",
		  const int width=640, const int height=400,
		  const Range& x_range = {-1.0, 1.0, 10, 2},
		  const Range& y_range = {-1.0, 1.0, 10, 2} ) :
    _title( title ), _width(width), _height(height),
    _window(nullptr), _curves(),
    _draw_axes( true ),
    _axis_x( "X", x_range),
    _axis_y( "Y", y_range)
  {
	// Create window _________________________________________________
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);
    
    _window = glfwCreateWindow(_width, _height, _title.c_str(), NULL, NULL);
    if (! _window ) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent( _window );
	// TODO can also be set to another DataStructure
	glfwSetWindowUserPointer( _window, this);
    glfwSetKeyCallback( _window, key_callback);
  }
  // ***************************************************** Figure::destruction
  ~Figure()
  {
	glfwSetWindowShouldClose(_window, GL_TRUE);
    if(_window)
      glfwDestroyWindow( _window);
  }
  // ******************************************************* Figure::add_curve
  CurvePtr add_curve( CurvePtr curve )
  {
	_curves.push_back( curve );

	return curve;
  }
  // *************************************************** Figure::set_draw_axes
  void set_draw_axes( bool draw_axes )
  {
    _draw_axes = draw_axes;
  }
  // ********************************************************** Figure::render
  void render( bool update_axes=false )
  {
	glfwMakeContextCurrent( _window );
	// TODO can also be set to another DataStructure
	glfwSetWindowUserPointer( _window, this);

	if( update_axes ) {
	  // Build proper axis by finding min/max on each axe
	  Curve::BoundingBox bbox{ std::numeric_limits<double>::max(),
		  (-std::numeric_limits<double>::max()),
		  std::numeric_limits<double>::max(),
		  -std::numeric_limits<double>::max() };
	  
	  for( const auto& curve: _curves ) {
		auto b = curve->get_bbox();
		if( b.x_min < bbox.x_min ) bbox.x_min = b.x_min;
		if( b.x_max > bbox.x_max ) bbox.x_max = b.x_max;
		if( b.y_min < bbox.y_min ) bbox.y_min = b.y_min;
		if( b.y_max > bbox.y_max ) bbox.y_max = b.y_max;
	  }

	  _axis_x = Axis( "X", {bbox.x_min,bbox.x_max, 10, 2});
	  _axis_y = Axis( "Y", {bbox.y_min,bbox.y_max, 10, 2});
	}
	
	// get window size
	glfwGetFramebufferSize( _window, &_width, &_height);
	// Info for scaling View and axes
    auto x_min_win = _axis_x.get_range()._min
      - 0.08 * (_axis_x.get_range()._max - _axis_x.get_range()._min);
    auto x_max_win = _axis_x.get_range()._max
      + 0.08 * (_axis_x.get_range()._max - _axis_x.get_range()._min);
    auto ratio_x = (x_max_win-x_min_win) / (double) _width;
    auto y_min_win = _axis_y.get_range()._min
      - 0.08 * (_axis_y.get_range()._max - _axis_y.get_range()._min);
    auto y_max_win = _axis_y.get_range()._max
      + 0.08 * (_axis_y.get_range()._max - _axis_y.get_range()._min);
    auto ratio_y = (y_max_win-y_min_win) / (double) _height;

	glViewport(0, 0, _width, _height);
	glClearColor( 1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho( x_min_win, x_max_win, y_min_win, y_max_win, 1.f, -1.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Basic axes
	if( _draw_axes ) {
	  _axis_x.render( ratio_x, ratio_y );
	  glPushMatrix(); // AXE_Y
	  glRotated( 90.0, 0.0, 0.0, 1.0 );
	  _axis_y.render( ratio_y, ratio_x);
	  glPopMatrix(); // AXE_Y
	}
	// All other objects
	for( const auto& curve: _curves) {
	  curve->render();
	}
	glfwSwapBuffers( _window );
	glfwPollEvents();	
  }

public:
  // ******************************************************* Figure::attributs
  std::string _title;
  int _width, _height;
  GLFWwindow* _window;
  /** All the curves */
  CurveList _curves;
  /** X and Y axes*/
  bool _draw_axes;
  Axis _axis_x, _axis_y;
};


#endif // FIGURE_HPP
