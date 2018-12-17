/* -*- coding: utf-8 -*- */

/** 
 * 002-dyncurve.cpp
 *
 * VisuGL example.
 * Window + Figure with title
 *   + Curve to which we periodically add samples
 *   + CurveMean that can switch between "NORMAL" and "MEAN" mode (key W)
 * 
 * 'W' or 'w' -> switch between "normal" and "mean" mode
 * 'D' or 'd' -> print nb of samples in each curve
 * 'S' or 's' -> save drawing as "001-curve.png"
 */

#include <string>
#include <stdlib.h>
#include <iostream>

#include <vector>

#include <window.hpp>
#include <figure.hpp>
#include <curve.hpp>

/** Window, Figure and Plotters */
Window* _win;
Figure* _fig;
Curve*  _curve;
CurveMean* _curve_mean;

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
  // S or s : save window
  else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    // save window
    _win->save( "002-dyncurve.png" );
  }
  // W or w : switch _curve_mean_mode 
  else if ( key == GLFW_KEY_Z && action == GLFW_PRESS) {
    // toggle _mean
    if( _curve_mean->get_mean_mode() ) {
      std::cout << "  Normal mode" << std::endl;
      _curve_mean->set_mean_mode( false );
    }
    else {
      std::cout << "  MEAN mode" << std::endl;
      _curve_mean->set_mean_mode( true );
      _curve_mean->recompute_means();
    }
  }
  // display nb of samples in curves
  else if ( key == GLFW_KEY_D && action == GLFW_PRESS ) {
    std::cout << _curve->get_samples().size() << " samples in each curve" << std::endl;
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
    // add data to _curve
    auto time_glfw = glfwGetTime();
    auto new_y = time_glfw/10.0 + sin(time_glfw)+0.1*sin(50.0*time_glfw);
    _curve->add_sample( {time_glfw, new_y, 0.0} );
    _curve_mean->add_sample( {time_glfw, new_y, 0.0} );

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
  // A dynamic Curve and its mean
  std::cout << "__CURVE" << std::endl;
  _curve = new Curve(); // default is red thin line
  _curve->clear();
  _curve_mean = new CurveMean();
  _curve_mean->set_color( {0.0, 0.0, 1.0} );
  _curve_mean->set_width( 2.0f );
  _curve_mean->clear();
  
  // ****** GRAPHIC ********
  init_glfw();
  GLenum err;
  std::cout << "__ READ buffer start" << std::endl;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "INIT::OpenGL error: " << err << std::endl;
  }
  std::cout << "__WINDOW and FIGURE" << std::endl;
  _win = new Window( "Top Window", 600, 600 );
  _fig = new Figure( *_win, "Dyn Curve    [ESC:quit, W:Mean ON/OFF, S:save as 002-dyncurve.png; D:info]" );
  _fig->_update_axes_x = true;
  _win->add_plotter( _fig );
  _fig->add_plotter( _curve_mean );
  _fig->add_plotter( _curve );      // will overwrite other curve if identical
  _win->update_bbox();
  
  std::cout << "__RENDER" << std::endl;
  update_and_render();

  std::cout << "__END" << std::endl;
  delete _curve;
  delete _fig;
  delete _win;

  glfw_end();
  
  return 0;
}
