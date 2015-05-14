/* -*- coding: utf-8 -*- */

/** 
 * test-003-ftgl.cpp
 * Modified from https://www.opengl.org/archives/resources/features/fontsurvey/
 *
 * Essai afficher du texte avec FTGL et GLFW.
 */

#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <stdlib.h> // exit()

#include <FTGL/ftgl.h>

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
  void init( const char* font_filename )
  {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    _fonts[0] = new FTGLOutlineFont(font_filename);
    _fonts[1] = new FTGLPolygonFont(font_filename);
    _fonts[2] = new FTGLTextureFont(font_filename);
    _fonts[3] = new FTGLBitmapFont(font_filename);
    _fonts[4] = new FTGLPixmapFont(font_filename);
    for (int i=0; i< 5; i++) {
      if (!_fonts[i]) {
	std::cerr << "ERROR: Unable to open file " << font_filename << "\n";
      }
      else {
	int point_size = 24;
	if (!_fonts[i]->FaceSize(point_size)) {
	  std::cerr << "ERROR: Unable to set font face size " << point_size << "\n";
	}
      }
    }
  }
  void draw_scene()
  {
    /* Set up some strings with the characters to draw. */
    unsigned int count = 0;
    char string[8][256];
    int i;
    for (i=1; i < 32; i++) { /* Skip zero - it's the null terminator! */
      string[0][count] = i;
      count++;
    }
    string[0][count] = '\0';
    
    count = 0;
    for (i=32; i < 64; i++) {
      string[1][count] = i;
      count++;
    }
    string[1][count] = '\0';
    
    count = 0;
    for (i=64; i < 96; i++) {
      string[2][count] = i;
      count++;
    }
    string[2][count] = '\0';
    
    count = 0;
    for (i=96; i < 128; i++) {
      string[3][count] = i;
      count++;
    }
    string[3][count] = '\0';
    
    count = 0;
    for (i=128; i < 160; i++) {
      string[4][count] = i;
      count++;
    }
    string[4][count] = '\0';
    
    count = 0;
    for (i=160; i < 192; i++) {
      string[5][count] = i;
      count++;
    }
    string[5][count] = '\0';
    
    count = 0;
    for (i=192; i < 224; i++) {
      string[6][count] = i;
      count++;
    }
    string[6][count] = '\0';
    
    count = 0;
    for (i=224; i < 256; i++) {
      string[7][count] = i;
      count++;
    }
    string[7][count] = '\0';
    
    
    glColor3f(1.0, 1.0, 1.0);
    
    for (int font = 0; font < 5; font++) {
      GLfloat x = -250.0;
      GLfloat y;
      GLfloat yild = 20.0;
      for (int j=0; j<4; j++) {
	y = 275.0-font*120.0-j*yild;
	if (font >= 3) {
	  glRasterPos2f(x, y);
	  _fonts[font]->Render(string[j]);
	}
	else {
	  if (font == 2) {
	    glEnable(GL_TEXTURE_2D);
	    glEnable(GL_BLEND);
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	  }
	  glPushMatrix(); {
	    glTranslatef(x, y, 0.0);
	    _fonts[font]->Render(string[j]);
	  } glPopMatrix();
	  if (font == 2) {
	    glDisable(GL_TEXTURE_2D);
	    glDisable(GL_BLEND);
	  }
	}
      }
    }
  }

  void render() {
    while (!glfwWindowShouldClose( _window )) {
      //float ratio;
      int width, height;
      
      glfwGetFramebufferSize( _window, &width, &height);
      
      glViewport(0, 0, width, height);
      glClearColor( 0.0, 0.0, 0.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);

      // do_ortho();
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      
      GLdouble size = (GLdouble)((width >= height) ? width : height) / 2.0;
      GLdouble aspect = 1.0;
      if (width <= height) {
	aspect = (GLdouble)height/(GLdouble)width;
	glOrtho(-size, size, -size*aspect, size*aspect,
		-100000.0, 100000.0);
      }
      else {
	aspect = (GLdouble)width/(GLdouble)height;
	glOrtho(-size*aspect, size*aspect, -size, size,
		-100000.0, 100000.0);
      }
      // Make the world and window coordinates coincide so that 1.0 in
      // model space equals one pixel in window space.
      glScaled(aspect, aspect, 1.0);
      
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      // draw_scene
      draw_scene();
      
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
  /** Des Fontes */
  FTFont* _fonts[5];
  
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
  Window win("FTGL Demo", 600, 600);

  int result = 1;
  if ( argc != 2 ) {
    std::cerr << "usage: " << argv[0] << " font_filename.ttf" << std::endl;
    result = 0;
  }
  else {
       win.init(argv[1]);

       win.render();
   }
  exit(result);
}




