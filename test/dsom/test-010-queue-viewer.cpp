/* -*- coding: utf-8 -*- */

/** 
 * Test viewer of 2 queues as
 * - idx of winning neuron vs iteration
 * - ideal idx according to input ([0,1] scaled according to idx_max
 */

#include <figure.hpp>
#include <curve.hpp>
#include <axis.hpp>

#include <dsom/r_network.hpp>
#include <fixedqueue.hpp>


unsigned int data_win[] = { 3,2,3,2,14,2,3,4,5,14,3,2,3,3,13, 2, 2, 2, 4, 15 };
float data_in[] = {0.1, 0.1, 0.1, 0.1, 0.9, 0.1, 0.1, 0.1, 0.1, 0.9, 0.1, 0.1, 0.1, 0.1, 0.9, 0.1, 0.1, 0.1, 0.1, 0.9};
unsigned int idx = 0;
FixedQueue<double> _win_queue(15);
FixedQueue<double> _truth_queue(15);
CurveDyn<FixedQueue<double>> c_win( _win_queue );
CurveDyn<FixedQueue<double>> c_inp( _truth_queue );

// ***************************************************************************
// ***************************************************************************
// ************************************************ callbacks needed by Figure
bool _end_render = false;
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
  // ESC -> stop rendering
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
	_end_render = true;
  }
  // N -> next
  else if (key == GLFW_KEY_N && action == GLFW_PRESS) {
    ++idx;
    if( idx == 20 ) idx = 0;
    //update queues
    _win_queue.push_front( static_cast<double>(data_win[idx]) );
    _truth_queue.push_front( static_cast<double>(data_in[idx] * 16) );
    // and curves
    c_win.update();
    c_inp.update();
  }
}

int main(int argc, char *argv[])
{
  // Data - only container with double are accepted by CurveDyn.
  for( unsigned int i = 0; i < 15; ++i) {
    _win_queue.push_front( static_cast<double>(data_win[i]) );
    _truth_queue.push_front( static_cast<double>(data_in[i] * 16) );
    ++idx;
  }
  c_win.update();
  c_inp.update();

  // Create a Figure
  Figure fig = Figure( "QueueViewer", 800, 600, false /* offscreen */,
		       -1, -1,
		       {0.0, 20.0, 10, 2}, {0.0, 20.0, 10, 2});

  // Curve
  fig.add_curve( &c_win );
  c_inp.set_color( {0.0, 1.0, 0.0} );
  fig.add_curve( &c_inp );

  while(not _end_render) {
    fig.render();
  }
  
  return 0;
}
