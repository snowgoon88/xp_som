/* -*- coding: utf-8 -*- */

/** 
 * test-005-dyncurve.cpp
 *
 * Fenetre avec Courbe Dynamique et Axes.
 */

#include <GLFW/glfw3.h>
#include <string>
#include <stdlib.h>
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <chrono>         //std::chrono
#include <memory>

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
    _axis_x( "X", {-1.0, 7.0, 8, 10} ), _axis_y( "Y", {-1.0, 1.0, 4, 10} )
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
    unsigned int nb_data = 1000;
    while (!glfwWindowShouldClose( _window )) {
      //float ratio;
      int width, height;
      
      glfwGetFramebufferSize( _window, &width, &height);
      //ratio = width / (float) height;
      
      glViewport(0, 0, width, height);
      glClearColor( 1.0, 1.0, 1.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);
      
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      //glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      glOrtho( -2.f, 8.f, -2.f, 2.f, 1.f, -1.f);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      // Basic axes
      _axis_x.render();
      glPushMatrix(); // AXE_Y
      glRotated( 90.0, 0.0, 0.0, 1.0 );
      _axis_y.render();
      glPopMatrix(); // AXE_Y

      // Add point to curve
      Curve::Sample pt;
      pt.x = 2.0 * M_PI * i / nb_data;
      pt.y = sin( pt.x );
      pt.z = 0.0;
      ++i;
      _curve.add_sample( pt );
      //std::cout << "Curve  size(data)=" << _data.size() << std::endl;
      std::cout << "bbox = {" << _curve.get_bbox().x_min <<"; " << _curve.get_bbox().x_max << "; ";
      std::cout << _curve.get_bbox().y_min << "; " << _curve.get_bbox().y_max << "}" << std::endl;
      // All other objects
      _curve.render();
      
      glfwSwapBuffers( _window );
      glfwPollEvents();
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      
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
