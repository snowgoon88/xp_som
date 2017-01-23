/* -*- coding: utf-8 -*- */

#pragma once

/** 
 * Trace un axe (selon Ox "relatif").
 */


#include <GL/gl.h>           // OpenGL
#include <FTGL/ftgl.h>       // Fontes dans OpenGL
#include <math.h>       

#define FONT_PATH "ressources/Consolas.ttf"
#define FONT_SIZE 12
#define DIM_MAJOR 6
// Scale par défaut des fontes : écran de 800 de large, axe de -1 à 1.
#define FONT_SCALE ((1.0 - -1.0) / 800.0)

// Paramètres pour mettre à jour les Ranges
#define NB_MAJ_MIN  5
#define NB_MAJ_MAX 10

/**
 * Un Range est une structure composée de :
 * - min :   Value of first major ticks 
 * - max :   Value of last major ticks 
 * - major : Number of major intervals
 * - minor : Number of minor intervals
 *
 * Lors de l'affichage, on calcule le scale_x et scale_y pour que le texte soit affiché
 * avec une taille de police de ~environs~ FONT_SIZE en fonction de la taille de la fenètre
 * et de la taille de l'espace affiche (dépend de la matrice de projection).
 * De même,
 * - DIM_MAJOR est la taille des ticks en 'pt' (même échelle que les fontes)
 * - les textes sont décalés de multiples de DIM_MAJOR.
 *
 * On utilise update(val) pour modifier un range de manière à ce que 'val' soit dans (_min,_max).
 * On modifie alors _min ou _max pour avoir entre NB_MAJ_MIN et NB_MAJ_MAX intervales majeurs.
 */
class Range
{
public:
  double _min,_max;
  unsigned int _major, _minor;

  /**
   * Ajuste Range avec une nouvelle valeur de max.
   * Il y aura entre NB_MAJ_MIN et NB_MAJ_MAX major tick d'une valeur de x.10^y, avec x entier.
   */
  void update( double vmax )
  {
    if (vmax > _max ) {
      //std::cout << "Range::update SUP with " << vmax << std::endl;
      // la valeur d'un intervale en x.10^y
      double delta = log10( (vmax - _min) / NB_MAJ_MIN );
      double exposant = exp10( floor( delta ));
      double mantisse = floor( exp10( delta - floor(delta) ));

      //std::cout << "===> delta = " << mantisse * exposant  << std::endl;
      // Ajuste Range
      _max = _min + NB_MAJ_MAX * mantisse * exposant;
      _major = NB_MAJ_MAX;
      //std::cout << "new Range = {" << _min << "; " << _max << ";" << _major << "}" << std::endl;
    }
    else if (vmax < _min) {
      // std::cout << "Range::update INF with " << vmax << std::endl;
      // la valeur d'un intervale en x.10^y
      double delta = log10( (_max - vmax) / NB_MAJ_MIN );
      double exposant = exp10( floor( delta ));
      double mantisse = floor( exp10( delta - floor(delta) ));

      // std::cout << "===> delta = " << mantisse * exposant  << std::endl;
      // Ajuste Range
      _min = _max - NB_MAJ_MAX * mantisse * exposant;
      _major = NB_MAJ_MAX;
      //std::cout << "new Range = {" << _min << "; " << _max << ";" << _major << "}" << std::endl;
    }
  }
};

/**
 * Un Axe par défaut à comme _title "X", avec un Range _range={-1,1,3,9}.
 */
class Axis
{
public:
  /** Creation */
  Axis() : _title("X"), _range({-1.0,1.0,3,9}) { init_font(); };
  Axis( const std::string& title,
	const Range& range= {-1.0, 1.0, 2, 10} ) :
    _title(title),
    _range(range) { init_font(); };

  /** Init Fonts */
  void init_font()
  {
    //Axis::_font = new FTGLTextureFont( FONT_PATH );
    if (! Axis::_font) {
      std::cerr << "ERROR: Unable to open file " << FONT_PATH << std::endl;
    }
    else {
      if (!_font->FaceSize(FONT_SIZE)) {
	std::cerr << "ERROR: Unable to set font face size " << FONT_SIZE << std::endl;
	}
    }
  }
  
  /** Draw Axis with OpenGL */
  void render ( double scale_x =  FONT_SCALE, double scale_y = FONT_SCALE)
  {
    // Couleur Noire
    glColor4d(0.0, 0.0, 0.0, 1.0);

    // Position (-1, 0, 0)
    glPushMatrix(); // POS 
    //glTranslated( -1.0, 0.0, 0.0);

    // Axis (SIZE_X)
    glLineWidth(2.0f);
    glBegin(GL_LINES); {
      glVertex3d( _range._min, 0.0, 0.0);
      glVertex3d( _range._max, 0.0, 0.0);
    }
    glEnd();

    // Major and Minor Ticks
    double delta_major = (_range._max - _range._min) / (double) (_range._major);
    double delta_minor = delta_major / (double) _range._minor;
    
    glLineWidth (2.0f);
    glBegin (GL_LINES); {
      for( unsigned int i = 0; i < _range._major+1; ++i) {
	glVertex3d( _range._min + delta_major*i, 0.0, 0.0 );
	glVertex3d( _range._min + delta_major*i, -DIM_MAJOR * scale_y, 0.0 );
      }
    }
    glEnd();
    
    glLineWidth (0.5f);
    glBegin( GL_LINES ); {
      for( unsigned int i = 0; i < _range._major; ++i) {
	for( unsigned int j = 1; j < _range._minor; ++j) {
	  glVertex3d( _range._min + delta_major*i + delta_minor*j, 0.0, 0.0 );
	  glVertex3d( _range._min + delta_major*i + delta_minor*j, -DIM_MAJOR/2.0*scale_y, 0.0 );
	}
      }
    }
    glEnd();

    // Label
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glPushMatrix(); {
      // Label
      glTranslated( _range._max, 2*DIM_MAJOR*scale_y, 0.0);
      glScaled( scale_x, scale_y, 1.0 );
      _font->Render( _title.c_str() );
    } glPopMatrix();
    
    // Major
    for( unsigned int i = 0; i < _range._major+1; ++i) {
      char tick_str[10];
      sprintf( tick_str, "%.3g", _range._min + delta_major*i);
      //std::string tick_str = std::to_string( _range.min + delta_major*i );
      glPushMatrix(); {
	glTranslated( _range._min + delta_major*i, -3*DIM_MAJOR*scale_y, 0.0);
	glScaled( scale_x, scale_y, 1.0 );
	_font->Render( tick_str );
      } glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glPopMatrix(); // POS
  }

  /** get Range */
  Range& get_range() {return _range;}; 
private:
  /** Titre de l'axe */
  std::string _title;
  /** Range pour l'axe */
  Range _range;
  /** Des Fontes pour écrire */
  static FTFont* _font;
};
// ******************************************************************** STATIC
FTFont* Axis::_font = new FTGLTextureFont( FONT_PATH );
// ***************************************************************************
