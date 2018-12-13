/* -*- coding: utf-8 -*- */

/** 
 * 001-curveoffscreen.cpp
 *
 * Full VisuGL example rendered OFFSCREEN and saved as 001-curveoffscreen.png
 * Window + Figure with title
 *   + Curve as example
 * 
 * 'S' or 's' -> save drawing as "001-curve.png"
 */

#include <string>
#include <stdlib.h>
#include <iostream>

#include <window.hpp>
#include <figure.hpp>
#include <curve.hpp>

/** Window, Figure and Plotters */
Window* _win;
Figure* _fig;
Curve*  _curve;

/** Parameters */
double time_glfw = 0;

/** Graphic */
bool _end_render = false;

//******************************************************************************
/**
 * Callback for keyboard events
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
  // else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
  //   // save window
  //   _win->save( "001-curve.png" );
  // }
}

// ***************************************************************************
// ********************************************************* Graphic Functions
// ***************************************************************************
/**
 * Init and end GLFW
*/
void init_glfw()
{
  std::cout << "__GLFW Init" << std::endl;
  
  glfwSetErrorCallback(Window::error_callback);
  
  if (!glfwInit())
        exit(EXIT_FAILURE);
}
void glfw_end()
{
  glfwTerminate();
  std::cout << "__GLFW destroyed" << std::endl;
}
// // ******************************************************************** render
// void render()
// {
//   while( not _end_render and !glfwWindowShouldClose(_win->_window) ) {
//     time_glfw = glfwGetTime();
    
//     _win->render();

//   }
// }
//******************************************************************************
int main( int argc, char *argv[] )
{
  // A static Curve
  std::cout << "__CURVE" << std::endl;
  _curve = new Curve(); // default is red thin line
  _curve->clear();
  
  const unsigned int _nb_data = 100;
  for( unsigned int i=0; i < _nb_data; ++i) {
    Curve::Sample pt;
      pt.x = 2.0 * M_PI * i / _nb_data;
      pt.y = sin( pt.x );
      pt.z = 0.0;      
      _curve->add_sample( pt );
  }

  
  // ****** GRAPHIC ********
  init_glfw();
  std::cout << "__WINDOW and FIGURE" << std::endl;
  _win = new Window( "Top Window", 600, 600, true /*offscreen*/ );
  _fig = new Figure( *_win, "sin( 2 * \\pi * x )     [ESC:quit, S:save as 001-curve.png]" );
  _win->add_plotter( _fig );
  _fig->add_plotter( _curve );

  _win->update_bbox();
  
  std::cout << "__RENDER" << std::endl;
  // render only once then save
  _win->render();
  _win->save( "001-curveoffscreen.png" );

  std::cout << "__END" << std::endl;
  delete _curve;
  delete _fig;
  delete _win;

  glfw_end();
  
  return 0;
}
