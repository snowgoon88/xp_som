/* -*- coding: utf-8 -*- */

/** 
 * test-007-viewer.cpp
 *
 * Display various internal values of RecDSOM
 *  - weights
 */

#include <GLFW/glfw3.h>
#include <string>
//#include <stdlib.h>
#include <iostream>
//#include <memory>

#include <curve.hpp>
#include <axis.hpp>

#include <dsom/r_network.hpp>
using namespace Model::DSOM;
// ******************************************************************** Window
/**
 *
 */
class Window
{
public:
  /**  
   * Create with title and size
   */
  Window(const std::string& title = "RecDSOM", int width=640, int height=400) :
    _axis_x(), _axis_y(), _c_weights() // "X", {-1.0, .0, 8, 10} ), _axis_y( "Y", {-1.0, 1.0, 4, 10} )
  {
    std::cout << "Window creation" << std::endl;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);
    
    _window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (! _window ) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent( _window );
	glfwSetWindowUserPointer( _window, this);
    glfwSetKeyCallback( _window, key_callback);

	// are Wide line supported ??
	GLfloat lineWidthRange[2] = {0.0f, 0.0f};
	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
	// Maximum supported line width is in lineWidthRange[1].
	std::cout << "glLineWidth in " << lineWidthRange[0] << " / " << lineWidthRange[1] << std::endl;
	init();
  }

  // ************************************************************ Window::init
  void init()
  {
	// Recurrent DSOM
	_rdsom = new RNetwork( 1, 100, -1 );
	// one input
	Eigen::VectorXd i1(1);
	i1 << 0.33;;
	_rdsom->computeWinner( i1, 0.5, 0.1, 0.1, 0.05 );
	std::cout << "max=" << _rdsom->_winner_similarity << " at " << _rdsom->_winner_neur << std::endl;

	// axis
	_axis_x = Axis{ "X", {0.0, (double)_rdsom->v_neur.size(), 10, 2} };
	_axis_y = Axis{ "Y", {0.0, 1.0, 5, 2} };

	// weights in black
	_c_weights.set_color( {0.0, 0.0, 0.0} );
	_c_weights.set_width( 2.f );
	for( unsigned int i = 0; i < _rdsom->v_neur.size(); ++i) {
	  _c_weights.add_sample( {(double)i, _rdsom->v_neur[i]->weights(0), 0.0} ); 
	}
	// similarity input in red, rec in green, merged in blue
	_c_sim_input.set_color( {1.0, 0.0, 0.0} );
	_c_sim_input.set_color( {0.0, 1.0, 0.0} );
	_c_sim_merged.set_color( {0.0, 0.0, 1.0} );
	_c_sim_convol.set_color( {0.0, 0.0, 1.0} );
	_c_sim_convol.set_width( 2.f );
	
	for( unsigned int i = 0; i < _rdsom->v_neur.size(); ++i) {
	  _c_sim_input.add_sample( {(double)i, _rdsom->_sim_w[i], 0.0} );
	  _c_sim_rec.add_sample( {(double)i, _rdsom->_sim_rec[i], 0.0} );
	  _c_sim_merged.add_sample( {(double)i, _rdsom->_sim_merged[i], 0.0} );
	  _c_sim_convol.add_sample( {(double)i, _rdsom->_sim_convol[i], 0.0} );
	}
  }
  
  void render() {
    while (!glfwWindowShouldClose( _window )) {
      //float ratio;
      int width, height;
      
      glfwGetFramebufferSize( _window, &width, &height);
      //ratio = width / (float) height;
      
      glViewport(0, 0, width, height);
      glClearColor( 1.0, 1.0, 1.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);

	  // Update according to _axis_x
      double x_min_win = _axis_x.get_range()._min - 0.05 * (_axis_x.get_range()._max - _axis_x.get_range()._min);
      double x_max_win = _axis_x.get_range()._max + 0.05 * (_axis_x.get_range()._max - _axis_x.get_range()._min);
	  double y_min_win = _axis_y.get_range()._min - 0.05 * (_axis_y.get_range()._max - _axis_y.get_range()._min);
      double y_max_win = _axis_y.get_range()._max + 0.05 * (_axis_y.get_range()._max - _axis_y.get_range()._min);
	  double ratio_x = (x_max_win-x_min_win) / (double) width;
	  double ratio_y = (y_max_win-y_min_win) / (double) height;
	  
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      //glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      glOrtho( x_min_win, x_max_win, y_min_win, y_max_win, 1.f, -1.f);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      // Basic axes
      _axis_x.render( ratio_x, ratio_y );
      glPushMatrix(); // AXE_Y
      glRotated( 90.0, 0.0, 0.0, 1.0 );
      _axis_y.render( ratio_y, ratio_x );
      glPopMatrix(); // AXE_Y
      // All other objects
      _c_weights.render();
	  _c_sim_input.render();
	  _c_sim_rec.render();
      _c_sim_merged.render();
	  _c_sim_convol.render();
	  
      glfwSwapBuffers( _window );
      glfwPollEvents();
    }
  }
  
  ~Window() {  
    glfwDestroyWindow( _window);
    
    //glfwTerminate();
    std::cout << "Window destroyed" << std::endl;
  }
private:
  /** Pointer on Window */
  GLFWwindow* _window;
  /** X and Y axes*/
  Axis _axis_x, _axis_y;
  /** Curves */
  Curve _c_weights;
  Curve _c_sim_input;
  Curve _c_sim_rec;
  Curve _c_sim_merged;
  Curve _c_sim_convol;
  /** RecDSOM */
  RNetwork* _rdsom;
  //******************************************************************************
  /**
   * Callback for key events
   */
  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GL_TRUE);
	}
	// Beware, QWERTY keyboard
	else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
	  ((Window *)glfwGetWindowUserPointer(window))->recompute_axis_y( key );
	} 
  }
  // ************************************************ Window::recompute_axis_y
  void recompute_axis_y( int key )
  {
	// Re-compute Y-Axis
	_axis_y = Axis{ "Y", {0.0, 1.0, 5, 2} };
	  
	for( auto& pt: _c_sim_convol.get_samples()) {
	  _axis_y.get_range().update( pt.y );
	}

	std::cout << "New AxisY: " << _axis_y.get_range()._min << ", " << _axis_y.get_range()._max << std::endl;
  }
  // ***************************************************************************
  /**
   * Callback pour GLFW error messages
   */
  static void error_callback(int error, const char* description)
  {
    std::cerr <<  description << std::endl;
    //fputs(description, stderr);
  }
};

//******************************************************************************
int main( int argc, char *argv[] )
{
  Window win("RecDSOM", 600, 600);
  win.render();
  return 0;
}
