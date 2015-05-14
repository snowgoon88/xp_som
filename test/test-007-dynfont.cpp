/* -*- coding: utf-8 -*- */

/**
 * test-006-dynaxe.cpp
 * 
 * Fenetre avec Courbe et Axe_X dynamiques.
 */

#include <GLFW/glfw3.h>
#include <string>
#include <stdlib.h>
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <chrono>         //std::chrono
#include <memory>
#include <algorithm>      // std::max

#include <curve.hpp>
#include <axis.hpp>


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
    _axis_x( "X", {0.0, 1.0, 2, 10} ), _axis_y( "Y", {-1.0, 1.0, 4, 10} )
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

    
  }

  void render() {
    unsigned int i = 0;
    unsigned int nb_data = 4;
    while (!glfwWindowShouldClose( _window )) {
      // Update
      // Add point to curve
      Curve::Sample pt;
      pt.x = 10.0 * M_PI * i / nb_data;
      pt.y = sin( pt.x ) * std::max( 1.0, i / (double)(nb_data*100) );
      pt.z = 0.0;
      ++i;
      _axis_x.get_range().update( pt.x );
      _axis_y.get_range().update( pt.y );
      _curve.add_sample( pt );
      //std::cout << "Curve  size(data)=" << _data.size() << std::endl;
      //std::cout << "bbox = {" << _curve.get_bbox().x_min <<"; " << _curve.get_bbox().x_max << "; ";
      //std::cout << _curve.get_bbox().y_min << "; " << _curve.get_bbox().y_max << "}" << std::endl;

      
      //float ratio;
      int width, height;
      
      glfwGetFramebufferSize( _window, &width, &height);
      //ratio = width / (float) height;
      
      glViewport(0, 0, width, height);
      glClearColor( 1.0, 1.0, 1.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);

      // Update Projection matrix (and thus displayed space size) according to _axis_x
      double x_min_win = _axis_x.get_range()._min - 0.08 * (_axis_x.get_range()._max - _axis_x.get_range()._min);
      double x_max_win = _axis_x.get_range()._max + 0.08 * (_axis_x.get_range()._max - _axis_x.get_range()._min);
      double ratio_x = (x_max_win-x_min_win) / (double) width;
      double y_min_win = _axis_y.get_range()._min - 0.08 * (_axis_y.get_range()._max - _axis_y.get_range()._min);
      double y_max_win = _axis_y.get_range()._max + 0.08 * (_axis_y.get_range()._max - _axis_y.get_range()._min);
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
      _axis_y.render( ratio_y, ratio_x);
      glPopMatrix(); // AXE_Y
      
      // All other objects
      _curve.render();
      
      glfwSwapBuffers( _window );
      glfwPollEvents();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      
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
  /** Des Axes */
  Axis _axis_x, _axis_y;
  /** Une Courbe */
  Curve _curve;
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
  Window win("Une Courbe et des Axes", 600, 600);
  win.render();
  return 0;
}

