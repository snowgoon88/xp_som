/* -*- coding: utf-8 -*- */

/** 
 * 004-img_cmap.cpp
 *
 * VisuGL example.
 * Window with title
 *   + Figure 
 *      + Img using ColormapPlotter<std::vector<double>>
 *   + CmapPlotter<std::vector<double>>
 */


#include <string>
#include <stdlib.h>
#include <iostream>

#include <vector>
#include <random>

#include <window.hpp>
#include <figure.hpp>
#include <img_plotter.hpp>
#include <cmap_plotter.hpp>

/** Figure and plotters */
Window* _win;
Figure* _fig;
ImgPlotter<std::vector<double>>* _img_plotter;
ColormapPlotter<std::vector<double>>* _cmap_plotter;

/** Random generator */
// init random double generation
std::random_device _rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 _gen(_rd()); //Standard mersenne_twister_engine seeded with rd()
std::uniform_real_distribution<> _dis(-0.1, 0.1);

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

// ******************************************************************** render
void update_and_render()
{
  glfwSetTime(0.0);

  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "UPD::OpenGL error: " << err << std::endl;
  }
  unsigned int nb_render = 0;
  while( not _end_render and !glfwWindowShouldClose(_win->_window) ) {    

    _win->update_bbox();
    
    while ((err = glGetError()) != GL_NO_ERROR) {
      std::cerr << "UPDA " << nb_render << " OpenGL error: " << err << std::endl;
    }
    _win->render();

    while ((err = glGetError()) != GL_NO_ERROR) {
      std::cerr << "REND " << nb_render << " OpenGL error: " << err << std::endl;
    }
    nb_render++;
  }
}

//******************************************************************************
int main( int argc, char *argv[] )
{
  // Create some data 100 x 200 doubles
  std::vector<double> data;
  for( unsigned int iy = 0; iy < 100; ++iy) {
    double y = -1.0 + double(iy)/(99.0) * 2.0;
    for( unsigned int ix = 0; ix < 200; ++ix) {
      double x = -2.0 + double(ix) / (199.0) * 4.0;

      data.push_back( sin( 10 * (x*x + y*y)) + _dis(_gen));
    }
  }
  
  // ****** GRAPHIC ********
  init_glfw();
  GLenum err;
  std::cout << "__ READ buffer start" << std::endl;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "INIT::OpenGL error: " << err << std::endl;
  }
  std::cout << "__WINDOW and FIGURE" << std::endl;
  _win = new Window( "Top Window", 600, 600 );
  _fig = new Figure( *_win, "sin( 10 (x^2 + y^2)= + noise    [ESC:quit]" );
  // VisuGL elements
  _img_plotter = new ImgPlotter<std::vector<double>>( data, 200, 100,
                                                      -0.2, 0.8, -0.2, 0.8);
  _cmap_plotter = new ColormapPlotter<std::vector<double>>( *_win,
                                                            *_img_plotter );
  
  _win->add_plotter( _fig );
  _fig->add_plotter( _img_plotter );
  _win->add_plotter( _cmap_plotter );
  _win->update_bbox();
  
  std::cout << "__RENDER" << std::endl;
  update_and_render();

  std::cout << "__END" << std::endl;
  delete _cmap_plotter;
  delete _img_plotter;
  delete _fig;
  delete _win;

  glfw_end();
  
  return 0;
}
