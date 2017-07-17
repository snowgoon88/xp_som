/* -*- coding: utf-8 -*- */

#ifndef GL_UTILS_HPP
#define GL_UTILS_HPP

/** 
 * Utilities for OpenGL
 * - save screen as PNG
 */
#include <iostream>              // std::cout
#include <png++/png.hpp>         // lipbg++
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
  std::cout << "__ SAVE to_png in " << filename << std::endl;
  // Window dimension, through VIEWPORT
  GLint screen_dim[4];
  glGetIntegerv( GL_VIEWPORT, screen_dim);

  // std::cout << "  dim = " << "+" << screen_dim[0] << ", " << screen_dim[1];
  // std::cout << " " << screen_dim[2] << " x " << screen_dim[3] << std::endl;

  png::image<png::rgb_pixel> image(screen_dim[2], screen_dim[3]);

  // get image pixels
  //unsigned char *pixels = (unsigned char*) malloc( 3 * screen_dim[2] * screen_dim[3] );
  // new buffer, access raw with .get()
  std::unique_ptr<unsigned char[]> pixels( new unsigned char [3 * screen_dim[2] * screen_dim[3]]);

  glReadPixels( screen_dim[0], screen_dim[1], screen_dim[2], screen_dim[3],
		GL_RGB, GL_UNSIGNED_BYTE, pixels.get() );
  // std::cout << "__ READ buffer end" << std::endl;

  // copy image
  for( auto y = screen_dim[1]; y <screen_dim[3]; ++y ) {
    for( auto x = screen_dim[0]; x < screen_dim[2]; ++x ) {
      image[screen_dim[3]-y-1][x] = png::rgb_pixel( pixels[3*x + y*3*screen_dim[2] + 0],
						    pixels[3*x + y*3*screen_dim[2] + 1],
						    pixels[3*x + y*3*screen_dim[2] + 2] );
    }
  }
  // save image
  image.write( filename.c_str() );
}

}; // namespace gl
}; // namespace utils


#endif // GL_UTILS_HPP
