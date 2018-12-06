/* -*- coding: utf-8 -*- */

#ifndef VISUGL_HPP
#define VISUGL_HPP

/** 
 * Global header for VisuGL
 */

#include <list>
// ******************************************************************* TypeDef
using GraphicText = struct {
  double x, y;
  const std::string msg;
};
using BoundingBox = struct {
  double x_min, x_max, y_min, y_max;
};
std::ostream& operator<<( std::ostream& os, const BoundingBox& bbox )
{
  os << "{" << bbox.x_min <<"; " << bbox.x_max << "; ";
  os << bbox.y_min << "; " << bbox.y_max << "}";
  return os;
}
class Plotter;
using PlotterPtr = Plotter*;
using PlotterList = std::list<PlotterPtr>;
// ***************************************************************************

#endif // VISUGL_HPP
