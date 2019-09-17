/* -*- coding: utf-8 -*- */

/** 
 * Show difference between Window and WindowStatic (Double or Single Buffer)
 * for displaying incremental graphic (no animation, noting to delete/erase).
 *
 * Random Walk in red.
 */

// ******************************************************************* GLOBALS
bool _static_window = true;
double _xpos = 0.0;
double _ypos = 0.0;
double _xspd, _yspd;
double _tsim = 0.0;
const double _delta_tsim = 0.01;

bool _end_render = false;

// ************************************************************ Random GLOBALS
#include <random>
// Random Generator
std::random_device _random_seeder;
std::default_random_engine _rnd_engine( _random_seeder() );
auto _unif = std::uniform_real_distribution<double>(0.0, 1.0);
auto _normal = std::normal_distribution<double>(0.0, sqrt(2.0));

// ************************************************************ Grafic GLOBALS
#include <window.hpp>
#include <window_static.hpp>
#include <figure.hpp>
#include <curve.hpp>

/** Window, Figure and Plotters */
Window *_win_traj;
WindowStatic *_winstat_traj;
Figure *_fig_traj;
Curve *_c_traj;

// ***************************************************************************
// ********************************************************* Graphic Functions
// ***************************************************************************
/**
 * Callback for keyboard events
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
}
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
  // init dynamics
  _xspd = _normal( _rnd_engine );
  _yspd = _normal( _rnd_engine );
  
  while( not _end_render ) {

    // update new speed
    // change speed dynamics ?
    if( _unif( _rnd_engine ) < 0.02 ) {
      _xspd = _normal( _rnd_engine );
      _yspd = _normal( _rnd_engine );
    }

    // update position and time
    _xpos += _xspd * _delta_tsim;
    _ypos += _yspd * _delta_tsim;
    _tsim += _delta_tsim;

    // rebound on world limits
    if (_xpos > 1.0) {
      _xpos = 1.0 - (_xpos - 1.0);
      _xspd = -_xspd;
    }
    if (_xpos < -1.0) {
      _xpos = -1.0 + (-1.0 - _xpos);
      _xspd = -_xspd;
    }
    if (_ypos > 1.0) {
      _ypos = 1.0 - (_ypos - 1.0);
      _yspd = -_yspd;
    }
    if (_ypos < -1.0) {
      _ypos = -1.0 + (-1.0 - _ypos);
      _yspd = -_yspd;
    }
    
    // update grafik
    _c_traj->add_data( {_xpos, _ypos, 0.0} );

    if (_static_window)
      _winstat_traj->render();
    else
      _win_traj->render();

    // test if windows ask for closing
    if (_static_window) {
      if (glfwWindowShouldClose(_winstat_traj->_window)) {
        _end_render = true;
      }
    }
    else {
      if (glfwWindowShouldClose(_win_traj->_window)) {
        _end_render = true;
      }
    }
  }
}

// ********************************************************************** MAIN
int main(int argc, char *argv[])
{
  // test for arguments
  if (argc != 2) {
    std::cout << "usage : " << argv[0] << " static | swap" << std::endl;
    exit(1);
  }
  // static - single buffer
  if ( strcmp( argv[1], "static" ) == 0 ) {
    _static_window = true;
  }
  // swap - double buffer
  else if ( strcmp( argv[1], "swap" ) == 0 ) {
    _static_window = false;
  }
  else {
    std::cout << "Unknown argument : " << argv[1] << std::endl;
    exit(1);
  }

  // init grafic
  std::cout << "__PLOTTERS" << std::endl;
  _c_traj = new Curve(); // default is red thin line
  
  init_glfw();
  std::cout << "__WINDOW and FIGURE" << std::endl;
  if (_static_window) {
    _winstat_traj = new WindowStatic( "STATIC Window", 600, 600, false, 10, 20 );
    _fig_traj = new Figure( *_winstat_traj, "[ESC:quit]",
                       {-1.0, 1.0, 10, 2}, {-1.0, 1.0, 10, 2} );
    _winstat_traj->add_plotter( _fig_traj );
  }
  else {
    _win_traj = new Window( "SWAPPING Window", 600, 600, false, 10, 20  );
    _fig_traj = new Figure( *_win_traj, "[ESC:quit]",
                       {-1.0, 1.0, 10, 2}, {-1.0, 1.0, 10, 2} );
    _win_traj->add_plotter( _fig_traj );
  }
  _fig_traj->add_plotter( _c_traj );

  std::cout << "__RENDER" << std::endl;
  render();
  
  std::cout << "__END" << std::endl;
  std::cout << "Added " <<  _c_traj->get_samples().size() << " points ";
  std::cout << " simulated time=" << _tsim << " s." << std::endl;
    
  delete _c_traj;
  delete _fig_traj;
  if (_static_window) 
    delete _winstat_traj;
  else
    delete _win_traj;

  glfw_end();

  
  return 0;
}

