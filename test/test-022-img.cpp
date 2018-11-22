/* -*- coding: utf-8 -*- */

/** 
 * test-022-img.cpp
 *
 * Display data as images in Figure
 */

#include <string>
#include <stdlib.h>
#include <iostream>

#include <figure.hpp>
#include <img_plotter.hpp>
#include <cmap_plotter.hpp>
#include <math.h>

/** Figure and plotters */
Figure* _fig;
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
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
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
  while( not _end_render and !glfwWindowShouldClose(_fig->_window) ) {
    time_glfw = glfwGetTime();
    
    _fig->render( true, true ); /* update x_axis, y_axis */

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
  _fig = new Figure( "ImgPlotter", 600, 600 );
  _img_plotter = new ImgPlotter<std::vector<double>>( data, 200, 100,
                                                      -0.5, 0.8, 0.0, 0.6);
  _cmap_plotter = new ColormapPlotter<std::vector<double>>( *_img_plotter );
 
  _fig->add_plotter( _img_plotter );
  _fig->add_plotter( _cmap_plotter );
  
  std::cout << "__RENDER" << std::endl;
  render();

  std::cout << "__END" << std::endl;
  delete _img_plotter;
  delete _fig;

  glfw_end();
  
  return 0;
}
