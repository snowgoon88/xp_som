/* -*- coding: utf-8 -*- */

/** 
 * test-020-anttweak.cpp
 *
 * Figure + AnttweakBar ?
 */

#include <figure.hpp>
#include <curve.hpp>

#include <AntTweakBar.h>

#include <GLFW/glfw3.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <memory>

/** Figure and curves */
Figure* _fig;
Curve* _c_sin;
float curve_color[] = { 0.7f, 0.5f, 0.6f };

/** Graphic */
TwBar* _bar;
bool _end_render = false;

// ***************************************************************************
// ********************************************************* Graphic Functions
// ***************************************************************************
/**
 * Init and end GLFW
*/
void init_glfw()
{
  std::cout << "__GLFW Init" << std::endl;
  
  glfwSetErrorCallback(error_callback);
  
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
  while( not _end_render and !glfwWindowShouldClose(_fig->_window) ) {
    _c_sin->set_color( {curve_color[0], curve_color[1], curve_color[2]} );

    // Altered Figure::render() to include AntTweakBar
    _fig->render( true, true ); /* update x_axis, y_axis */

    // Draw tweak bars
    TwDraw();

    glfwSwapBuffers( _fig->_window );
    glfwPollEvents();
    
  }
}
// ***************************************************************************
/**
 * Callback for GLFW errors
 */
static void error_callback(int error, const char* description)
{
  std::cerr <<  description << std::endl;
}
/**
 * Callback for GLFW key events
 */
static void key_callback(GLFWwindow* window, int key, int scancode,
                         int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    _end_render = true;
  }
}
// ***************************************************************************

// ***************************************************************************
// **************************************************************** TwCallback
void TwEventMouseButtonGLFW3( GLFWwindow *win, /*not used*/
                                     int glfwButton, int glfwAction,
                                     int mods /* not used */)
{
  TwEventMouseButtonGLFW( glfwButton, glfwAction );
}
int TW_CALL TwEventMousePosGLFW3( GLFWwindow *win, /*not used*/
                                  double xpos, double ypos )
{
  std::cout << "++Pos x=" << xpos << " y=" << ypos << std::endl;
  return TwEventMousePosGLFW( (int) (xpos+0.5), (int) (ypos+0.5) );
}
int TW_CALL TwEventMouseWheelGLFW3( GLFWwindow *win, /*not used*/
                                    double xoffset, double yoffset )
{
  std::cout << "++Wheel x=" << xoffset << " y=" << yoffset << std::endl;
  return TwEventMouseWheelGLFW( (int) (yoffset+0.5) );
}
int TW_CALL TwEventKeyGLFW3( GLFWwindow *win, /*not used*/
                             int glfwKey, int glfwScanCode,
                             int glfwAction, int glfwMods)
{
  /* TODO rewrite using glfwMods, etc... [see TwEventGLFW.c] */
  return TwEventKeyGLFW( glfwKey, glfwAction);
}
int TW_CALL TwEventCharGLFW3( GLFWwindow *win, /*not used*/
                              int glfwChar )
{
  return TwEventCharGLFW( glfwChar, GLFW_PRESS );
}
// ***************************************************************************
// ********************************************************************** Main
// ***************************************************************************
int main(int argc, char *argv[])
{
  init_glfw();

  // Create Figure (thus Window)
  std::cout << "__FIGURE" << std::endl;
  _fig = new Figure( "Figure and AntTweakBar" );
  
  std::cout << "__AntTWEAK" << std::endl;
  // Initialize AntTweakBar
  TwInit(TW_OPENGL, NULL);
  TwWindowSize( _fig->_width, _fig->_height);

  
  // Create AntTweakBar
  _bar = TwNewBar( "TweakBar" );
  TwDefine(" GLOBAL help='Example to integrate AntTweakBar with Figure.' ");

  // curve_color
  // TwAddVarRW( _bar, "curve_color", TW_TYPE_COLOR3F, &curve_color, 
  //              " label='Curve color' help='Color of the curve.' ");

  
  std::cout << "__SET Callback" << std::endl;
  // - Directly redirect GLFW mouse button events to AntTweakBar
  glfwSetMouseButtonCallback( _fig->_window,
                              TwEventMouseButtonGLFW3);
  std::cout << "  MouseButton" << std::endl;
  // // - Directly redirect GLFW mouse position events to AntTweakBar
  // glfwSetCursorPosCallback( _fig->_window,
  //                           (GLFWcursorposfun)TwEventMousePosGLFW3);
  // std::cout << "  CursorPos" << std::endl;
  // // - Directly redirect GLFW mouse wheel events to AntTweakBar
  // glfwSetScrollCallback( _fig->_window,
  //                        (GLFWscrollfun)TwEventMouseWheelGLFW3);
  // std::cout << "  Scroll" << std::endl;
  // // - Directly redirect GLFW key events to AntTweakBar
  // glfwSetKeyCallback( _fig->_window, (GLFWkeyfun)TwEventKeyGLFW3);
  // std::cout << "  Key" << std::endl;
  // // - Directly redirect GLFW char events to AntTweakBar
  // glfwSetCharCallback( _fig->_window, (GLFWcharfun)TwEventCharGLFW3);
  // std::cout << "  EventChar" << std::endl;
  
  
  // Curves
  std::cout << "__Curves" << std::endl;
  _c_sin = new Curve();
  _c_sin->create_data();
  _fig->add_curve( _c_sin );

  std::cout << "__RENDER" << std::endl;
  render();

  std::cout << "__END" << std::endl;
  delete _c_sin;
  delete _fig;

  TwDeleteBar( _bar );
  TwTerminate();
  glfw_end();

  
  return 0;
}
// *********************************************************************** END
