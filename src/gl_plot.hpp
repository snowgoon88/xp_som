/* -*- coding: utf-8 -*- */

#ifndef GL_PLOT_HPP
#define GL_PLOT_HPP

/** 
 * GLFW Window with a plot several Curve.
 * - TODO add_curve
 * - TODO show()
 */

#include <GLFW/glfw3.h>
#include <iostream>                  // std::cout
#include <string>                    // std::string
#include <limits>                    // max dbl
#include <thread>                    // std::thread
#include <chrono>                    //std::chrono

#include <curve.hpp>
#include <axis.hpp>

// ***************************************************************************
// ******************************************************************** GLPlot
// ***************************************************************************
class GLPlot
{
public:
  // *********************************************************** GLplot::alias
  using CurvePtr = std::unique_ptr<Curve>;
  using CurveList = std::list<CurvePtr>;
public:
  // ******************************************************** GLPlot::creation
  GLPlot( const std::string& title = "Plot",
	  const int width=640, const int height=400 ) :
    _title( title ), _width(width), _height(height),
    _window(nullptr), _curve_list()
  {
  }
  // ***************************************************** GLPlot::destruction
  ~GLPlot()
  {
    if(_window)
      glfwDestroyWindow( _window);
    
    glfwTerminate();
  }
  // ******************************************************* GLPlot::add_curve
  void add_curve( CurvePtr curve )
  {
    _curve_list.push_back( std::move(curve) );
  }
  // ************************************************************ GLPlot::show
  void show()
  {
    // // Window creation
    // glfwSetErrorCallback(error_callback);

    // if (!glfwInit())
    //     exit(EXIT_FAILURE);
    
    // _window = glfwCreateWindow(_width, _height, _title.c_str(), NULL, NULL);
    // if (! _window ) {
    //   glfwTerminate();
    //   exit(EXIT_FAILURE);
    // }
    // glfwMakeContextCurrent( _window );
    // glfwSetKeyCallback( _window, key_callback);

    // Prepare context________________________________________________
    // Build proper axis by finding min/max on each axe
    BoundingBox bbox{
      std::numeric_limits<double>::max(),
	-std::numeric_limits<double>::max(),
	std::numeric_limits<double>::max(),
	-std::numeric_limits<double>::max() };
    for( const auto& curve: _curve_list) {
      auto b = curve->get_bbox();
      if( b.x_min < bbox.x_min ) bbox.x_min = b.x_min;
      if( b.x_max > bbox.x_max ) bbox.x_max = b.x_max;
      if( b.y_min < bbox.y_min ) bbox.y_min = b.y_min;
      if( b.y_max > bbox.y_max ) bbox.y_max = b.y_max;
    }
    auto _axis_x = Axis( "time", {bbox.x_min,bbox.x_max,2, 10});
    auto _axis_y = Axis( "obs",  {bbox.y_min,bbox.y_max,2, 10});

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
    glfwSetKeyCallback( _window, key_callback);

    // Render_________________________________________________________
    while (!glfwWindowShouldClose( _window )) {
      glfwGetFramebufferSize( _window, &_width, &_height);

      glViewport(0, 0, _width, _height);
      glClearColor( 1.0, 1.0, 1.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho( x_min_win, x_max_win, y_min_win, y_max_win, 1.f, -1.f);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      // Basic axes
      _axis_x.render( ratio_x, ratio_y );
      glPushMatrix(); // AXE_Y
      glRotated( 90.0, 0.0, 0.0, 1.0 );
      _axis_y.render( ratio_y, ratio_x);
      glPopMatrix(); // AXE_Y

      // All other objects
      for( const auto& curve: _curve_list) {
	curve->render();
      }
      glfwSwapBuffers( _window );
      glfwPollEvents();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
private:
  // ******************************************************* GLPlot::attributs
  /** Windows attributes */
  std::string _title;
  int _width, _height;
  /** Window ptr */
  GLFWwindow* _window;
  /** List of Curves */
  CurveList _curve_list;

  // ******************************************************* GLPlot::callbacks
  /**
   * Callback for 'key' events
   */
  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GL_TRUE);
  }
  /**
   * Callback for GLFW errors
   */
  static void error_callback(int error, const char* description)
  {
    std::cerr <<  description << std::endl;
    //fputs(description, stderr);
  }
};

#endif // GL_PLOT_HPP
