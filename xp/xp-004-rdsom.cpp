/* -*- coding: utf-8 -*- */

/** 
 * XP with recurrent DSOM.
 *  - load HMM
 *  - load Traj
 *  - create and save DSOM
 *  - load DSOM
 *  - run XP
 *  - TODO what must be saved ??
 *  - TODO graphic
 */

#include <iostream>                // std::cout
#include <iomanip>                 // std::setw
#include <fstream>                 // std::ofstream
#include <string>                  // std::string
#include <sstream>                 // std::stringdtream
#include <rapidjson/document.h>    // rapidjson's DOM-style API
#include <json_wrapper.hpp>        // JSON::IStreamWrapper

#include <hmm-json.hpp>
#include <input.hpp>
#include <hmm_trajectory.hpp>
#include <dsom/r_network.hpp>

#include <figure.hpp>
#include <fixedqueue.hpp>
#include <dsom/rdsom1D_viewer.hpp>

// Parsing command line options
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <utils.hpp>                  // various str_xxx
using namespace utils::rj;

// ******************************************************************** Global
// HMM
using Problem = struct {
  std::string expr = "ABCD";
  bica::hmm::T t;
  bica::hmm::O o;
  unsigned int nb_states;
};
std::unique_ptr<Problem> _pb = nullptr;

// Trajectory
using Traj = Trajectory::HMM::Data;
std::unique_ptr<Traj>    _data = nullptr;

// RDSOM
using RDSOM = Model::DSOM::RNetwork;
std::unique_ptr<RDSOM>     _rdsom;
using TInput = Model::DSOM::RNeuron::TWeight;
using TParam = Model::DSOM::RNeuron::TNumber;
Traj::iterator _ite_step;

// Graphic
Figure*                    _fig_rdsom = nullptr;
FixedQueue<unsigned int>*  _winner_queue = nullptr;
RDSOMViewer*               _rdsom_viewer = nullptr;
Figure*                    _fig_weight = nullptr;
Figure*                    _fig_rweight = nullptr;
Curve* _c_weight;
CurveDyn<RDSOM::Similarities> *_c_sim_input;
Curve* _c_rweight;
CurveDyn<RDSOM::Similarities> *_c_sim_rec;
CurveDyn<RDSOM::Similarities> *_c_sim_merged;
CurveDyn<RDSOM::Similarities> *_c_sim_convol;
CurveDyn<RDSOM::Similarities> *_c_sim_hh_dist;
CurveDyn<RDSOM::Similarities> *_c_sim_hh_rec;
bool _end_render = false;
bool _run_update = false;
unsigned int _nb_step = 0;
unsigned int _learn_length_multiplier = 1;
// ******************************************************************* Options
// Options
std::unique_ptr<std::string> _opt_fileload_hmm       = nullptr;
std::unique_ptr<std::string> _opt_fileload_traj      = nullptr;
int                          _opt_rdsom_size           = 10;
std::unique_ptr<std::string> _opt_filesave_rdsom     = nullptr;
std::unique_ptr<std::string> _opt_fileload_rdsom     = nullptr;
TParam                       _opt_beta               = 0.5;
TParam                       _opt_sig_input          = 0.1;
TParam                       _opt_sig_recur          = 0.1;
TParam                       _opt_sig_convo          = 0.1;
TParam                       _opt_eps                = 0.1;
TParam                       _opt_ela                = 0.2;
TParam                       _opt_ela_rec            = 0.2;
bool                         _opt_graph              = false;
unsigned int                 _opt_queue_size         = 5;
bool                         _opt_verb               = false;

// ******************************************************** forward references
void update_graphic();
void learn( RDSOM& rdsom,
	    const Traj::iterator& input_begin,
	    const Traj::iterator& input_end);
void step_learn( RDSOM& rdsom,
		 const Traj::iterator::difference_type& length,
		 const Traj::iterator& input_begin,
		 const Traj::iterator& input_end);
