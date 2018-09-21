/* -*- coding: utf-8 -*- */

/** 
 * test-020-anttweak.cpp
 *
 * Figure + AnttweakBar ?
 */

#include <figure_ant.hpp>
#include <curve.hpp>
#include <scatter.hpp>

#include <AntTweakBar.h>

#include <GLFW/glfw3.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <memory>

/** Figure and curves */
FigureAnt* _fig;
Curve* _c_sin;
float _curve_color[] = { 0.7f, 0.5f, 0.6f };
ScatterPlotter* _scatter;
bool _scatter_active = false;
float _scatter_size = 0.15;
float _scatter_color[] = { 1.0f, 0.0f, 0.0f };

/** Parameters */
double time_glfw = 0;

/** Graphic */
bool _end_render = false;

// ***************************************************************************
// ********************************************************* Graphic Functions
// ***************************************************************************
/**
 * Init and end GLFW
*/
void init_glfw()
{
  std::cout << "__GLFW Init" << std::endl;
  
  glfwSetErrorCallback(error_callback);
  
  if (!glfwInit())
        exit(EXIT_FAILURE);
}
void glfw_end()
{
  glfwTerminate();
  std::cout << "__GLFW destroyed" << std::endl;
}
// ******************************************************************** render
void render()
{
  while( not _end_render and !glfwWindowShouldClose(_fig->_window) ) {
    time_glfw = glfwGetTime();
    
    _c_sin->set_color( {_curve_color[0], _curve_color[1], _curve_color[2]} );
    _scatter->set_active( _scatter_active );
    _scatter->set_size( _scatter_size );
    _scatter->set_color( {_scatter_color[0], _scatter_color[1], _scatter_color[2]} );
    _fig->render( true, true ); /* update x_axis, y_axis */

  }
}
// ***************************************************************************
/**
 * Callback for GLFW errors
 */
static void error_callback(int error, const char* description)
{
  std::cerr <<  description << std::endl;
}
/**
 * Callback for GLFW key events
 */
static void key_callback(GLFWwindow* window, int key, int scancode,
                         int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    _end_render = true;
  }
  TwEventKeyGLFW3( window, key, scancode, action, mods);
}
// ***************************************************************************


// ***************************************************************************
// ********************************************************************** Main
// ***************************************************************************
int main(int argc, char *argv[])
{
  init_glfw();

  // Create Figure (thus Window)
  std::cout << "__FIGURE" << std::endl;
  _fig = new FigureAnt( "Figure and AntTweakBar" );
  
  // Create AntTweakBar
  // Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit */
  TwAddVarRO( _fig->_bar, "time", TW_TYPE_DOUBLE, &time_glfw,
             " label='Time' precision=1 help='Time (in seconds).' ");

    
  // curve_color
  TwAddVarRW( _fig->_bar, "curve_color", TW_TYPE_COLOR3F, &_curve_color, 
              " label='Curve color' help='Color of the curve.' ");

  // scatter active ?
  TwAddVarRW( _fig->_bar, "scattter ?", TW_TYPE_BOOLCPP, &_scatter_active,
              " label='Plot scatter ?' help='Switch between plot/no-plot' ");

  // scatter size ?
  TwAddVarRW( _fig->_bar, "size", TW_TYPE_FLOAT, &_scatter_size,
              " label='Scatter Size' min=0 step=0.01 help='Size of each marker.' " );
  // scatter_color
  TwAddVarRW( _fig->_bar, "scatter_color", TW_TYPE_COLOR3F, &_scatter_color, 
              " label='Scatter color' help='Color of the scatter marker.' ");
  
  // Curves
  std::cout << "__Curves" << std::endl;
  _c_sin = new Curve();
  _c_sin->create_data();
  _fig->add_curve( _c_sin );
  // ScatterPlotter
  _scatter = new ScatterPlotter( *_c_sin );
  _fig->add_curve( _scatter );

  std::cout << "__RENDER" << std::endl;
  render();

  std::cout << "__END" << std::endl;
  delete _c_sin;
  delete _scatter;
  delete _fig;

  glfw_end();

  return 0;
}

// *********************************************************************** END
