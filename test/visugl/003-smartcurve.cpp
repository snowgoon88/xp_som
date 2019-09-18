/* -*- coding: utf-8 -*- */

/** 
 * 003-smartcurve.hpp
 *
 * VisuGL example.
 * Window + Figure with title
 *  + CurveDyn (as visualizer of model : std::vector<double>)
 *
 * A CurveDyn is associated to a model (assumed to be a Collection of double).
 * A Functor can modify the way model is visualized (ex: x 2)
 *
 * 'C' or 'c' -> change data (and update curves)
 */

#include <string>
#include <iostream>

#include <vector>
#include <random>

#include <window.hpp>
#include <figure.hpp>
#include <curve.hpp>

/** Window, Figure and Plotters */
Window* _win;
Figure* _fig;
CurveDyn<std::vector<double>>* _curve1;
CurveDyn<std::vector<double>>* _curve2;
/** Data to be plotted */
std::vector<double> _data;
/** Random generator */
// init random double generation
std::random_device _rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 _gen(_rd()); //Standard mersenne_twister_engine seeded with rd()
std::uniform_real_distribution<> _dis(-0.5, 0.5);

/** Graphic */
bool _end_render = false;


void change_data();
//******************************************************************************
/**
 * Callback for keyboard events
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
  // c/C : change data
  else if ( key == GLFW_KEY_C && action == GLFW_PRESS ) {
    change_data();
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

// *************************************************************** change_data
void change_data()
{
  // A model: Collection of double
  _data.clear();
  for( unsigned int i = 0; i < 100; ++i) {
    _data.push_back( 2.0 * cos( double(i) / 50.0 * 2.0 * M_PI) + _dis(_gen) );
  }

  // update curves
  _curve1->update();
  _curve2->update();
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
  // A CurveDyn
  std::cout << "__CURVE" << std::endl;
  // default is red thin line
  _curve1 = new CurveDyn<std::vector<double>>( _data );
  // offset and halfvalue thanks to Functor
  _curve2 = new CurveDyn<std::vector<double>>( _data,
                                               [] (std::vector<double>::const_iterator it) -> double {return (*it) * 0.5 + 0.5; } );
  _curve2->set_color( {0.0, 0.0, 1.0} ); // blue
  
  // populate _data
  change_data();
  
  // ****** GRAPHIC ********
  init_glfw();
  GLenum err;
  std::cout << "__ READ buffer start" << std::endl;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "INIT::OpenGL error: " << err << std::endl;
  }
  std::cout << "__WINDOW and FIGURE" << std::endl;
  _win = new Window( "Top Window", 600, 600 );
  _fig = new Figure( *_win, "Smart Curve    [ESC:quit, C/c: change data]" );
  _fig->_update_axes_x = true;
  _win->add_plotter( _fig );
  _fig->add_plotter( _curve1 );
  _fig->add_plotter( _curve2 );
  _win->update_bbox();
  
  std::cout << "__RENDER" << std::endl;
  update_and_render();

  std::cout << "__END" << std::endl;
  delete _curve1;
  delete _fig;
  delete _win;

  glfw_end();
  
  return 0;
}
