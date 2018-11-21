/* -*- coding: utf-8 -*- */

#ifndef IMG_PLOTTER_HPP
#define IMG_PLOTTER_HPP

#include <vector>
#include <colormap.hpp>

/** 
 * plot an image (and its Colormap) on screen.
 */

// ***************************************************************************
// **************************************************************** ImgPlotter
// ***************************************************************************
template< typename TData >
class ImgPlotter
{
public:
  // **************************************************** ImgPlotter::creation
  ImgPlotter( TData& data, unsigned int width, unsigned int height ) :
    _data(data), _img_width(width), _img_height(height), _cmap()
  {
    // _img_width = 300;
    // _img_height = 200;

    // // fill image
    // for( unsigned int row = 0; row < _img_height; ++row) {
    //   for( unsigned int col = 0; col < _img_width; ++col) {
    //     auto c = ((((row&0x8)==0)^((col&0x8))==0))*255;
    //     _img.push_back( (float) c / 255.0f );
    //     _img.push_back( (float) c / 300.0f );
    //     _img.push_back( (float) c / 400.0f );
    //   }
    // }

    auto colors = _cmap.apply( _data );
    for( auto& col: colors) {
      _img.push_back( col.r );
      _img.push_back( col.g );
      _img.push_back( col.b );
    }
  }
  // ****************************************************** ImgPlotter::render
  virtual void render()
  {
    glRasterPos2i(0, 0);
    glDrawPixels( _img_width, _img_height, GL_RGB, GL_FLOAT,
                  _img.data() );
                  
  }
  // *************************************************** ImgPlotter::attributs
  TData& _data;
  unsigned int _img_width, _img_height;
  Colormap _cmap;
  std::vector<float> _img;
}; // ImgPlotter



#endif // IMG_PLOTTER_HPP
