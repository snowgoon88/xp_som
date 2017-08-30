/* -*- coding: utf-8 -*- */

#ifndef FIGURE_HPP
#define FIGURE_HPP

/** 
 * A Window with a Figure inside to plot several curves.
 * Has its own font.
 * Can be rendered and saved OFFSCREEN.
 */

// include OpenGL > 1.1
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <iostream>                  // std::cout
#include <string>                    // std::string

//#include <GL/gl.h>                   // OpenGL
#include <FTGL/ftgl.h>               // Fonts in OpenGL
#define FONT_PATH "ressources/Consolas.ttf"
#define FONT_SIZE 12
#define DIM_MAJOR 6
// Default scale for fonts : screen width=800, axe from -1 to 1.
#define FONT_SCALE ((1.0 - -1.0) / 800.0)

#include <curve.hpp>
#include <axis.hpp>
#include <gl_utils.hpp>              // utils::gl::to_png

#include <list>
#include <memory>

static void error_callback(int error, const char* description );
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// ***************************************************************************
// ******************************************************************** Figure
// ***************************************************************************
class Figure
{
public:
  // *************************************************************************
  // ***************************************************** Figure::Graphictext
  // *************************************************************************
  using GraphicText = struct {
    double x, y;
    const std::string msg;
  };
public:
  // *********************************************************** Figure::Types
  using CurvePtr = Curve* ;
  using CurveList = std::list<CurvePtr>;
public:
  // ******************************************************** Figure::creation
  Figure( std::string title = "Figure",
	  const int width=640, const int height=400,
	  const bool offscreen=false,
	  const int posx=-1, const int posy = -1,
	  const Range& x_range = {-1.0, 1.0, 10, 2},
	  const Range& y_range = {-1.0, 1.0, 10, 2} ) :
    _title( title ), _width(width), _height(height),
    _offscreen(offscreen),
    _window(nullptr), _curves(),
    _draw_axes( true ),
    _axis_x( "X", x_range),
    _axis_y( "Y", y_range),
    _text_list()
  {
	// Create window _________________________________________________
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if( _offscreen) {
      glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }
    _window = glfwCreateWindow(_width, _height, _title.c_str(), NULL, NULL);
    if (! _window ) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
    glfwSetWindowPos( _window, posx, posy );
    glfwMakeContextCurrent( _window );
    // TODO can also be set to another DataStructure
    glfwSetWindowUserPointer( _window, this);
    glfwSetKeyCallback( _window, key_callback);

    /** Init Fonts */
    _font = new FTGLTextureFont( FONT_PATH );
    if (! _font) {
      std::cerr << "ERROR: Unable to open file " << FONT_PATH << std::endl;
    }
    else {
      if (!_font->FaceSize(FONT_SIZE)) {
	std::cerr << "ERROR: Unable to set font face size " << FONT_SIZE << std::endl;
      }
    }

    /** offscreen => need RenderBuffer in FrameBufferObject */
    if( _offscreen ) {
      GLenum error = glewInit();
      if (error != GLEW_OK) {
	std::cout << "error with glew init() : " << glewGetErrorString(error) << std::endl;
      } else {
        std::cout << "glew is ok\n\n";
      }
      // std::cout << "__CREATE RenderBuffer" << std::endl;
      glGenRenderbuffers( 1 /* nb buffer */, &_render_buf);
      utils::gl::check_error();
      glBindRenderbuffer( GL_RENDERBUFFER, _render_buf );
      utils::gl::check_error();
      glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB32F, width, height);
      utils::gl::check_error();
      glBindRenderbuffer( GL_RENDERBUFFER, 0 );
      utils::gl::check_error();

      // std::cout << "__CREATE FrameBufferObject"  << std::endl;
      glGenFramebuffers(1 /* nb objects*/, &_fbo);
      utils::gl::check_error();
      glBindFramebuffer( GL_FRAMEBUFFER, _fbo );
      utils::gl::check_error();
      glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, /* attach point */
				 GL_RENDERBUFFER, _render_buf );
      utils::gl::check_error();
      
