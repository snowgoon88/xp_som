/* -*- coding: utf-8 -*- */

/** 
 * test-002-smartcurve.cpp
 *
 * Try building a curve from
 *  - any Container
 */
#include <GLFW/glfw3.h>
#include <string>
#include <stdlib.h>
#include <iostream>
//#include <memory>

#include <curve.hpp>
#include <axis.hpp>
#include <vector>

// ******************************************************************** Window
/**
 *
 */
class Window
{
public:
  /**  
   * Création avec titre et taille de fenetre.
   */
  Window(const std::string& title = "GLFW Window", int width=640, int height=400) :
	_axis_x( "X", {-1.0, 100, 8, 10} ), _axis_y( "Y", {-1.0, 1.0, 4, 10} )
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
    glfwSetKeyCallback( _window, key_callback);

	for( unsigned int i=0; i < 100; ++i) {
      _data1.push_back( cos(2.0 * M_PI * (double) i / 40.0) );
    }

	_c1 = new CurveDyn<std::vector<double>>( _data1 );
	_c2 = new CurveDyn<std::vector<double>>( _data1, [] (std::vector<double>::const_iterator it ) -> double {return (*it)*0.8;} );
	_c2->set_color( {0.0, 1.0, 0.0} );
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
	  
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      //glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      glOrtho( x_min_win, x_max_win, -2.f, 2.f, 1.f, -1.f);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      // Basic axes
	  _axis_x.render();
      glPushMatrix(); // AXE_Y
      glRotated( 90.0, 0.0, 0.0, 1.0 );
      _axis_y.render();
      glPopMatrix(); // AXE_Y

	  // All other objects
      _c1->render();
	  _c2->render();
      
      glfwSwapBuffers( _window );
      glfwPollEvents();
    }
  }
  
  ~Window() {  
    glfwDestroyWindow( _window);
    
    glfwTerminate();
    std::cout << "Window destroyed" << std::endl;
  }
private:
  /** Ptr sur la Fenetre */
  GLFWwindow* _window;
  /** Une Courbe */
  Curve* _c1;
  Curve* _c2;
   /** Des Axes */
  Axis _axis_x, _axis_y;
  /** data */
  std::vector<double> _data1;

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
};

//******************************************************************************
int main( int argc, char *argv[] )
{
  Window win("Une Courbe", 600, 600);
  win.render();
  return 0;
}
