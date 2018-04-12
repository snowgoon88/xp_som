/* -*- coding: utf-8 -*- */

/** 
 * test-002-offscreen-curve.cpp
 *
 * Make RenderBuffer, attach it to FrameBufferObject so as to be able to write (draw)
 * and read (save) it.
 */

// include OpenGL > 1.1
#include <GL/glew.h>


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
  Window(const std::string& title = "GLFW Window", int width=640, int height=400) :
    _window(nullptr)
  {
    _curve.create_data();
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
  /** Create with offscreen rebderbuffer */
  Window(int width=640, int height=400) :
    _window(nullptr)
  {
    _curve.create_data();

    // Create a GL context using GLFW
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
      exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_VISIBLE, false );
    _offscreen_context = glfwCreateWindow(width, height, "", NULL, NULL);
    // The context must be made current to GL
    glfwMakeContextCurrent( _offscreen_context );
    
    GLenum error = glewInit();
    if (error != GLEW_OK){
      std::cout << "error with glew init() : " << glewGetErrorString(error) << std::endl;
    }else{
        std::cout << "glew is ok\n\n";
    }
    std::cout << "__CREATE RenderBuffer" << std::endl;
    glGenRenderbuffers( 1 /* nb buffer */, &_render_buf);
    utils::gl::check_error();
    std::cout << "  bind" << std::endl;
    glBindRenderbuffer( GL_RENDERBUFFER, _render_buf );
    utils::gl::check_error();
    std::cout << "  storage" << std::endl;
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB32F, width, height);
    utils::gl::check_error();
    std::cout << "  unbind" << std::endl;
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    utils::gl::check_error();

    std::cout << "__CREATE FrameBufferObject"  << std::endl;
    glGenFramebuffers(1 /* nb objects*/, &_fbo);
    std::cout << "  check" << std::endl;
    utils::gl::check_error();
    std::cout << "  bind" << std::endl;
    glBindFramebuffer( GL_FRAMEBUFFER, _fbo );
    utils::gl::check_error();
    std::cout << "  attache Render color" << std::endl;
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, /* attach point */
			       GL_RENDERBUFFER, _render_buf );
    utils::gl::check_error();
    std::cout << "  complete ? ";
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      std::cout << "E Framebuffer incomplete" << std::endl;
    }
    else {
      std::cout << "Yes" << std::endl;
    }
    utils::gl::check_error();
    // switch back to window-system-provided framebuffer
    std::cout << "  unbind -> default windo provided frambuffer" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    utils::gl::check_error();
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

  void off_screen_render()
  {
    // set rendering destination to FBO
    std::cout << "__SET offscreen FBO " << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    utils::gl::check_error();
    
    //float ratio;
    int width, height;

    std::cout << "__GET size of Renderbuffer " << std::endl;
    glBindRenderbuffer( GL_RENDERBUFFER, _render_buf );
    utils::gl::check_error();
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    utils::gl::check_error();
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    utils::gl::check_error();
    
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
    
    // try to save to PNG
    utils::gl::to_png( "imag_offscreen.png" );
  }
  
  ~Window() {
    if( _window ) {
      glfwDestroyWindow( _window);
    
      glfwTerminate();
      std::cout << "Window destroyed" << std::endl;
    }
  }
private:
  /** Ptr sur la Fenetre */
  GLFWwindow* _window;
  GLFWwindow* _offscreen_context;
  /** Une Courbe */
  Curve _curve;
  /** Various GL variables */
  GLuint _fbo, _render_buf;
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
  //Window win("Une Courbe", 600, 400);
  Window win(600, 400);
  // win.render();
  win.off_screen_render();
  return 0;
}
