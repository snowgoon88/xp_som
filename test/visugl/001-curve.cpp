/* -*- coding: utf-8 -*- */

/** 
 * 001-curve.cpp
 *
 * Full VisuGL example.
 * Window + Figure with title
 *   + Curve1 as example
 *   + Curve2 as a copy of Curve1 with added Samples
 *   + Curve3 fed with Collections of double
 *   + Curve4 fed with "time serie" (Collection of Y)
 *   + text to label the Curves
 * 
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
Curve*  _curve1;
Curve*  _curve2;
Curve*  _curve3;
Curve*  _curve4;

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
  else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    // save window
    _win->save( "001-curve.png" );
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
  // A static Curve
  std::cout << "__CURVE" << std::endl;
  _curve1 = new Curve(); // default is red thin line
  _curve1->clear();
  
  const unsigned int _nb_data = 100;
  for( unsigned int i=0; i < _nb_data; ++i) {
    Curve::Sample pt;
      pt.x = 2.0 * M_PI * i / _nb_data;
      pt.y = sin( pt.x );
      pt.z = 0.0;      
      _curve1->add_sample( pt );
  }

  // Copy to which we change points, and "go back"
  _curve2 = new Curve( *_curve1 );
  _curve2->set_color( {0.0, 0.1, 1.0} ); // blue line
  _curve2->set_width( 3.f );             // thicker
  // inverse points
  // get_samples (a COPY) , inverse, and copy back to curve2
  auto samples2 = _curve2->get_samples(); // a COPY
  _curve2->clear();                       // can clear without fear
  for( auto& pt: samples2) {
    pt.y = -pt.y;
    _curve2->add_sample( pt );
  }
  // then add some "weird" points
  _curve2->add_sample( {3.14, 0.4, 0.0} ); // NOT working (because inside bbox)
  _curve2->add_data( {3.14, -0.4, 0.0} );  // working (even inside bbox)

  // Set up data using Collections
  std::vector<double> cx;
  std::vector<double> cy;
  for( unsigned int idc = 0; idc < 16; ++idc) {
    auto angle = double(idc) / 15.0 * 2.0 * M_PI;
    cx.push_back( 2.0 + 0.4 * angle );
    cy.push_back( 0.2 + 0.4 * sin( angle ));
  }
  _curve3 = new Curve();
  _curve3->set_color( {0.0, 1.0, 0.0} );
  _curve3->add_sample( cx.begin(), cx.end(), cy.begin(), cy.end() );

  // Can also create "time serie" where implicitely, x is in range[0, size(y)]
  std::vector<double> ty;
  for( unsigned int i = 0; i < 7; ++i) {
    ty.push_back( -0.5 + 0.1 * double(i));
  }
  _curve4 = new Curve();
  _curve4->set_color( {0.0, 1.0, 1.0} );
  _curve4->add_time_serie( ty.begin(), ty.end() );
  
  // ****** GRAPHIC ********
  init_glfw();
  std::cout << "__WINDOW and FIGURE" << std::endl;
  _win = new Window( "Top Window", 600, 600 );
  _fig = new Figure( *_win, "Several Curves          [ESC:quit, S:save as 001-curve.png]" );
  _win->add_plotter( _fig );
  _fig->add_plotter( _curve1 );
  _fig->add_text("curve1", 2.0, 0.95, {1.0,0.0,0.0});
  _fig->add_plotter( _curve2 );
  _fig->add_text("curve2", 5.2, 0.95, {0.0,0.0,1.0});
  _fig->add_plotter( _curve3 );
  _fig->add_text("curve3", 4.0, 0.2, {0.0,1.0,0.0});
  _fig->add_plotter( _curve4 );
  _fig->add_text("curve4", 1.0, -0.35, {0.0,1.0,1.0});
  
  _win->update_bbox();
  
  std::cout << "__RENDER" << std::endl;
  render();

  std::cout << "__END" << std::endl;
  delete _curve1;
  delete _curve2;
  delete _curve3;
  delete _curve4;
  delete _fig;
  delete _win;

  glfw_end();
  
  return 0;
}
