/* -*- coding: utf-8 -*- */

#ifndef WINDOW_HPP
#define WINDOW_HPP

/** 
 * A Window is Plotter.
 * Has its own font.
 * Can be rendered and saved OFFSCREEN.
 *
 * MUST have error_callback and key_callback defined as static somewhere !!
 */

// include OpenGL > 1.1
#include <GL/glew.h>

#include <GLFW/glfw3.h>

// Fonts
#include <FTGL/ftgl.h>               // Fonts in OpenGL
#define FONT_PATH "ressources/Consolas.ttf"
#define FONT_SIZE 12
// Default scale for fonts : screen width=800, axe from -1 to 1.
#define FONT_SCALE ((1.0 - -1.0) / 800.0)

#include <gl_utils.hpp>              // utils::gl::to_png, error
#include <algorithm>

#include <visugl.hpp>
#include <plotter.hpp>

// ************************************************* MUST BE DEFINED SOMEWHERE
static void key_callback(GLFWwindow* window,
                         int key, int scancode, int action, int mods);
// ***************************************************************************


// ***************************************************************************
// ******************************************************************** Window
// ***************************************************************************
class Window : public Plotter
{
public:
  // ******************************************************** Window::creation
  Window( std::string title = "Window",
          const int width=640, const int height=400,
          const bool offscreen=false,
          const int posx=-1, const int posy = -1 ) :
    Plotter( -1.0, 2.0, -1.0, 2.0 ), // default _bbox
    _title( title ), _width(width), _height(height),
    _offscreen(offscreen),
    _window(nullptr), _font(nullptr)
  {
    // Create window _________________________________________________
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if( _offscreen) {
      glfwWindowHint(GLFW_VISIBLE, false );
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
    glfwSetWindowSizeCallback( _window, resize_callback );
    
    /** Init Fonts */
    _font = new FTGLTextureFont( FONT_PATH );
    if (! _font) {
      std::cerr << "ERROR: Unable to create Font" << std::endl;
    }
    else if (_font->Error()) {
      std::cerr << "ERROR: Unable to open file " << FONT_PATH << std::endl;
    }
    else {
      if (! _font->FaceSize(FONT_SIZE)) {
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
  // ****************************************************** Window::destructor
  virtual ~Window()
  {
    if (_window) {
      glfwSetWindowShouldClose(_window, GL_TRUE);
      glfwDestroyWindow( _window);
      _window = nullptr;
    }
    if( _offscreen ) {
      glDeleteRenderbuffers( 1, &_render_buf );
      utils::gl::check_error();
      glDeleteFramebuffers( 1, &_fbo );
      utils::gl::check_error();
    }
  }

  // ********************************************************** Window::update
  virtual void update_bbox()
  {
    // update BBox to be slightly larger than inner_bboxes
    BoundingBox bbox{ std::numeric_limits<double>::max(),
        (-std::numeric_limits<double>::max()),
        std::numeric_limits<double>::max(),
        -std::numeric_limits<double>::max() };
    
    for( const auto& plotter: _plotters ) {
      plotter->update_bbox();
      auto b = plotter->get_bbox();
      if( b.x_min < bbox.x_min ) bbox.x_min = b.x_min;
      if( b.x_max > bbox.x_max ) bbox.x_max = std::max( b.x_max, b.x_min+0.1 );
      if( b.y_min < bbox.y_min ) bbox.y_min = b.y_min;
      if( b.y_max > bbox.y_max ) bbox.y_max = std::max( b.y_max, b.y_min+0.1 );
    }
    auto range_x = bbox.x_max - bbox.x_min;
    bbox.x_min = bbox.x_min - 0.05 * range_x;
    bbox.x_max = bbox.x_max + 0.05 * range_x;
    auto range_y = bbox.y_max - bbox.y_min;
    bbox.y_min = bbox.y_min - 0.05 * range_y;
    bbox.y_max = bbox.y_max + 0.05 * range_y;
    set_bbox( bbox );
    
    // std::cout << "__UPD BBOX WIN window  = " << get_bbox() << std::endl;
  }
  // ********************************************************** Window::render
  virtual void render( float screen_ratio_x = 1.0, float screen_ratio_y = 1.0 )
  {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "WIN RENDER OpenGL error: " << err << std::endl;
    }
    
    // Make sure using current window
    glfwMakeContextCurrent( _window );
    // TODO can also be set to another DataStructure
    glfwSetWindowUserPointer( _window, this);
    
    if( _offscreen ) {
      // set rendering destination to FBO
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
      utils::gl::check_error();
    }

    // screen ratio
    auto ratio_x = (_bbox.x_max-_bbox.x_min) / (double) _width;
    auto ratio_y = (_bbox.y_max-_bbox.y_min) / (double) _height;

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "WIN before glVieport OpenGL error: " << err << std::endl;
    }
    glViewport(0, 0, _width, _height);
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "WIN after glViewport OpenGL error: " << err << std::endl;
    }
    glClearColor( 1.0, 1.0, 1.0, 1.0);
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "WIN glClearColor glViewport OpenGL error: " << err << std::endl;
    }
    glClear(GL_COLOR_BUFFER_BIT);
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "WIN glClear OpenGL error: " << err << std::endl;
    }
    
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "WIN PROJECTION OpenGL error: " << err << std::endl;
    }
    glOrtho( _bbox.x_min, _bbox.x_max, _bbox.y_min, _bbox.y_max, 1.f, -1.f);
    while ((err = glGetError()) != GL_NO_ERROR) {
      std::cout << "W=" << _width << "H=" << _height << std::endl;
      std::cerr << _bbox << std::endl;
      std::cerr << "WIN ORTHO OpenGL error: " << err << std::endl;
    }
    glMatrixMode(GL_MODELVIEW);
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "WIN MODELVIEW OpenGL error: " << err << std::endl;
    }
    glLoadIdentity();

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "WIN glLoadIdentity OpenGL error: " << err << std::endl;
    }
    
    glClearColor( 1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "WIN Plot OpenGL error: " << err << std::endl;
    }
    // Render Plots
    for( const auto& plotter: _plotters) {
      plotter->render( ratio_x, ratio_y );
    }
    
    if( not _offscreen ) {
      glfwSwapBuffers( _window );
      glfwPollEvents();
    }
  }
  // ************************************************************ Window::save
  void save( const std::string& filename )
  {
    std::cout << "__SAVE" << std::endl;
    // Make sure using current window
    glfwMakeContextCurrent( _window );
    // TODO can also be set to another DataStructure
    glfwSetWindowUserPointer( _window, this);

    if( _offscreen ) {
      // set rendering destination to FBO
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
      utils::gl::check_error();
    }
    // debug
    std::cout << "  before" << std::endl;
    utils::gl::to_png( filename );
    std::cout << "  after" << std::endl;
  }

  // ******************************************************* Window::attributs
  std::string _title;
  int _width, _height;
  bool _offscreen;
  GLFWwindow* _window;
  /** Fonts to write text */
  FTGLTextureFont* _font;
  /** GLew variables for FrameBufferObject, RenderBuffer */
  GLuint _fbo, _render_buf;

private:
  // *************************************************** Window::GLFW callback
  static void resize_callback( GLFWwindow* window, int width, int height )
  {
    // call Class method
    ((Window *)glfwGetWindowUserPointer(window))->on_resized();
  }
  void on_resized()
  {
    std::cout << "__RESIZE" << std::endl;
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
  }
public:
  static void error_callback(int error, const char* description)
  {
    std::cerr <<  description << std::endl;
    //fputs(description, stderr);
  }
}; // Window

#endif // WINDOW_HPP
