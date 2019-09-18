/* -*- coding: utf-8 -*- */

/** 
 * test-002-curve-save.cpp
 *
 * Plot then save
 */

#include <GLFW/glfw3.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <memory>

#include <curve.hpp>
#include <gl_utils.hpp>

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
    // create_data 
    const unsigned int _nb_data = 100;
    for( unsigned int i=0; i < _nb_data; ++i) {
      Curve::Sample pt;
      pt.x = 2.0 * M_PI * i / _nb_data;
      pt.y = sin( pt.x );
      pt.z = 0.0;      
      _curve.add_sample( pt );
    }
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
      glOrtho( -1.f, 8.f, -2.f, 2.f, 1.f, -1.f);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      // Basic axes
      // Couleur bleue
      glColor4d( 0.0, 0.0, 1.0, 1.0);
      // -------------------------------------------------------------------------
      //  Rendering using GL_LINE_STRIP
      // -------------------------------------------------------------------------
      glEnable (GL_BLEND);
      glEnable (GL_LINE_SMOOTH);
      glLineWidth (2.0);
      glBegin(GL_LINES); {
	glVertex3d( -1.0, 0.0, 0.0 );
	glVertex3d(  1.0, 0.0, 0.0 );
	glVertex3d( 0.0, -1.0, 0.0 );
	glVertex3d( 0.0,  1.0, 0.0 );
      }
      glEnd();

      // All other objects
      _curve.render();
      // -----------------

      glBegin(GL_TRIANGLES);
      glColor3f(1.f, 0.f, 0.f);
      glVertex3f(-0.6f, -0.4f, 0.f);
      glColor3f(0.f, 1.f, 0.f);
      glVertex3f(0.6f, -0.4f, 0.f);
      glColor3f(0.f, 0.f, 1.f);
      glVertex3f(0.f, 0.6f, 0.f);
      glEnd();
      
      glfwSwapBuffers( _window );
      glfwPollEvents();
    }

    // try to save to PNG
    utils::gl::to_png( "imag.png" );
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
  Window win("Une Courbe", 600, 400);
  win.render();
  return 0;
}
