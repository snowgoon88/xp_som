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
    _axis_x(), _axis_y(), _c_weights(), // "X", {-1.0, .0, 8, 10} ), _axis_y( "Y", {-1.0, 1.0, 4, 10} )
	_nb_iter(0)
  {
    std::cout << "Window creation" << std::endl;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);
    
    _window_input = glfwCreateWindow(width, height, "INPUT - Weights", NULL, NULL);
    _window_rec = glfwCreateWindow(width, height, "RPOS - Rec_Weights", NULL, NULL);   
    if( (! _window_input) or (! _window_rec) ) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent( _window_input );
    glfwSetWindowUserPointer( _window_input, this);
    glfwSetWindowUserPointer( _window_rec, this);
    glfwSetKeyCallback( _window_input, key_callback);
    glfwSetKeyCallback( _window_rec, key_callback);

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
	_rdsom = new RNetwork( 1, 500, -1 );
	std::cout << "__CREATION" << std::endl << _rdsom->str_dump() << std::endl;
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
	_c_sim_rec.set_color( {0.0, 1.0, 0.0} );
	_c_sim_merged.set_color( {0.0, 0.0, 1.0} );
	_c_sim_convol.set_color( {0.0, 0.0, 1.0} );
	_c_sim_convol.set_width( 2.f );
	// hn
	_c_sim_hn_dist.set_color( {1.0, 0.0, 0.0} );
	_c_sim_hn_dist.set_width( 2.f );
	_c_sim_hn_rec.set_color( {0.0, 1.0, 0.0} );
	_c_sim_hn_rec.set_width( 2.f );
	
	for( unsigned int i = 0; i < _rdsom->v_neur.size(); ++i) {
	  _c_sim_input.add_sample( {(double)i, _rdsom->_sim_w[i], 0.0} );
	  _c_sim_rec.add_sample( {(double)i, _rdsom->_sim_rec[i], 0.0} );
	  _c_sim_merged.add_sample( {(double)i, _rdsom->_sim_merged[i], 0.0} );
	  _c_sim_convol.add_sample( {(double)i, _rdsom->_sim_convol[i], 0.0} );
	  _c_sim_hn_dist.add_sample( {(double)i, _rdsom->_sim_hn_dist[i], 0.0} );
	  _c_sim_hn_rec.add_sample( {(double)i, _rdsom->_sim_hn_rec[i], 0.0} );
	}
  }
  
  void render() {
    while (!glfwWindowShouldClose( _window_input )) {
      //float ratio;
      int width, height;


      // INPUT
      glfwMakeContextCurrent( _window_input );
      glfwGetFramebufferSize( _window_input, &width, &height);
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
	  _c_sim_hn_dist.render();
	  _c_sim_hn_rec.render();
	  
      glfwSwapBuffers( _window_input );

      // REC
      glfwMakeContextCurrent( _window_rec );
      glfwGetFramebufferSize( _window_rec, &width, &height);
      //ratio = width / (float) height;
      
      glViewport(0, 0, width, height);
      glClearColor( 1.0, 1.0, 1.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);

      // 	  // Update according to _axis_x
      // double x_min_win = _axis_x.get_range()._min - 0.05 * (_axis_x.get_range()._max - _axis_x.get_range()._min);
      // double x_max_win = _axis_x.get_range()._max + 0.05 * (_axis_x.get_range()._max - _axis_x.get_range()._min);
      // 	  double y_min_win = _axis_y.get_range()._min - 0.05 * (_axis_y.get_range()._max - _axis_y.get_range()._min);
      // double y_max_win = _axis_y.get_range()._max + 0.05 * (_axis_y.get_range()._max - _axis_y.get_range()._min);
      // 	  double ratio_x = (x_max_win-x_min_win) / (double) width;
      // 	  double ratio_y = (y_max_win-y_min_win) / (double) height;
	  
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
	  _c_sim_hn_dist.render();
	  _c_sim_hn_rec.render();
	  
      glfwSwapBuffers( _window_rec );
      
      glfwPollEvents();
    }
  }
  
  ~Window() {  
    glfwDestroyWindow( _window_input);
    glfwDestroyWindow( _window_rec);
    
    //glfwTerminate();
    std::cout << "Window destroyed" << std::endl;
  }
private:
  /** Pointer on Window */
  GLFWwindow* _window_input;
  GLFWwindow* _window_rec;
  /** X and Y axes*/
  Axis _axis_x, _axis_y;
  /** Curves */
  Curve _c_weights;
  Curve _c_sim_input;
  Curve _c_sim_rec;
  Curve _c_sim_merged;
  Curve _c_sim_convol;
  Curve _c_sim_hn_dist, _c_sim_hn_rec;
  /** RecDSOM */
  RNetwork* _rdsom;
  /** nb_iterations */
  int _nb_iter;
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
    else if (key == GLFW_KEY_N && action == GLFW_PRESS) {
      ((Window *)glfwGetWindowUserPointer(window))->step_learn( key );
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
  void step_learn( int key )
  {
    // one input
    // Eigen::VectorXd i1 = Eigen::VectorXd::Random(1);
    // i1 = (i1.array() - -1.0) / (1.0 - -1.0);
	Eigen::VectorXd i1(1);
	i1 << (double) (_nb_iter % 3) / 4.0 + 0.1;
	
    _rdsom->forward( i1 );
    _rdsom->deltaW( i1, 0.1, 0.2, true );

    _c_weights.clear();
    _c_sim_input.clear();
    _c_sim_rec.clear();
    _c_sim_merged.clear();
    _c_sim_convol.clear();
	_c_sim_hn_dist.clear();
	_c_sim_hn_rec.clear();
    for( unsigned int i = 0; i < _rdsom->v_neur.size(); ++i) {
      _c_weights.add_sample( {(double)i, _rdsom->v_neur[i]->weights(0), 0.0} ); 
      _c_sim_input.add_sample( {(double)i, _rdsom->_sim_w[i], 0.0} );
      _c_sim_rec.add_sample( {(double)i, _rdsom->_sim_rec[i], 0.0} );
      _c_sim_merged.add_sample( {(double)i, _rdsom->_sim_merged[i], 0.0} );
      _c_sim_convol.add_sample( {(double)i, _rdsom->_sim_convol[i], 0.0} );
	  _c_sim_hn_dist.add_sample( {(double)i, _rdsom->_sim_hn_dist[i], 0.0} );
	  _c_sim_hn_rec.add_sample( {(double)i, _rdsom->_sim_hn_rec[i], 0.0} );
	}

	_nb_iter++;
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
