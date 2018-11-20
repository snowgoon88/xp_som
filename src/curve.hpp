/* -*- coding: utf-8 -*- */

#pragma once

/** 
 * A Curve rended in OpenGL. **!! Classical/Legacy OpenGL !!**
 *
 * Samples can be static copies of data
 * - add_sample
 * - add_time_serie
 *
 * Samples can be dynamical and come from
 * - a container : 
 * - something with constant iterators :
 *
 * WARNING : 'samples' are considered taken from a growing time_serie.
 * I've added 'add_data' which does not suppose a time serie.
 */

#include <list>
#include <iostream>
#include <math.h>
#include <functional>

// ***************************************************************************
// ********************************************************************* Curve
// ***************************************************************************
class Curve
{
public:
  /** Un Sample est un triplet */
  using Sample = struct {
    double x;
    double y;
    double z;
  };

  /** Bounding box around data */
  using BoundingBox = struct {
    double x_min, x_max, y_min, y_max;
  };

  /** Color */
  using Color = struct {
    double r,g,b;
  };

public:
  // ********************************************************* Curve::creation
  /** Creation */
  Curve() : _fg_col{1,0,0}, _line_width(1.f) // red,thin
  {}
  virtual ~Curve()
  {
    //std::cout << "Curve destroyed" << std::endl;
  };

  // ******************************************************Curve::copycreation
  Curve( const Curve& c)
    : _data(c._data), _bbox( c._bbox ),
      _fg_col( c._fg_col ), _line_width( c._line_width )
  {}
  
  // ************************************************************ Curve::clear
  void clear()
  {
    _data.clear();
	_bbox = {0.0, 0.0, 0.0, 0.0};
  }
  // ******************************************************* Curve::add_sample
  template<typename Itr>
  void add_sample( const Itr& x_data_begin, const Itr& x_data_end,
		   const Itr& y_data_begin, const Itr& y_data_end )
  {
    //std::cout << "Curve::add_sample_with_Itr" << std::endl;
    auto it_x = x_data_begin;
    auto it_y = y_data_begin;

	for (; it_x != x_data_end and it_y != y_data_end; ++it_x, ++it_y) {
	  //std::cout << "add " << *it_x << ", " << *it_y << std::endl;
	  add_sample( {*it_x, *it_y, 0.0} );
	}
  }
  /** Add a point to the Curve and adjust _bbox */
  virtual void add_sample( const Sample sample)
  {
    //std::cout << "Curve::add_sample" << std::endl;
    // First sample ?
    if (_data.size() == 0 ) {
      //std::cout << "add_sample : NEW" << std::endl;
      _data.push_back( sample );
      _bbox.x_min = sample.x;
      _bbox.x_max = sample.x;
      _bbox.y_min = sample.y;
      _bbox.y_max = sample.y;
    }
    else {
      //std::cout << "add_sample : ADD" << std::endl;
      // Check it is after the last point
      if (sample.x > get_bbox().x_max) {
	//std::cout << "add_sample : OK"  << std::endl;
	_data.push_back( sample );
	_bbox.x_max = sample.x;
	if (sample.y > get_bbox().y_max) _bbox.y_max = sample.y;
	if (sample.y < get_bbox().y_min) _bbox.y_min = sample.y;
      }
    }
  }
  /** Add any data (not as atime serie) to Curve and adust _bbox */
  virtual void add_data( const Sample sample)
  {
    //std::cout << "Curve::add_data" << std::endl;
    // First sample ?
    if (_data.size() == 0 ) {
      //std::cout << "add_sample : NEW" << std::endl;
      _data.push_back( sample );
      _bbox.x_min = sample.x;
      _bbox.x_max = sample.x;
      _bbox.y_min = sample.y;
      _bbox.y_max = sample.y;
    }
    else {
      //std::cout << "add_sample : ADD" << std::endl;
      _data.push_back( sample );
      
      // Check it is after the last point
      if (sample.x > get_bbox().x_max) _bbox.x_max = sample.x;
      if (sample.x < get_bbox().x_min) _bbox.x_min = sample.x;
      if (sample.y > get_bbox().y_max) _bbox.y_max = sample.y;
      if (sample.y < get_bbox().y_min) _bbox.y_min = sample.y;
    }
  }
  // *************************************************** Curve::add_time_serie
  template<typename Itr>
  void add_time_serie( const Itr& data_begin, const Itr& data_end )
  {
    auto it = data_begin;
    for (auto t_data=0.0; it != data_end; ++it, t_data+=1.0) {
	  const Sample s = {t_data, it, 0.0};
      add_sample( s );
    }
  }