std::string str_queue();
// ***************************************************************************
// ******************************************************************* options
// ***************************************************************************
void setup_options(int argc, char **argv)
{
  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "produce help message")
	("load_hmm,m", po::value<std::string>(), "load HMM from filename")
	("load_traj,t", po::value<std::string>(), "load Traj from filename")
	("rdsom_size", po::value<int>(&_opt_rdsom_size)->default_value(_opt_rdsom_size),"rdsom size")
	("save_rdsom", po::value<std::string>(), "save RDSOM in filename")
    ("load_rdsom,d", po::value<std::string>(), "load RDSOM from filename")
	("dsom_beta", po::value<TParam>(&_opt_beta)->default_value(_opt_beta), "dsom beta")
	("dsom_sig_i", po::value<TParam>(&_opt_sig_input)->default_value(_opt_sig_input), "dsom sigma input")
	("dsom_sig_r", po::value<TParam>(&_opt_sig_recur)->default_value(_opt_sig_recur), "dsom sigma recurrent")
	("dsom_sig_c", po::value<TParam>(&_opt_sig_convo)->default_value(_opt_sig_convo), "dsom sigma convolution")
	("dsom_eps", po::value<TParam>(&_opt_eps)->default_value(_opt_eps), "dsom epsilon")
	("dsom_ela", po::value<TParam>(&_opt_ela)->default_value(_opt_ela), "dsom elasticity")
        ("dsom_ela_rec", po::value<TParam>(&_opt_ela_rec)->default_value(_opt_ela_rec), "dsom elasticity recurrent")
        ("graph,g", "graphics" )
        ("queue_size", po::value<unsigned int>(&_opt_queue_size)->default_value(_opt_queue_size), "Length of Queue for Graph")
	("verb,v", "verbose" )
	;

  // Options en ligne de commande
  po::options_description cmdline_options;
  cmdline_options.add(desc);
  
  // Options qui sont 'apres'
  po::positional_options_description pod;
  //pod.add( "data_file", 1);

  // Parse
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).
	      options(desc).positional(pod).run(), vm);
    
    if (vm.count("help")) {
      std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
      std::cout << desc << std::endl;
      exit(1);
    }
    
    po::notify(vm);
  }
  catch(po::error& e)  { 
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << desc << std::endl; 
    exit(2);
  }

  // HMM
  if (vm.count("load_hmm")) {
    _opt_fileload_hmm = make_unique<std::string>(vm["load_hmm"].as< std::string>());
  }
   // TRAJ
  if (vm.count("load_traj")) {
    _opt_fileload_traj = make_unique<std::string>(vm["load_traj"].as< std::string>());
  }
  // RDSOM
  if (vm.count("save_rdsom")) {
    _opt_filesave_rdsom = make_unique<std::string>(vm["save_rdsom"].as< std::string>());
  }
  if (vm.count("load_rdsom")) {
    _opt_fileload_rdsom = make_unique<std::string>(vm["load_rdsom"].as< std::string>());
  }

  // Options
  if( vm.count("graph") ) {
    _opt_graph = true;
  }
  if( vm.count("verb") ) {
    _opt_verb = true;
  }
}
// ***************************************************************************
// ********************************************************* Graphic Callbacks
// ***************************************************************************
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
  // else if (key == GLFW_KEY_N && action == GLFW_PRESS) {
  //   if( _opt_fileload_traj and _opt_fileload_rdsom ) {
  //     if( _opt_verb ) {
  // 	std::cout << "__LEARN" << std::endl;
  //     }
  //     learn( *_rdsom, _data->begin(), _data->end() );
  //     _nb_step += _data->size();

  //     update_graphic();
  //   }
  // }
  // Run 'n' learning steps
  else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    if( _opt_fileload_traj and _opt_fileload_rdsom ) {
      // if( _opt_verb ) {
      // 	std::cout << "__STEP" << std::endl;
      // }
      auto learn_length = _opt_queue_size * _learn_length_multiplier;
      if( learn_length == 0 ) learn_length = 1;
      step_learn( *_rdsom, learn_length,
		  _data->begin(), _data->end() );
      
      update_graphic();
    }
  }
  // increase / decrease learning length P/M
  else if( key == GLFW_KEY_P && action == GLFW_PRESS) {
    if( _learn_length_multiplier == 0 ) _learn_length_multiplier = 1;
    _learn_length_multiplier = _learn_length_multiplier * 10;
  }
  else if( key == GLFW_KEY_SEMICOLON && action == GLFW_PRESS) {
    _learn_length_multiplier = _learn_length_multiplier / 10;
  }
  // On/off continuous learning SPACE
  else if( key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    _run_update = !_run_update;
  }
  // On/Off verbose V
    else if( key == GLFW_KEY_V && action == GLFW_PRESS) {
    _opt_verb = !_opt_verb;
  }
}
// ************************************************************ update_graphic
void update_graphic()
{
  // update data
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
  
  _fig_rdsom->clear_text();
  std::stringstream str;
  auto learn_length = _opt_queue_size * _learn_length_multiplier;
  if( learn_length == 0 ) learn_length = 1;
  str << "Step=" << _nb_step << " (x" << learn_length << ")";
  _fig_rdsom->add_text( str.str(), 0.7, 0.1);
}
// ***************************************************************** str_queue
std::string str_queue()
{
  std::stringstream ss;
  ss << "__QUEUE" << std::endl;
  for (auto it = _winner_queue->begin(); it != _winner_queue->end(); ++it) {
    ss << "  " << _rdsom->v_neur[*it]->str_display() << std::endl;
  }
  return ss.str();
}
// ****************************************************************** load_hmm
Problem create_hmm( const std::string& expr = "ABCD" )
{
  Problem pb;
  pb.expr = expr;
  auto hmm = bica::hmm::make(expr);
  std::tie(pb.t, pb.o) = hmm.first;
  pb.nb_states = hmm.second;

  return pb;
}Problem load_hmm( const std::string& filename )
{
  auto pfile = std::ifstream( filename );

  // Wrapper pour lire document
  JSON::IStreamWrapper instream(pfile);
  // Parse into a document
  rj::Document read_doc;
  read_doc.ParseStream( instream );
  pfile.close();

  auto expr = bica::hmm::unserialize( read_doc["hmm"] );
  return create_hmm( expr );
}
// ***************************************************************** load_traj
Traj load_traj( const std::string& filename )
{
  Traj traj;
  auto pfile = std::ifstream( filename );
  Trajectory::HMM::read( pfile, traj );
  pfile.close();

  return traj;
}
// ************************************************************** create_rdsom
RDSOM create_rdsom(int input_dim=1, int rdsom_size=10, int rdsom_nb_link=-1,
				   float w_min=0.0, float w_max=1.0 )
{
  RDSOM rdsom = RDSOM( input_dim, rdsom_size, rdsom_nb_link,
				  w_min, w_max );
  return rdsom;
}
// **************************************************************** save_rdsom
void save_rdsom( const std::string& filename, const RDSOM& rdsom )
{
  auto ofile = std::ofstream( filename );

  rapidjson::Document doc;
  rapidjson::Value obj = _rdsom->serialize( doc );
  //std::cout << str_obj( obj ) << std::endl;
  // Write in a file
  ofile << str_obj( obj ) << std::endl;
  ofile.close();
}
RDSOM load_rdsom( const std::string& filename )
{
  std::ifstream ifile( filename );
  RDSOM net_read( ifile );
  ifile.close();

  return net_read;
}

