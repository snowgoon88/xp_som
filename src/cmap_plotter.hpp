/* -*- coding: utf-8 -*- */

#ifndef CMAP_PLOTTER_HPP
#define CMAP_PLOTTER_HPP

/** 
 * Plot a Colormap vertically
 */
template< typename TData >
class ColormapPlotter : public Plotter
{
public:
  // *********************************************** ColormapPlotter::creation
  ColormapPlotter( ImgPlotter<TData>& img_plotter ) :
    Plotter(), _plt(img_plotter)
  {
    // create cmap_data to be displayed
    _cmap_data.clear();
    for( unsigned int i = 0; i < _plt._cmap._veridis_cmap.size(); ++i) {
      _cmap_data.push_back( double(i) );
    }

    // compute bounding box
    update();

    _cmap_plotter = new ImgPlotter<std::vector<double>>
      ( _cmap_data,
        1, _plt._cmap._veridis_cmap.size(),
        _bbox.x_min, _bbox.x_max, _bbox.y_min, _bbox.y_max);
  }
  virtual ~ColormapPlotter()
  {
    delete _cmap_plotter;
  }
  // ************************************************* ColormapPlotter::update
  void update()
  {
    // Where to plot it
    auto plt_bbox = _plt.get_bbox();
    // x: ratio of BoundingBox of ImgPlotter, with min=20 pixels
    auto x_min = plt_bbox.x_max + 0.05 * (plt_bbox.x_max - plt_bbox.x_min);
    auto x_max = x_min + 0.05 * (plt_bbox.x_max - plt_bbox.x_min);
    // y : same as plt_bbox
    _bbox = {x_min, x_max, plt_bbox.y_min, plt_bbox.y_max};

    // Need also minmax of data
    // auto minmax = std::minmax_element( plt.data.begin(), plt.data.end() );
    // _min_cmap = *(minmax.first);
    // _max_cmap = *(minmax.second);
   }
  // ************************************************* ColormapPlotter::render
  virtual void render( float screen_ratio_x = 1.0, float screen_ratio_y = 1.0 )
  {
    update();
    _cmap_plotter->render( screen_ratio_x, screen_ratio_y );
  }
  // ********************************************** ColormapPlotter::attributs
  ImgPlotter<TData>& _plt;
  std::vector<double> _cmap_data;
  ImgPlotter<std::vector<double>>* _cmap_plotter;
  
}; // ColormapPlotter



#endif // CMAP_PLOTTER_HPP