  // ******************************************************** Curve::set_color
  void set_color( const Color& col )
  {
    _fg_col = col;
  }
  // ******************************************************** Curve::set_width
  void set_width( const GLfloat& width )
  {
	_line_width = width;
  }
  // *********************************************************** Curve::render
  /** Draw curve with OpenGL */
  virtual void render()
  {
    //std::cout << "  Curve::render" << std::endl;
    // for( auto& pt : _data) {
    //   std::cout << "[ " << pt.x << "; " << pt.y << "; " << pt.z << "]" << std::endl;
    // }

    // Color
    glColor4d( _fg_col.r, _fg_col.g, _fg_col.b, 1.0);
    // -------------------------------------------------------------------------
    //  Rendering using GL_LINE_STRIP
    // -------------------------------------------------------------------------
    glEnable (GL_BLEND);
    glEnable (GL_LINE_SMOOTH);
    glLineWidth( _line_width );

    glBegin(GL_LINE_STRIP);
    for( auto& pt: _data) {
      glVertex3d( pt.x, pt.y, pt.z );
    }
    glEnd();
    
  }
  // ******************************************************** Curve::attributs
  /** get BoundingBox */
  const BoundingBox& get_bbox() const {return _bbox;}
  std::list<Sample> get_samples() const { return _data; }
  Color get_color() const { return _fg_col; }
protected:
  /** Data are a list of Samples*/
  std::list<Sample> _data;
  /** Bounding box around Data */
  BoundingBox _bbox;
  /** Color for the Curve */
  Color _fg_col;
  /** Line Width */
  GLfloat _line_width;
  
public:
  /** Create artificial data y=sin(x) pour x=[0,2PI[ */
  void create_data()
  {
	const unsigned int _nb_data = 100;
	
    std::cout << "Create_data nb=" << _nb_data << std::endl;
    for( unsigned int i=0; i < _nb_data; ++i) {
      Sample pt;
      pt.x = 2.0 * M_PI * i / _nb_data;
      pt.y = sin( pt.x );
      pt.z = 0.0;      
      _data.push_back( pt );
    }
    _bbox = {0.0, 2.0 * M_PI, -1.0, 1.0};
    std::cout << "Curve  size(data)=" << _data.size() << std::endl;
    std::cout << "bbox = {" << get_bbox().x_min <<"; " << get_bbox().x_max << "; ";
    std::cout << get_bbox().y_min << "; " << get_bbox().y_max << "}" << std::endl;
  }
};
// ***************************************************************************

// ***************************************************************** CurveMean
/**
 * CurveMean : 
 * in mean_mode, temporal means are computed online to get a "mouth" line.
 * The _mean_length is computed dynamically. When new Samples are added, 
 * _mean_nb_current is incremented and a new mean point is added when
 * it equals _mean_length.
 */
