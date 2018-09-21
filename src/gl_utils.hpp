/* -*- coding: utf-8 -*- */

#ifndef GL_UTILS_HPP
#define GL_UTILS_HPP

/** 
 * Utilities for OpenGL
 * - save screen as PNG
 * - check for opengl errors
 */
#include <iostream>              // std::cout
#define PNG_SKIP_SETJMP_CHECK    // see /usr/include/libpng12/pngconf.h:383
#include <pngwriter.h>
#include <GL/gl.h>               // OpenGL
#include <utils.hpp>             // for make_unique

namespace utils {
namespace gl {
// ******************************************************************** to_png
void to_png( const std::string& filename )
{
  // std::cout << "__ SAVE to_png in " << filename << std::endl;
  // Window dimension, through VIEWPORT
  GLint screen_dim[4];
  glGetIntegerv( GL_VIEWPORT, screen_dim);

  // std::cout << "  dim = " << "+" << screen_dim[0] << ", " << screen_dim[1];
  // std::cout << " " << screen_dim[2] << " x " << screen_dim[3] << std::endl;

  pngwriter image( screen_dim[2], screen_dim[3],
		   0, // background color
		   filename.c_str() );
  // get image pixels
  // new buffer, access raw with .get()
  std::unique_ptr<GLfloat[]> pixels( new GLfloat [3 * screen_dim[2] * screen_dim[3]]);

  // check OpenGL error
  GLenum err;
  // std::cout << "__ READ buffer start" << std::endl;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL error: " << err << std::endl;
  }
  glReadPixels( screen_dim[0], screen_dim[1], screen_dim[2], screen_dim[3],
		GL_RGB, GL_FLOAT, pixels.get() );
  // std::cout << "__ READ buffer end" << std::endl;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL error: " << err << std::endl;
  }
  // copy image
  // std::cout << "__ SET image start" << std::endl;
  for( auto y = screen_dim[1]; y <screen_dim[3]; ++y ) {
    for( auto x = screen_dim[0]; x < screen_dim[2]; ++x ) {
      image.plot( x, y,
		  pixels[3*x + y*3*screen_dim[2] + 0],
		  pixels[3*x + y*3*screen_dim[2] + 1],
		  pixels[3*x + y*3*screen_dim[2] + 2] );
    }
  }
  // std::cout << "__ SET image end" << std::endl;
  // save image
  // std::cout << "__ SAVE to "<< filename << std::endl;
  image.close();
}

// *************************************************************** check_error
void _check_error(const char *file, int line);
 ///
/// Usage
/// [... some opengl calls]
/// utils::gl::check_error();
///
#define check_error() _check_error(__FILE__,__LINE__)

void _check_error(const char *file, int line) {
  GLenum err (glGetError());
 
  while(err!=GL_NO_ERROR) {
    std::string error;
 
    switch(err) {
    case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
    case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
    case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
    case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
    }
    
    std::cerr << "GL_" << error.c_str() <<" - "<<file<<":"<<line<<std::endl;
    err=glGetError();
  }
}  
  
}; // namespace gl
}; // namespace utils


#endif // GL_UTILS_HPP