      // switch back to window-system-provided framebuffer
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      utils::gl::check_error();
    }
  }
  // ***************************************************** Figure::destruction
  ~Figure()
  {
    glfwSetWindowShouldClose(_window, GL_TRUE);
    if(_window)
      glfwDestroyWindow( _window);
    if( _offscreen ) {
      glDeleteRenderbuffers( 1, &_render_buf );
      utils::gl::check_error();
      glDeleteFramebuffers( 1, &_fbo );
      utils::gl::check_error();
    }
  }
  // ******************************************************* Figure::add_curve
  CurvePtr add_curve( CurvePtr curve )
  {
	_curves.push_back( curve );

	return curve;
  }
  // *************************************************** Figure::set_draw_axes
  void set_draw_axes( bool draw_axes )
  {
    _draw_axes = draw_axes;
  }
  // ***************************************************** Figure::GraphicText
  void clear_text()
  {
    _text_list.clear();
  }
  void add_text( const std::string msg, double x=0.0, double y=0.0 )
  {
    _text_list.push_back( GraphicText{ x, y, msg } );
  }
  // ************************************************************ Figure::save
  void save( const std::string& filename )
  {
    // Make sure using current window
    glfwMakeContextCurrent( _window );
    // TODO can also be set to another DataStructure
    glfwSetWindowUserPointer( _window, this);

    if( _offscreen ) {
      // set rendering destination to FBO
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
      utils::gl::check_error();
    }
    utils::gl::to_png( filename );

  }
  // ********************************************************** Figure::render
  void render( bool update_axes_x=false, bool update_axes_y=false )
  {
    glfwMakeContextCurrent( _window );
    // TODO can also be set to another DataStructure
    glfwSetWindowUserPointer( _window, this);

    if( _offscreen ) {
      // set rendering destination to FBO
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
      utils::gl::check_error();
    }
    
    if( update_axes_x || update_axes_y ) {
      // Build proper axis by finding min/max on each axe
      Curve::BoundingBox bbox{ std::numeric_limits<double>::max(),
	  (-std::numeric_limits<double>::max()),
	  std::numeric_limits<double>::max(),
	  -std::numeric_limits<double>::max() };
      
      for( const auto& curve: _curves ) {
	auto b = curve->get_bbox();
	if( b.x_min < bbox.x_min ) bbox.x_min = b.x_min;
	if( b.x_max > bbox.x_max ) bbox.x_max = b.x_max;
	if( b.y_min < bbox.y_min ) bbox.y_min = b.y_min;
	if( b.y_max > bbox.y_max ) bbox.y_max = b.y_max;
      }
      if( update_axes_x) 
	_axis_x = Axis( "X", {bbox.x_min,bbox.x_max, 10, 2});
      if( update_axes_y )
	_axis_y = Axis( "Y", {bbox.y_min,bbox.y_max, 10, 2});
    }
    
    // get window size
    if( _offscreen ) {
      glBindRenderbuffer( GL_RENDERBUFFER, _render_buf );
      utils::gl::check_error();
      glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_width);
      glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_height);
      utils::gl::check_error();
      glBindRenderbuffer( GL_RENDERBUFFER, 0 );
      utils::gl::check_error();
    }
    else {
      glfwGetFramebufferSize( _window, &_width, &_height);
    }
    // Info for scaling View and axes
    auto x_min_win = _axis_x.get_range()._min
      - 0.08 * (_axis_x.get_range()._max - _axis_x.get_range()._min);
    auto x_max_win = _axis_x.get_range()._max
      + 0.08 * (_axis_x.get_range()._max - _axis_x.get_range()._min);
    auto ratio_x = (x_max_win-x_min_win) / (double) _width;
    auto y_min_win = _axis_y.get_range()._min
      - 0.08 * (_axis_y.get_range()._max - _axis_y.get_range()._min);
    auto y_max_win = _axis_y.get_range()._max
      + 0.08 * (_axis_y.get_range()._max - _axis_y.get_range()._min);
    auto ratio_y = (y_max_win-y_min_win) / (double) _height;

	glViewport(0, 0, _width, _height);
	glClearColor( 1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho( x_min_win, x_max_win, y_min_win, y_max_win, 1.f, -1.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Basic axes
	if( _draw_axes ) {
	  _axis_x.render( ratio_x, ratio_y );
	  glPushMatrix(); // AXE_Y
	  glRotated( 90.0, 0.0, 0.0, 1.0 );
	  _axis_y.render( ratio_y, ratio_x);
	  glPopMatrix(); // AXE_Y
	}
	// All other objects
	for( const auto& curve: _curves) {
	  curve->render();
	}

	// GraphicText
	for( auto& txt: _text_list) {
	  glPushMatrix(); {
	    glTranslated( txt.x, txt.y, 0.0);
	    glScaled( ratio_x, ratio_y, 1.0 );
	    _font->Render( txt.msg.c_str() );
	  } glPopMatrix();
	}

	if( not _offscreen ) {
	  glfwSwapBuffers( _window );
	  glfwPollEvents();
	}
  }

public:
  // ******************************************************* Figure::attributs
  std::string _title;
  int _width, _height;
  bool _offscreen;
  GLFWwindow* _window;
  /** All the curves */
  CurveList _curves;
  /** X and Y axes*/
  bool _draw_axes;
  Axis _axis_x, _axis_y;
  /** List of Graphictext */
  std::list<GraphicText> _text_list;
  /** Fonts to write text */
  /*static*/ FTFont* _font;
  /** GLew variables for FrameBufferObject, RenderBuffer */
  GLuint _fbo, _render_buf;
};


#endif // FIGURE_HPP
