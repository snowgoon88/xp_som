/* -*- coding: utf-8 -*- */

/** 
 * test-023-visugl.cpp
 *
 * Full VisuGL example.
 * - Window with title
 *   + Curve as example
 */

#include <string>
#include <stdlib.h>
#include <iostream>

#include <window.hpp>
#include <figure.hpp>
#include <curve.hpp>
#include <img_plotter.hpp>
#include <cmap_plotter.hpp>

/** Window, Figure and Plotters */
Window* _win;
Figure* _fig;
Curve*  _curve_example;
ImgPlotter<std::vector<double>>* _img_plotter;
ColormapPlotter<std::vector<double>>* _cmap_plotter;

/** Parameters */
double time_glfw = 0;

/** Graphic */
bool _end_render = false;

//******************************************************************************
/**
 * Callback qui gère les événements 'key'
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
  else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    // save window
    _win->save( "top_win.png" );
  }
}
// ***************************************************************************
/**
 * Callback pour gérer les messages d'erreur de GLFW
 */
static void error_callback(int error, const char* description)
{
  std::cerr <<  description << std::endl;
  //fputs(description, stderr);
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
  while( not _end_render and !glfwWindowShouldClose(_win->_window) ) {
    time_glfw = glfwGetTime();
    
    _win->render();

  }
}
//******************************************************************************
int main( int argc, char *argv[] )
{
  // Create some data
  std::vector<double> data;
  for( unsigned int iy = 0; iy < 100; ++iy) {
    double y = -1.0 + double(iy)/(99.0) * 2.0;
    for( unsigned int ix = 0; ix < 200; ++ix) {
      double x = -2.0 + double(ix) / (199.0) * 4.0;

      data.push_back( sin( 10 * (x*x + y*y)) );
    }
  }
  
  // ****** GRAPHIC ********
  init_glfw();
  _win = new Window( "Top Window", 600, 600 );
  _fig = new Figure( *_win, "Titre de la Figure" );
  _win->add_plotter( _fig );
  
  _curve_example = new Curve();
  // _curve_example->create_data();
  const unsigned int _nb_data = 100;
    for( unsigned int i=0; i < _nb_data; ++i) {
      Curve::Sample pt;
      pt.x = 2.0 * M_PI * i / _nb_data;
      pt.y = sin( pt.x );
      pt.z = 0.0;      
      _curve_example->add_sample( pt );
    }
  _fig->add_plotter( _curve_example );
  //_fig->update_axes(); // explicitely or set _fig->_update_axes_x/y
  _img_plotter = new ImgPlotter<std::vector<double>>( data, 200, 100,
                                                      -0.5, 0.8, 0.0, 0.6);
  _fig->add_plotter( _img_plotter );

  _cmap_plotter = new ColormapPlotter<std::vector<double>>( *_win, *_img_plotter );
  _win->add_plotter( _cmap_plotter );
  std::cout << "__UPD_BBOX" << std::endl;
  _win->update_bbox();
  
  std::cout << "__RENDER" << std::endl;
  render();

  std::cout << "__END" << std::endl;
  delete _cmap_plotter;
  delete _img_plotter;
  delete _curve_example;
  delete _fig;
  delete _win;

  glfw_end();
  
  return 0;
}
