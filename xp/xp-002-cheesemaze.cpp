/* -*- coding: utf-8 -*- */

/** 
 * Generate Cheeze-Maze POMDP.
 */

#include <pomdp/pomdp.hpp>

#include <iostream>                // std::cout
#include <fstream>                 // std::ofstream
#include <string>                  // std::string
#include "rapidjson/document.h"         // rapidjson's DOM-style API

// Parsing command line options
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <utils.hpp>                  // various str_xxx
using namespace utils::rj;

// ******************************************************************** global
#define NB_ACTIONS 4

// ******************************************************************* options
double _proba;
unsigned int _length;
std::string*           _filename = nullptr;
void setup_options(int argc, char **argv)
{
  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "produce help message")
    ("prob,p", po::value<double>(&_proba)->default_value(1.0), "Proba of successful transition ")
    ("length,l", po::value<unsigned int>(&_length)->default_value(1), "Corridor length") 
    ("file,f",  po::value<std::string>(), "Save into that file")
    ;

  // Options en ligne de commande
  po::options_description cmdline_options;
  cmdline_options.add(desc);

  // Options qui sont 'après'
  po::positional_options_description pod;
  pod.add( "file", 1);

  // Parse
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).
	    options(desc).positional(pod).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
      std::cout << desc << std::endl;
    exit(1);
  }

  // file names
  if (vm.count("file")) {
    _filename = new std::string(vm["file"].as< std::string>());
  }
}
// ********************************************************** make_cheese_maze
Model::POMDP make_cheese_maze( const double prob_success = 1.0,
		       const unsigned int corridor_length = 1 )
{
  // Perceptions
  std::vector<Model::Transition> perc;
  Model::Transition t_perc;

  // Actions : UP, RIGHT, DOWN, LEFT
  // Encoder la structure du graphe
  std::vector<std::vector<unsigned int>> graph;
  std::vector<unsigned int> t_graph;
  // Etat 0
  unsigned int state = 0;
  graph.push_back( {0, (2+corridor_length), 1, 0} );
  t_perc._proba = {1.0, 0, 0, 0, 0, 0};
  perc.push_back( t_perc );
  // // First corridor
  for( unsigned int i = 0; i < corridor_length; ++i) {
    state++;
    graph.push_back( {state-1, state, state+1, state} );
    t_perc._proba = {0, 1.0, 0, 0, 0, 0};
    perc.push_back( t_perc );
  }
  // // End first corridor
  state++;
  graph.push_back( {state-1, state, state, state} );
  t_perc._proba =  {0, 0, 1.0, 0, 0, 0};
  perc.push_back( t_perc );
  // // 1st case right
  state++;
  graph.push_back( {state, state+1, state, state-(2+corridor_length)} ); 
  t_perc._proba = {0, 0, 0, 1.0, 0, 0};
  perc.push_back( t_perc );
  // // 2nd case right
  state++;
  graph.push_back( {state, state+(2+corridor_length), state+1, state-1} );
  t_perc._proba = {0, 0, 0, 0, 1.0, 0};
  perc.push_back( t_perc );
  // // Second corridor
  for( unsigned int i = 0; i < corridor_length; ++i) {
    state++;
    graph.push_back( {state-1, state, state+1, state} );
    t_perc._proba = {0, 1.0, 0, 0, 0, 0};
    perc.push_back( t_perc );
  }
  // // End of second corridor
  state++;
  graph.push_back( {state-1, state, state, state} );
  t_perc._proba = {0, 0, 1.0, 0, 0, 0};
  perc.push_back( t_perc );
  // // 3rd case right
  state++;
  graph.push_back( {state, state+1, state, state-(2+corridor_length)} );
  t_perc._proba = {0, 0, 0, 0, 1.0, 0};
  perc.push_back( t_perc );
  // // Last case right
  state++;
  graph.push_back( {state, state, state+1, state-1} );
  t_perc._proba = {0, 0, 0, 0, 0, 1.0};
  perc.push_back( t_perc );
  // // last corridor
  for( unsigned int i = 0; i < corridor_length; ++i) {
    state++;
    graph.push_back( {state-1, state, state+1, state} );
    t_perc._proba = {0, 1.0, 0, 0, 0, 0};
    perc.push_back( t_perc );
  }
  // // Last corridor end
  state++;
  graph.push_back( {state-1, state, state, state} );
  t_perc._proba = {0, 0, 1.0, 0, 0, 0};
  perc.push_back( t_perc );

  // nombre d'états
  unsigned int nb_states =  8 + 3 * corridor_length;
  std::vector<Model::Node> states;
  for( unsigned int i = 0; i < nb_states; ++i) {
    states.push_back( {i, std::to_string(i)} );
  }
  // transitions
  Model::Transition trans;
  // Vecteur initial tout à 0
  for( unsigned int s = 0; s < nb_states; ++s) {
    trans._proba.push_back( 0.0 );
  }
  // All the transitions
  std::vector<std::vector<Model::Transition>> all_trans;
  for( unsigned int s = 0; s < nb_states; ++s) {
    std::vector<Model::Transition> trans_s;
    for( unsigned int a = 0; a < NB_ACTIONS; ++a) {
      // init trans to 0.0...
      std::fill( trans._proba.begin(), trans._proba.end(), 0.0 );

      // moving in that direction
      trans._proba[graph[s][a]] += prob_success;
      
      // // Every other direction
      for( unsigned int d = 0; d < 4; ++d) {
       	if( d != a ) { 
      	  trans._proba[graph[s][d]] += (1.f - prob_success)/3.f;
       	}
      }
      trans_s.push_back( trans );
    }
    all_trans.push_back( trans_s );
  }

  // Reward
  std::vector<double> reward(nb_states);
  std::fill( reward.begin(), reward.end(), -1.0 );
  reward[nb_states/2 + 1] = 10.0f;

  //Model::POMDP pomdp;
  Model::POMDP pomdp = { states,
  			 {{0,"LC"},{1,"||"},{2,"U"},{3,"="},{4,"-"},{5,"RC"}},
  			 {{0,"Up"},{1,"Ri"},{2,"Do"},{3,"Le"}},
  			 all_trans,
  			 perc,
  			 reward };

  return pomdp;
}

//******************************************************************************
int main( int argc, char *argv[] )
{
  setup_options( argc, argv );
  
  Model::POMDP pomdp = make_cheese_maze( _proba, _length);
  std::cout << pomdp.str_dump() << std::endl;

  if( _filename ) {
    std::string fullname = *_filename + ".json";
    std::cout << "*** write to file : " << fullname << std::endl;

    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember( "pomdp", pomdp.serialize(doc), doc.GetAllocator());

    std::ofstream ofile( fullname );
    ofile << str_obj(doc) << std::endl;
    ofile.close();
  }
  return 0;
}
