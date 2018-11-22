/* -*- coding: utf-8 -*- */

#ifndef IMG_PLOTTER_HPP
#define IMG_PLOTTER_HPP

#include <vector>
#include <colormap.hpp>
#include <plotter.hpp>      // for BoundingBox

/** 
 * plot an image (and its Colormap) on screen.
 */

// ***************************************************************************
// **************************************************************** ImgPlotter
// ***************************************************************************
template< typename TData >
class ImgPlotter : public Plotter
{
  // /** Bounding box around data */
  // using BoundingBox = struct {
  //   double x_min, x_max, y_min, y_max;
  // };
public:
  // **************************************************** ImgPlotter::creation
  ImgPlotter( TData& data, unsigned int width, unsigned int height,
              double x_min = 0.0, double x_max = 1.0,
              double y_min = 0.0, double y_max = 1.0) :
    Plotter( x_min, x_max, y_min, y_max ),
    _data(data), _img_width(width), _img_height(height),
    _cmap()
  {
    auto colors = _cmap.apply( _data );
    for( auto& col: colors) {
      _img.push_back( col.r );
      _img.push_back( col.g );
      _img.push_back( col.b );
    }
  }
  // ****************************************************** ImgPlotter::render
  /* scr_ratio is (max - min) in screen space / length in pixel size */
  virtual void render( float screen_ratio_x = 1.0, float screen_ratio_y = 1.0 )
  {
    GLfloat zoomx = (_bbox.x_max - _bbox.x_min) /
      screen_ratio_x / float(_img_width);
    GLfloat zoomy = (_bbox.y_max - _bbox.y_min) /
      screen_ratio_y / float(_img_height);
    
    glRasterPos2f( _bbox.x_min, _bbox.y_min); // in screen coordinate
    glPixelZoom( zoomx, zoomy );
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
