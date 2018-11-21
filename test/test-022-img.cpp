/* -*- coding: utf-8 -*- */

/** 
 * test-022-img.cpp
 *
 * Display data as images in Figure
 */

#include <GLFW/glfw3.h>
#include <string>
#include <stdlib.h>
#include <iostream>

#include <img_plotter.hpp>
#include <math.h>

ImgPlotter<std::vector<double>>* _img_plotter;

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
  Window(const std::string& title = "GLFW Window", int width=640, int height=400)
  {
    std::cout << "Window creation" << std::endl;
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);
    
    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!window) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window)) {
      float ratio;
      int width, height;
      
      glfwGetFramebufferSize(window, &width, &height);
      ratio = width / (float) height;
      
      glViewport(0, 0, width, height);
      glClear(GL_COLOR_BUFFER_BIT);
      
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      glMatrixMode(GL_MODELVIEW);
      
      _img_plotter->render();
      
      glfwSwapBuffers(window);
      glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    
    glfwTerminate();
    //exit(EXIT_SUCCESS);
  }
private:
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
  // Create some data
  std::vector<double> data;
  for( unsigned int iy = 0; iy < 100; ++iy) {
    double y = -1.0 + double(iy)/(99.0) * 2.0;
    for( unsigned int ix = 0; ix < 200; ++ix) {
      double x = -2.0 + double(ix) / (199.0) * 4.0;

      data.push_back( sin( 10 * (x*x + y*y)) );
    }
  }
  
  _img_plotter = new ImgPlotter<std::vector<double>>( data, 200, 100 );
  Window win("ImgPlotter", 600, 600);
  
  return 0;
}
