/* -*- coding: utf-8 -*- */

#ifndef CMAP_PLOTTER_HPP
#define CMAP_PLOTTER_HPP

#include <sstream>
#include <iomanip>
#include <FTGL/ftgl.h>       // Fontes dans OpenGL
#define FONT_PATH "ressources/Consolas.ttf"
#define FONT_SIZE 12
#define DIM_MAJOR 6
// Scale par défaut des fontes : écran de 800 de large, axe de -1 à 1.
#define FONT_SCALE ((1.0 - -1.0) / 800.0)

/** 
 * Plot a Colormap vertically
 */
template< typename TData >
class ColormapPlotter : public Plotter
{
public:
  // *********************************************** ColormapPlotter::creation
  ColormapPlotter( const Window& window,
                   ImgPlotter<TData>& img_plotter ) :
    Plotter(), _plt(img_plotter),
    _font( window._font )
  {
    // create cmap_data to be displayed
    _cmap_data.clear();
    for( unsigned int i = 0; i < _plt._cmap._veridis_cmap.size(); ++i) {
      _cmap_data.push_back( double(i) );
    }

    // compute bounding box
    update_bbox();

    // as a 2Dimage of size 1 x size_of_colorma
    _cmap_plotter = new ImgPlotter<std::vector<double>>
      ( _cmap_data,
        1, _plt._cmap._veridis_cmap.size(),
        _bbox.x_min, _bbox.x_max, _bbox.y_min, _bbox.y_max );
  }
  virtual ~ColormapPlotter()
  {
    delete _cmap_plotter;
  }
  // ************************************************* ColormapPlotter::update
  virtual void update_bbox()
  {
    // Where to plot it
    auto plt_bbox = _plt.get_bbox();
    // x: ratio of BoundingBox of ImgPlotter, with min=20 pixels
    auto x_min = plt_bbox.x_max + 0.05 * (plt_bbox.x_max - plt_bbox.x_min);
    auto x_max = x_min + 0.05 * (plt_bbox.x_max - plt_bbox.x_min);
    // y : same as plt_bbox
    set_bbox( {x_min, x_max, plt_bbox.y_min, plt_bbox.y_max} );
    // std::cout << "  cmap    = " << get_bbox() << std::endl;

    // Need also minmax of data
    auto minmax = std::minmax_element( _plt._data.begin(), _plt._data.end() );
    _min_cmap = double(*(minmax.first));
    _max_cmap = double(*(minmax.second));
    
   }
  // ************************************************* ColormapPlotter::render
  virtual void render( float screen_ratio_x = 1.0, float screen_ratio_y = 1.0 )
  {
    //std::cout << "  render_cmap at " << get_bbox() << std::endl;
    //update();
    _cmap_plotter->render( screen_ratio_x, screen_ratio_y );

    // Print CMAP min,max
    std::stringstream min_str, max_str;
    min_str << std::setprecision(2) << _min_cmap;
    max_str << std::setprecision(2) << _max_cmap;

    glColor3f( 0.f, 0.f, 0.f ); // TODO: black but _fg_color ?
    glPushMatrix(); {
      glTranslated( _bbox.x_max + 0.08 * (_bbox.x_max - _bbox.x_min),
                    _bbox.y_min,
                    0.0);
      glScaled( screen_ratio_x, screen_ratio_y, 1.0 );
      _font->Render( min_str.str().c_str() );
    } glPopMatrix();
    
    glPushMatrix(); {
      glTranslated( _bbox.x_max + 0.08 * (_bbox.x_max - _bbox.x_min),
                    _bbox.y_max,
                    0.0);
      glScaled( screen_ratio_x, screen_ratio_y, 1.0 );
      _font->Render( max_str.str().c_str() );
    } glPopMatrix();
  }
  
  // ********************************************** ColormapPlotter::attributs
  ImgPlotter<TData>& _plt;
  std::vector<double> _cmap_data;
  ImgPlotter<std::vector<double>>* _cmap_plotter;
  double _min_cmap, _max_cmap;
  /** Fonts for text */
  FTFont* _font;
  
}; // ColormapPlotter

#endif // CMAP_PLOTTER_HPP
