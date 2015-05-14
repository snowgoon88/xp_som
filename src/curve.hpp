/* -*- coding: utf-8 -*- */

#pragma once
#include <list>
#include <iostream>
#include <math.h>

/** 
 * Trace une courbe en OpenGL.
 */
const unsigned int _nb_data = 100;


class Curve
{
public:
  /** Un Sample est un triplet */
  typedef struct {
    double x;
    double y;
    double z;
  } Sample;

  /** Bounding box autour des données */
  typedef struct {
    double x_min, x_max, y_min, y_max;
  } BoundingBox;

public:
  /** Creation */
  Curve() {std::cout << "Curve creation" << std::endl; /*create_data();*/};
  ~Curve() {std::cout << "Curve destroyed" << std::endl;};

  /** Ajoute un point à la courbe et ajuste _bbox */
  void add_sample( const Sample& sample)
  {
    // Premier sample ?
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
      // Vérifie qu'il est après le dernier point
      if (sample.x > get_bbox().x_max) {
	//std::cout << "add_sample : OK"  << std::endl;
	_data.push_back( sample );
	_bbox.x_max = sample.x;
	if (sample.y > get_bbox().y_max) _bbox.y_max = sample.y;
	if (sample.y < get_bbox().y_min) _bbox.y_min = sample.y;
      }
    }
  }
  
  
  /** Draw curve with OpenGL */
  void render()
  {
    // for( auto& pt : _data) {
    //   std::cout << "[ " << pt.x << "; " << pt.y << "; " << pt.z << "]" << std::endl;
    // }

    // Couleur rouge
    glColor4d( 1.0, 0.0, 0.0, 1.0);
    // -------------------------------------------------------------------------
    //  Rendering using GL_LINE_STRIP
    // -------------------------------------------------------------------------
    glEnable (GL_BLEND);
    glEnable (GL_LINE_SMOOTH);
    glLineWidth (1.0);

    glBegin(GL_LINE_STRIP);
    for( auto& pt: _data) {
      glVertex3d( pt.x, pt.y, pt.z );
    }
    glEnd();
    
  }

  /** get BoundingBox */
  const BoundingBox& get_bbox() {return _bbox;}; 

private:
  /** Les données sont une list de Sample */
  std::list<Sample> _data;
  /** Bounding box autour des données */
  BoundingBox _bbox;

public:
  /** Create artificial data y=sin(x) pour x=[0,2PI[ */
  void create_data()
  {
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