class CurveMean : public Curve
{
public:
  // *******************************************************CuveMean::creation
  CurveMean() : Curve (), _mean_mode(false),
				_mean_length(0), _mean_nb_point(0), _mean_x(0.0), _mean_y(0.0)
  {
  }
  // ******************************************************Curve::copycreation
  CurveMean( const CurveMean& c)
    : Curve(c), _mean_mode( c._mean_mode), _mean_data( c._mean_data ),
      _mean_length( c._mean_length ), _mean_nb_point( c._mean_nb_point ),
      _mean_x( c._mean_x ), _mean_y( c._mean_y )
  {}
  // ************************************************** CurveMean::switch_mode
  void set_mean_mode( bool mode )
  {
	_mean_mode = mode;
  }
  bool get_mean_mode() { return _mean_mode; }
  // ********************************************** CurveMean::recompute_means
  void recompute_means()
  {
	_mean_data.clear();
	// _mean_length so as to have around 100 pts on curve
	_mean_length = _data.size() / 100;

	// then recompute means
	_mean_x = 0.0;
	_mean_y = 0.0;
	_mean_nb_point = 0;
	for( auto data_it = _data.begin(); data_it != _data.end(); ++data_it) {
	  _mean_x += data_it->x;
	  _mean_y += data_it->y;
	  ++_mean_nb_point;
	  if( _mean_nb_point == _mean_length ) {
		_mean_data.push_back( {_mean_x/(double)_mean_length,
			  _mean_y/(double) _mean_length,
			  0.0 } );
		_mean_x = 0.0;
		_mean_y = 0.0;
		_mean_nb_point = 0; 
	  }	
	}
  }
  // *************************************************** CurveMean::add_sample
  /** Add a point to the CurveMean */
  void add_sample( const Sample sample)
  {
	Curve::add_sample( sample );

	if( _mean_mode ) {
	  ++_mean_nb_point;
	  _mean_x += sample.x;
	  _mean_y += sample.y;
	  if( _mean_nb_point == _mean_length ) {
		_mean_data.push_back( {_mean_x/(double)_mean_length,
			  _mean_y/(double) _mean_length,
			  0.0 } );
		_mean_x = 0.0;
		_mean_y = 0.0;
		_mean_nb_point = 0; 
	  }
	}
  }
  // ******************************************************* CurveMean::render
  /** Draw curve with OpenGL */
  virtual void render()
  {
    //std::cout << "  Curve::render" << std::endl;
    // for( auto& pt : _data) {
    //   std::cout << "[ " << pt.x << "; " << pt.y << "; " << pt.z << "]" << std::endl;
    // }

    // Color
    glColor4d( _fg_col.r, _fg_col.g, _fg_col.b, 1.0);
    // -------------------------------------------------------------------------
    //  Rendering using GL_LINE_STRIP
    // -------------------------------------------------------------------------
    glEnable (GL_BLEND);
    glEnable (GL_LINE_SMOOTH);
    glLineWidth( _line_width );

    glBegin(GL_LINE_STRIP);
	if( _mean_mode ) {
	  for( auto& pt: _mean_data) {
		glVertex3d( pt.x, pt.y, pt.z );
	  }
	}
	else {
	  for( auto& pt: _data) {
		glVertex3d( pt.x, pt.y, pt.z );
	  }
	}
    glEnd();
  }
  
  // **************************************************** CurveMean::attributs
protected:
  bool _mean_mode;
  /** Mean Data are a list of Samples*/
  std::list<Sample> _mean_data;
  /** Mean parameters and variables */
  int _mean_length, _mean_nb_point;
  double _mean_x, _mean_y;
};

// ***************************************************************************
// ****************************************************************** CurveDyn
// ***************************************************************************
template< typename T>
class CurveDyn : public Curve
{
public:
  // ****************************************************** CurveDyn::creation
  /**
   * With a Container and a Functor that access
   * the elemen inside a element of class T.
   * By default, it only dereferences the Iterator
   */
  using FunIt =  std::function<const double (typename T::const_iterator )>;
  CurveDyn( T& container,
			std::function<const double (typename T::const_iterator)> fun = [] (typename T::const_iterator it) -> const double {const double res = *(it); return res;} ) :
	Curve(),
	_model(container), _fun_it(fun)
  {
	auto it= _model.begin();
	for( double t_data=0.0;
		 it != _model.end();
		 ++it, t_data+=1.0 ) {
	  const Sample s{t_data, _fun_it(it), 0.0};
	  add_sample( s );
	}
  }
  // ******************************************************** CurveDyn::update
  void update()
  {
	clear();
	auto it = _model.begin();
	for( auto t_data=0.0;
		 it != _model.end();
		 ++it, t_data+=1.0 ) {
	  auto s = Sample{t_data, _fun_it(it), 0.0};
	  add_sample( s );
	}
  }
  // ***************************************************** CurveDyn::attributs
  /** Model is a Container */
  const T& _model;
  /** Functor from Container::iterator to double */
  FunIt _fun_it;
};
