/* -*- coding: utf-8 -*- */

#ifndef RDSOM1D_VIEWER_HPP
#define RDSOM1D_VIEWER_HPP

/** 
 * Graphical display for a RDSOM network.
 * Derives from Curve, draw small circles and arrow,
 * for neuron weigts and r_weigts, according to a queue.
 *
 * Warning : only on 1D-grid RNetwork.
 */
#include <curve.hpp>

#include <dsom/r_network.hpp>
using RDSOM = Model::DSOM::RNetwork;
using RNeuron = Model::DSOM::RNeuron;
// ***************************************************************************
// *************************************************************** RDSOMViewer
// ***************************************************************************
class RDSOMViewer : public Curve
{
public:
  // ******************************************************* RDSOMViewer::type
  using Pt2D = struct {
    double x,y;
  };
  using Color = struct {
    double r,g,b;
  };
public:
  // *************************************************** RDSOMViewer::creation
  RDSOMViewer( RDSOM& rdsom, std::list<unsigned int>& win_buffer) :
    Curve(),
    _rdsom(rdsom), _win_buffer(win_buffer),
    _ang_min( 0.1 * M_PI ), _ang_max( 2.0 * M_PI ),
    _radius( 1.0), _radius_inner(0.05)
  {
    // Check right structure
    if( rdsom._nb_link != -1 ) {
      std::cerr << "RDSOMViewer only created with 1D-grid RNetwork" << std::endl;
      exit(1);
    }

    // Define the Bounding Box
    _bbox = { -1.5 * _radius, 1.5 * _radius,
	      -1.5 * _radius, 1.5 * _radius };
  }
  // ************************************************ RDSomviewer::plot_neuron
  void plot_neuron( RNeuron& neur )
  {
    // Position of the neuron
    const Pt2D npos
    { _radius * cos( neur.r_pos(0) * (_ang_max - _ang_min)+_ang_min),
	_radius * sin( neur.r_pos(0) * (_ang_max - _ang_min)+_ang_min) };
    // Position of from
    const Pt2D fpos
    { _radius * cos( neur.r_weights(0) * (_ang_max - _ang_min)+_ang_min),
	_radius * sin( neur.r_weights(0) * (_ang_max - _ang_min)+_ang_min) };

    // A small disc à neuron position, color according to weights
    draw_circle( npos,
		 Color{neur.weights(0), neur.weights(0), neur.weights(0)},
		 _radius_inner);
    // and an arrow pointing to neuron
    draw_vector( fpos, npos );
    
  }
  // ***************************************************** RDSomviewer::render
  void render()
  {
    //std::cout << "  RDSOMViewer::draw_back()" << std::endl;
    draw_back( _radius );
    for( auto& idx: _win_buffer) {
      plot_neuron( *(_rdsom.v_neur[idx]) );
    }
    //plot_neuron( *(_rdsom.v_neur[0]) );
    //plot_neuron( *(_rdsom.v_neur[5]) );
    
    //draw_circle( {0.5,0.0}, {0.7, 0.7, 0.7}, 0.5 );
    //draw_circle( {-0.8, 0.2}, {0.9, 0.5, 0.2}, 0.5 );
  }
  // ************************************************** RDSOMViewer::attributs
  /** Model */
  RDSOM& _rdsom;
  /** How many neurones to display */
  std::list<unsigned int>& _win_buffer;
  /** Angles Limits */
  double _ang_min, _ang_max;
  /** Radius of circle */
  double _radius;
  double _radius_inner; // small circles
private:
  // ******************************************** RDSOMviewer::graphic_utility
  void draw_circle( const Pt2D& pt, const Color& col,
		    const double radius = 0.05,
		    unsigned int nb_side=16)
  {
    
    // Inner_cirle
    glColor3d( col.r, col.g, col.b );
    glBegin(GL_TRIANGLE_FAN); {
      glVertex3d( pt.x, pt.y, 0.0);
      glVertex3d( pt.x + radius, pt.y, 0.0);
      for( unsigned int i = 0; i < nb_side; ++i) {
	glVertex3d( pt.x + radius * cos( (double) (i+1) * 2.0 * M_PI / (double) nb_side),
		    pt.y + radius * sin( (double) (i+1) * 2.0 * M_PI / (double) nb_side),
		    0.0 );
      }
    }
    glEnd();

    // dark outer circle
    glColor3d( 0.0, 0.0, 0.0 );
    glBegin(GL_LINE_STRIP); {
      glVertex3d( pt.x + radius, pt.y, 0.0);
      for( unsigned int i = 0; i < nb_side; ++i) {
	glVertex3d( pt.x + radius * cos( (double) (i+1) * 2.0 * M_PI / (double) nb_side),
		    pt.y + radius * sin( (double) (i+1) * 2.0 * M_PI / (double) nb_side),
		    0.0 );
      }
    }
    glEnd();
  }
  // -----------------------------------
  /** vector = ligne + triangle */
  void draw_vector( const Pt2D& from, const Pt2D& to )
  {
    // colinear vector
    Pt2D cv { (to.x-from.x), (to.y-from.y) };
    double norm_cv = sqrt( cv.x * cv.x + cv.y * cv.y  );
    // perp vector
    Pt2D ov {-cv.y, cv.x };

    // New start-end
    Pt2D start { from.x + _radius_inner / norm_cv * cv.x,
	from.y + _radius_inner / norm_cv * cv.y };
    Pt2D end { to.x - _radius_inner / norm_cv * cv.x,
	    to.y - _radius_inner / norm_cv * cv.y };
    
    // Arrow pts
    double tip_length = 0.12;
    double tip_width = 0.02;
    Pt2D a1{ end.x - tip_length*cv.x + tip_width * ov.x,
	end.y - tip_length*cv.y + tip_width * ov.y };
    Pt2D a2{ end.x - tip_length*cv.x - tip_width * ov.x,
	end.y - tip_length*cv.y - tip_width * ov.y };

    // Drawing LINE then TRIANGLE
    glColor3d( 0.0, 0.0, 0.0);
    glBegin(GL_LINES); {
      glVertex3d( start.x, start.y, 0.0 );
      glVertex3d( end.x, end.y, 0.0 );
    }
    glEnd();
    glBegin(GL_TRIANGLES); {
      glVertex3d( end.x, end.y, 0.0 );
      glVertex3d( a1.x, a1.y, 0.0 );
      glVertex3d( a2.x, a2.y, 0.0 );
    }
    glEnd();
  }
  // -----------------------------------
  /** RDSOM1D as an arc of a circle */
  void draw_back( const double radius )
  {
    unsigned int nb_side = 128;
    
    glColor3d( 0.0, 0.0, 0.0 );
    glBegin(GL_LINE_STRIP); {
      auto angle = _ang_min;
      glVertex3d( radius* cos(angle), radius* sin(angle), 0.0);
      for( unsigned int i = 0; i <= nb_side; ++i) {
	angle = _ang_min + (double) i / (double) nb_side * (_ang_max - _ang_min);
	glVertex3d( radius * cos( angle ),
		    radius * sin( angle ),
		    0.0 );
      }
    }
    glEnd();
    
    // start and end lines
    double rmin = 0.9;
    double rmax = 1.1;
    glBegin(GL_LINES); {
      glVertex3d( radius*rmin*cos(_ang_min), radius*rmin*sin(_ang_min), 0.0);
      glVertex3d( radius*rmax*cos(_ang_min), radius*rmax*sin(_ang_min), 0.0);
      glVertex3d( radius*rmin*cos(_ang_max), radius*rmin*sin(_ang_max), 0.0);
      glVertex3d( radius*rmax*cos(_ang_max), radius*rmax*sin(_ang_max), 0.0);
    }
    glEnd();
  }
  
};


#endif // RDSOM1D_VIEWER_HPP