// ***************************************************************************
// ********************************************************************* learn
// ***************************************************************************
void learn( RDSOM& rdsom,
			const Traj::iterator& input_begin,
			const Traj::iterator& input_end)
{
  for( auto it = input_begin; it != input_end; ++it ) {
	// Forward new input and update network
	Eigen::VectorXd input(1);
	input << (double) it->id_o;
	if( _opt_verb ) {
	  std::cout << "  in=" << input << std::endl;
	}
	rdsom.forward( input, _opt_beta,
				   _opt_sig_input, _opt_sig_recur, _opt_sig_convo,
				   _opt_verb);
	rdsom.deltaW( input, _opt_eps, _opt_ela, _opt_ela_rec, _opt_verb);

	if( _winner_queue ) {
	  _winner_queue->push_front( rdsom.get_winner() );
	}
  }
}
void step_learn( RDSOM& rdsom,
		 const Traj::iterator::difference_type& length,
		 const Traj::iterator& input_start,
		 const Traj::iterator& input_end)
{
  for( unsigned int i = 0; i < length; ++i) {
    // Forward new input and update network
    Eigen::VectorXd input(1);
    input << (double) _ite_step->id_o;
    // if( _opt_verb ) {
    //   std::cout << "  in=" << input << std::endl;g
    // }
    rdsom.forward( input, _opt_beta,
		   _opt_sig_input, _opt_sig_recur, _opt_sig_convo,
		   _opt_verb);
    rdsom.deltaW( input, _opt_eps, _opt_ela, _opt_ela_rec, _opt_verb);
    
    if( _winner_queue ) {
      _winner_queue->push_front( rdsom.get_winner() );
    }

    // update iterator
    ++ _ite_step;
    if( _ite_step == input_end ) {
      _ite_step = input_start;
    }
    // update nb step
    ++ _nb_step;

    if( _opt_verb ) {
      std::cout << str_queue() << std::endl;
    }
  }
}
// ***************************************************************************
// ********************************************************************** main
// ***************************************************************************
int main(int argc, char *argv[])
{
   setup_options( argc, argv );
   
   // HMM _______________________
   if( _opt_fileload_hmm ) {
    if( _opt_verb )
      std::cout << "__LOAD HMM from " << *_opt_fileload_hmm << std::endl;
     _pb = make_unique<Problem>( load_hmm( *_opt_fileload_hmm ));
     bica::sampler::HMM hmm1(_pb->t,_pb->o);
     if( _opt_verb )
       std::cout << "__" << _pb->expr << "__ with " << _pb->nb_states << " states" << std::endl;
   }
   // Traj_______________________
   if( _opt_fileload_traj ) {
     if( _opt_verb )
       std::cout << "__LOAD Traj from " << *_opt_fileload_traj << std::endl;
     _data = make_unique<Traj>( load_traj( *_opt_fileload_traj ) );
     _ite_step = _data->begin();
     _nb_step = 0;
     if( _opt_verb )
       std::cout << "  data read" << std::endl;
   }
   // RDSOM _____________________
   if( _opt_filesave_rdsom ) {
     if( _opt_verb )
       std::cout << "__CREATE RDSOM " << std::endl;
     _rdsom = make_unique<RDSOM>( create_rdsom(1, _opt_rdsom_size, -1,
											 0.0, 1.0 ));
	 if( _opt_verb )
       std::cout << "__SAVE RDSOM to " << *_opt_filesave_rdsom << std::endl;
     save_rdsom( *_opt_filesave_rdsom, *_rdsom);
   }
   if( _opt_fileload_rdsom ) {
     if( _opt_verb )
       std::cout << "__LOAD RDSOM from " << *_opt_fileload_rdsom << std::endl;
     _rdsom = make_unique<RDSOM>( load_rdsom( *_opt_fileload_rdsom ));
     
     // test
     if( _opt_verb )
       std::cout << _rdsom->str_dump() << std::endl;
   }
   // Graphic before Learning (as Learn will depend on it)
   if( _opt_graph ) {
     if( _opt_verb) {
       std::cout << "__INIT GRAPHIC" << std::endl;
     }
     _fig_rdsom = new Figure( "RDSOM: r-network", 450, 450, 340, 0 );
     _winner_queue = new FixedQueue<unsigned int>( _opt_queue_size);
     _rdsom_viewer = new RDSOMViewer( *_rdsom, *_winner_queue );
     _fig_rdsom->add_curve( _rdsom_viewer );
     _fig_rdsom->set_draw_axes( false );

     _fig_weight = new Figure( "Input/Weights", 800, 350, 800, 50,
			       {0.0, (double) _rdsom->v_neur.size(),10,2},
			       {0.0, 1.0, 10, 2} );
     // Weights
     _c_weight = new Curve();
     _c_weight->set_color( {0.0, 0.0, 0.0} );
     _c_weight->set_width( 3 );
     _fig_weight->add_curve( _c_weight );
     // Input Similarities
     _c_sim_input = new CurveDyn<RDSOM::Similarities>( _rdsom->_sim_w );
     _c_sim_input->set_color( {1.0, 0.0, 0.0} );
     _fig_weight->add_curve( _c_sim_input );
     // Merged Similarities
     _c_sim_merged = new CurveDyn<RDSOM::Similarities>( _rdsom->_sim_merged );
     _c_sim_merged->set_color( {0.0, 0.0, 1.0} );
     _c_sim_convol = new CurveDyn<RDSOM::Similarities>( _rdsom->_sim_convol );
     _c_sim_convol->set_color( {0.0, 0.0, 1.0} );
     _c_sim_convol->set_width( 2 );
     _fig_weight->add_curve( _c_sim_merged );
     _fig_weight->add_curve( _c_sim_convol );
     _c_sim_hh_dist = new CurveDyn<RDSOM::Similarities>( _rdsom->_sim_hn_dist );
     _c_sim_hh_dist->set_color( {0.0, 1.0, 0.0} );
     _c_sim_hh_dist->set_width( 3 );
     _fig_weight->add_curve( _c_sim_hh_dist );
  
     _fig_rweight = new Figure( "Recurrent/RWeights", 800, 350, 800, 430,
				{0.0, (double) _rdsom->v_neur.size(),10,2},
				{0.0, 1.0, 10, 2} );
     _c_rweight = new Curve();
     _c_rweight->set_color( {0.0, 0.0, 0.0} );
     _c_rweight->set_width( 3 );
     // Rec Similarities
     _c_sim_rec = new CurveDyn<RDSOM::Similarities>( _rdsom->_sim_rec );
     _c_sim_rec->set_color( {1.0, 0.0, 0.0} );
     _fig_rweight->add_curve( _c_sim_rec );
     _fig_rweight->add_curve( _c_sim_merged );
     _fig_rweight->add_curve( _c_sim_convol );
     _c_sim_hh_rec = new CurveDyn<RDSOM::Similarities>( _rdsom->_sim_hn_rec );
     _c_sim_hh_rec->set_color( {0.0, 1.0, 0.0} );
     _c_sim_hh_rec->set_width( 3 );
     _fig_rweight->add_curve( _c_sim_hh_rec );

     _fig_rweight->add_curve( _c_rweight );
   
     _fig_rdsom->render( true );
     _fig_weight->render();
     _fig_rweight->render();

     while( not _end_render ) {
       // update if _run_updata
       if( _run_update ) {
	 auto learn_length = _opt_queue_size * _learn_length_multiplier;
	 if( learn_length == 0 ) learn_length = 1;
	 step_learn( *_rdsom, learn_length,
		     _data->begin(), _data->end() );

	 update_graphic();
       }       
       _fig_rdsom->render();
       _fig_weight->render();
       _fig_rweight->render();
     }
   }
   // Learn_________________________
  if( _opt_fileload_traj and _opt_fileload_rdsom ) {
    if( _opt_verb ) {
      std::cout << "__LEARN" << std::endl;
    }
    learn( *_rdsom, _data->begin(), _data->end() );

	// Save learned RDSOM
	// Some kind of criteria
  }
   return 0;
}

