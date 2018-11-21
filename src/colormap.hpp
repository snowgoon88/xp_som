/* -*- coding: utf-8 -*- */

#ifndef COLORMAP_HPP
#define COLORMAP_HPP

#include <vector>
#include <algorithm>    // std::minmax
//#include <iostream>     // std::cout
#include <math.h>       // floor
/** 
 * Defintion and usage of Colormap.
 * Most often Colormap.apply( to_data )
 */

// ***************************************************************************
// ****************************************************************** Colormap
// ***************************************************************************
class Colormap
{
public:
  using ColorRGB = struct {
    float r, g, b;
  };
  using ColorList = std::vector<ColorRGB>;

public:
  // ****************************************************** Colormap::creation
  Colormap()
  {
  }
  // ********************************************************* Colormap::apply
  ColorList apply( const std::vector<double>& data )
  {
    // To normalize data, need min and max
    auto minmax = std::minmax_element( data.begin(), data.end() );
    auto min = *(minmax.first);
    auto max = *(minmax.second);

    ColorList result;
    
    for( auto& val: data) {
      auto normalized = (val - min)/(max - min) * (_veridis_cmap.size()-1);
      auto idx_color = static_cast<unsigned int>( floor(normalized) );
      result.push_back( _veridis_cmap[idx_color] );
    }

    return result;
  }
  // ****************************************************** Colormap::atributs
#include "veridis_cmap.hpp"

}; // Colormap


#endif // COLORMAP_HPP
