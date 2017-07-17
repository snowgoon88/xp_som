/* -*- coding: utf-8 -*- */

#ifndef GL_UTILS_HPP
#define GL_UTILS_HPP

/** 
 * Utilities for OpenGL
 * - save screen as PNG
 */
#include <iostream>              // std::cout
#include <pngwriter.h>
#include <GL/gl.h>               // OpenGL
#include <utils.hpp>              // make_unique

// ***************************************************************************
// ************************************************* Save OpenGL screen as PNG
// ******************************************************************** to_png
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
  
}; // namespace gl
}; // namespace utils


#endif // GL_UTILS_HPP
