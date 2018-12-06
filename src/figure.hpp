/* -*- coding: utf-8 -*- */

#ifndef FIGURE_HPP
#define FIGURE_HPP

/** 
 * A Figure has Axes and is a Plotter (which can containes Plotter).
 * Can have a title.
 */

#include <iostream>                  // std::cout
#include <string>                    // std::string

#include <plotter.hpp>
#include <window.hpp>
#include <axis.hpp>
#include <gl_utils.hpp>              // utils::gl::to_png
#include <visugl.hpp>


// ***************************************************************************
// ******************************************************************** Figure
// ***************************************************************************
class Figure : public Plotter
{
public:
  // ******************************************************** Figure::creation
  Figure( const Window& window,
          std::string title = "",
	  const Range& x_range = {-1.0, 1.0, 10, 2},
	  const Range& y_range = {-1.0, 1.0, 10, 2} ) :
    Plotter(),
    _title( title ), 
    _update_axes_x( false ), _update_axes_y( false ),
    _draw_axes( true ),
    _axis_x( "X", x_range),
    _axis_y( "Y", y_range),
    _text_list(),
    _font( window._font ), _title_offset( 0.0 )
  {
  }
  // ***************************************************** Figure::destruction
  ~Figure()
  {
  }

  // *************************************************** Figure::set_draw_axes
  void set_draw_axes( bool draw_axes )
  {
    _draw_axes = draw_axes;
  }
  // ***************************************************** Figure::GraphicText
  void clear_text()
  {
    _text_list.clear();
  }
  void add_text( const std::string msg, double x=0.0, double y=0.0 )
  {
    _text_list.push_back( GraphicText{ x, y, msg } );
  }
  // ********************************************************** Figure::update
  BoundingBox get_innerbbox()
  {
    // Build proper axis by finding min/max on each axe
    BoundingBox bbox{ std::numeric_limits<double>::max(),
        (-std::numeric_limits<double>::max()),
        std::numeric_limits<double>::max(),
        -std::numeric_limits<double>::max() };
    
    for( const auto& plotter: _plotters ) {
      auto b = plotter->get_bbox();
      if( b.x_min < bbox.x_min ) bbox.x_min = b.x_min;
      if( b.x_max > bbox.x_max ) bbox.x_max = b.x_max;
      if( b.y_min < bbox.y_min ) bbox.y_min = b.y_min;
      if( b.y_max > bbox.y_max ) bbox.y_max = b.y_max;
    }

    return bbox;
  }
  virtual void update_bbox()
  {
    auto innerbox = get_innerbbox();
    // std::cout << "__UPD axes with bbox={" << innerbox.x_min << "; " << innerbox.x_max << "; " << innerbox.y_min << "; " << innerbox.y_max << "}" << std::endl;
    _axis_x = Axis( "X", {innerbox.x_min, innerbox.x_max, 10, 2});
    _axis_y = Axis( "Y", {innerbox.y_min, innerbox.y_max, 10, 2});

    // Some room for title
    if (_title != "" ) {
      innerbox.y_max += 0.1* (innerbox.y_max - innerbox.y_min);
      auto titlebox = _font->BBox( _title.c_str() );
      _title_offset = (titlebox.Upper().X() - titlebox.Lower().X()) / 2.0;
    }
    
    set_bbox( innerbox );
    // std::cout << "  figure  = " << get_bbox() << std::endl;
    // std::cout << "  offset = " << _title_offset << std::endl;
  }
  
  // ********************************************************** Figure::render
  virtual void render( float screen_ratio_x = 1.0, float screen_ratio_y = 1.0 )
  {

    if( _update_axes_x || _update_axes_y ) {
      auto innerbox = get_innerbbox();

      if( _update_axes_x) 
        _axis_x = Axis( "X", {innerbox.x_min, innerbox.x_max, 10, 2});
      if( _update_axes_y )
        _axis_y = Axis( "Y", {innerbox.y_min, innerbox.y_max, 10, 2});
      // Some room for title
      if (_title != "" ) {
        innerbox.y_max += 0.1* (innerbox.y_max - innerbox.y_min);
        auto titlebox = _font->BBox( _title.c_str() );
        _title_offset = (titlebox.Upper().X() - titlebox.Lower().X()) / 2.0;
      }

      set_bbox( innerbox );
    }
    

    // All other objects
    for( const auto& plotter: _plotters) {
      plotter->render( screen_ratio_x, screen_ratio_y );
    }
    
    // Basic axes
    if( _draw_axes ) {
      _axis_x.render( screen_ratio_x, screen_ratio_y );
      glPushMatrix(); // AXE_Y
      glRotated( 90.0, 0.0, 0.0, 1.0 );
      _axis_y.render( screen_ratio_y, screen_ratio_x);
      glPopMatrix(); // AXE_Y
    }

    // GraphicText
    for( auto& txt: _text_list) {
      glPushMatrix(); {
        glTranslated( txt.x, txt.y, 0.0);
        glScaled( screen_ratio_x, screen_ratio_y, 1.0 );
        _font->Render( txt.msg.c_str() );
      } glPopMatrix();
    }
    // Title
    if (_title != "") {
      glPushMatrix(); {
        glTranslated( (get_bbox().x_max - get_bbox().x_min) / 2.0 - (_title_offset*screen_ratio_x),
                      (get_bbox().y_max - 0.05 * (get_bbox().y_max - get_bbox().y_min)),
                      0.0 );
        glScaled( screen_ratio_x, screen_ratio_y, 1.0 );
        _font->Render( _title.c_str() );
      } glPopMatrix();
    }
  }

public:
  // ******************************************************* Figure::attributs
  std::string _title;
  /** X and Y axes*/
  bool _update_axes_x, _update_axes_y;
  bool _draw_axes;
  Axis _axis_x, _axis_y;
  /** List of Graphictext */
  std::list<GraphicText> _text_list;
  /** Fonts to write text */
  FTFont* _font;
  double _title_offset;
}; // class Figure

#endif // FIGURE_HPP
