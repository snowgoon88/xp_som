/* -*- coding: utf-8 -*- */

/**
 * test-008-multifig.cpp
 * 
 * Multi-windows with adaptive DynCurves ?
 */

#include <window.hpp>
#include <figure.hpp>
#include <curve.hpp>
#include <axis.hpp>


#include <GLFW/glfw3.h>
#include <string>
#include <stdlib.h>
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <chrono>         //std::chrono
#include <memory>
#include <algorithm>      // std::max


#include <dsom/r_network.hpp>
using namespace Model::DSOM;
RNetwork* _rdsom;
Window* _w_weight;
Figure* _f_weight;
Window* _w_rweight;
Figure* _f_rweight;
Curve* _c_weight;
CurveDyn<RNetwork::Similarities> *_c_sim_input;
Curve* _c_rweight;
CurveDyn<RNetwork::Similarities> *_c_sim_rec;
CurveDyn<RNetwork::Similarities> *_c_sim_merged;
CurveDyn<RNetwork::Similarities> *_c_sim_convol;
CurveDyn<RNetwork::Similarities> *_c_sim_hh_dist;
CurveDyn<RNetwork::Similarities> *_c_sim_hh_rec;
bool _end_render = false;

// ************************************************************** declarations
void step_data();
// ***************************************************************************
// ***************************************************************************
// ***************************************************************** Callbacks
/**
 * Callback pour gérer les messages d'erreur de GLFW
 */
static void error_callback(int error, const char* description)
{
  std::cerr <<  description << std::endl;
  //fputs(description, stderr);
}
/**
 * Callback qui gère les événements 'key'
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
	_end_render = true;
  }
  else if (key == GLFW_KEY_N && action == GLFW_PRESS) {
	step_data();
  }
}
// ***************************************************************************
// ***************************************************************************
// ********************************************************************** Data
void init_data()
{
  // Creation of network
  _rdsom = new RNetwork( 1, 100, -1 );
  std::cout << "__CREATION" << std::endl << _rdsom->str_dump() << std::endl;

  _w_weight = new Window( "Input/Weights", 800, 600, false, -1, -1 );
  _f_weight = new Figure( *_w_weight, "Input/weights", {0.0,100.0,10,2}, {0.0, 1.0, 10, 2} );
  _w_weight->add_plotter( _f_weight );
  
  // Weights
  _c_weight = new Curve();
  _c_weight->set_color( {0.0, 0.0, 0.0} );
  _c_weight->set_width( 3 );
  _f_weight->add_plotter( _c_weight );
  // Input Similarities
  _c_sim_input = new CurveDyn<RNetwork::Similarities>( _rdsom->_sim_w );
  _c_sim_input->set_color( {1.0, 0.0, 0.0} );
  _f_weight->add_plotter( _c_sim_input );
  // Merged Similarities
  _c_sim_merged = new CurveDyn<RNetwork::Similarities>( _rdsom->_sim_merged );
  _c_sim_merged->set_color( {0.0, 0.0, 1.0} );
  _c_sim_convol = new CurveDyn<RNetwork::Similarities>( _rdsom->_sim_convol );
  _c_sim_convol->set_color( {0.0, 0.0, 1.0} );
  _c_sim_convol->set_width( 2 );
  _f_weight->add_plotter( _c_sim_merged );
  _f_weight->add_plotter( _c_sim_convol );
  _c_sim_hh_dist = new CurveDyn<RNetwork::Similarities>( _rdsom->_sim_hn_dist );
  _c_sim_hh_dist->set_color( {0.0, 1.0, 0.0} );
  _c_sim_hh_dist->set_width( 3 );
  _f_weight->add_plotter( _c_sim_hh_dist );
  _w_weight->update_bbox();

  
  _w_rweight = new Window( "Recurrent/RWeights", 800, 600, false, -1, -1 );
  _f_rweight = new Figure( *_w_rweight, "Recurrent/RWeights",
                           {0.0,100.0,10,2}, {0.0, 1.0, 10, 2} );
  _w_rweight->add_plotter( _f_rweight );
  
  _c_rweight = new Curve();
  _c_rweight->set_color( {0.0, 0.0, 0.0} );
  _c_rweight->set_width( 3 );
  // Rec Similarities
  _c_sim_rec = new CurveDyn<RNetwork::Similarities>( _rdsom->_sim_rec );
  _c_sim_rec->set_color( {1.0, 0.0, 0.0} );
  _f_rweight->add_plotter( _c_sim_rec );
  _f_rweight->add_plotter( _c_sim_merged );
  _f_rweight->add_plotter( _c_sim_convol );
  _c_sim_hh_rec = new CurveDyn<RNetwork::Similarities>( _rdsom->_sim_hn_rec );
  _c_sim_hh_rec->set_color( {0.0, 1.0, 0.0} );
  _c_sim_hh_rec->set_width( 3 );
  _f_rweight->add_plotter( _c_sim_hh_rec );

  _f_rweight->add_plotter( _c_rweight );
  _w_rweight->update_bbox();
}
void step_data()
{
  // one input
  Eigen::VectorXd i1 = Eigen::VectorXd::Random(1);
  i1 = (i1.array() - -1.0) / (1.0 - -1.0);
	// Eigen::VectorXd i1(1);
	// i1 << (double) (_nb_iter % 3) / 4.0 + 0.1;
	
  _rdsom->forward( i1, 0.5, 0.1, 0.1, 0.1 );
  _rdsom->deltaW( i1, 0.1, 0.2, true );
}
void update_data()
{
  _c_weight->clear();
  _c_rweight->clear();
  for( unsigned int i = 0; i < _rdsom->v_neur.size(); ++i) {
	_c_weight->add_sample( {(double)i, _rdsom->v_neur[i]->weights(0), 0.0} );
	_c_rweight->add_sample( {(double)i, _rdsom->v_neur[i]->r_weights(0), 0.0} ); 
  }
  _c_sim_input->update();
  _c_sim_rec->update();
  _c_sim_merged->update();
  _c_sim_convol->update();
  _c_sim_hh_dist->update();
  _c_sim_hh_rec->update();

  //_w_weight->update_bbox();
}
void end_data()
{
  delete _c_weight;
  delete _c_sim_input;
  delete _c_sim_merged;
  delete _c_sim_convol;
  delete _f_weight;
  delete _w_weight;

  delete _c_rweight;
  delete _c_sim_rec;
  delete _f_rweight;
  delete _w_rweight;
}

// ***************************************************************************
// ***************************************************************************
// ****************************************************************** GLFWInit
/**
 * Init GLFW
*/
void init_glfw()
{
  std::cout << "__GLFW Init" << std::endl;
  
  glfwSetErrorCallback(error_callback);
  
  if (!glfwInit())
	exit(EXIT_FAILURE);
}
// ******************************************************************** Render
void render() {
  while( not _end_render ) {
	update_data();
	_w_weight->render();
	_w_rweight->render();
  }
}
void glfw_end()
{
  glfwTerminate();
  std::cout << "__GLFW destroyed" << std::endl;
}

//******************************************************************************
// ********************************************************************** Main
int main( int argc, char *argv[] )
{
  init_glfw();

  init_data();
  update_data();

  render();

  end_data();
  glfw_end();

  return 0;
}

