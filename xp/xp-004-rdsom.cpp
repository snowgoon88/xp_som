/* -*- coding: utf-8 -*- */

/** 
 * XP with recurrent DSOM.
 *  - load HMM
 *  - load Traj
 *  - create and save DSOM
 *  - load DSOM
 *  - run XP
 *  - what must be saved ??
 *  - graphic
 *  - batch learning
 *  - batch testing with rdsom and traj
 *
 * INTERFACE
 *  - ESC : end
 *  - s : one step
 *  - p/m : +/- learning length
 *  - SPACE : switch running
 *  - v : switch verbose
 *  - z : switch mean error
 *  - w : force mean update if in mean error mode
 *  - f : save figweight, 
 */

#include <iostream>                // std::cout
#include <iomanip>                 // std::setw
#include <fstream>                 // std::ofstream
#include <string>                  // std::string
#include <sstream>                 // std::stringdtream
#include <rapidjson/document.h>    // rapidjson's DOM-style API
#include <json_wrapper.hpp>        // JSON::IStreamWrapper
#include <chrono>
#include <thread>

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

// Analyse of sequence frequency
using Sequence = std::vector<unsigned int>;
using SequenceMap = std::map<Sequence, int>;
using SequencePair = std::pair<Sequence, int>;
using SequenceVec = std::vector<SequencePair>;
SequenceMap _seqmap;
SequenceMap _seqmap_learn;

// ******************************************************************** Global
// HMM
using Problem = struct {
  std::string expr = "ABCD";
  bica::hmm::T t;
  bica::hmm::O o;
  unsigned int nb_states;
};
std::unique_ptr<Problem> _pb = nullptr;

// Iteration counter
unsigned int _ite_cur = 0;

// Trajectory
using Traj = Trajectory::HMM::Data;
std::unique_ptr<Traj>    _data = nullptr;

// RDSOM
using RDSOM = Model::DSOM::RNetwork;
std::unique_ptr<RDSOM>     _rdsom;
using TInput = Model::DSOM::RNeuron::TWeight;
using TParam = Model::DSOM::RNeuron::TNumber;
Traj::iterator _ite_step;

// Log some errors
std::vector<double> _v_err_input, _v_err_rec, _v_err_pred;     

// Graphic
Figure*                    _fig_rdsom = nullptr;
FixedQueue<unsigned int>*  _winner_queue = nullptr;
RDSOMViewer*               _rdsom_viewer = nullptr;
Figure*                    _fig_weight = nullptr;
Figure*                    _fig_rweight = nullptr;
Figure*                    _fig_error = nullptr;
Curve* _c_weight;
CurveDyn<RDSOM::Similarities> *_c_sim_input;
Curve* _c_rweight;
CurveDyn<RDSOM::Similarities> *_c_sim_rec;
CurveDyn<RDSOM::Similarities> *_c_sim_merged;
CurveDyn<RDSOM::Similarities> *_c_sim_convol;
CurveDyn<RDSOM::Similarities> *_c_sim_hh_dist;
CurveDyn<RDSOM::Similarities> *_c_sim_hh_rec;
CurveMean* _c_error_input;
CurveMean* _c_error_rec;
CurveMean* _c_error_pred;
// Curve for history of activation
FixedQueue<double>       *_act_queue;
FixedQueue<double>       *_input_queue;
Figure                    *_fig_hist;
CurveDyn<FixedQueue<double>> *_c_hist_act;
CurveDyn<FixedQueue<double>> *_c_hist_input;
// flags
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
bool                         _opt_figerror           = false;
unsigned int                 _opt_queue_size         = 5;
unsigned int                 _opt_hist_size          = 50;
std::unique_ptr<std::string> _opt_filesave_result    = nullptr;
unsigned int                 _opt_learn_length       = 100;
unsigned int                 _opt_period_save        = 50;
bool                         _opt_test               = false;
unsigned int                 _opt_nb_test            = 2;
bool                         _opt_verb               = false;
double                       _opt_seqlog_threshold   = 0.9;
unsigned int                 _opt_seqlog_size        = 6;
unsigned int                 _opt_seqlog_nb          = 10;

// ******************************************************** forward references
void save_figseq( const std::string& filename,
                  const std::string& title,
                  bool verb);
void save_figweight( const std::string& filename,
                     const std::string& title,
                     bool verb);
void save_figerror( const std::string& filename,
                    const std::string& title,
                    bool verb);
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
    ("figerror", "fig with errors at end")
        ("queue_size", po::value<unsigned int>(&_opt_queue_size)->default_value(_opt_queue_size), "Length of Queue for Graph")
    ("history_size", po::value<unsigned int>(&_opt_hist_size)->default_value(_opt_hist_size), "Length of Queue for History")
    ("save_result", po::value<std::string>(), "save RESULTS in filename")
    ("learn_length", po::value<unsigned int>(&_opt_learn_length)->default_value(_opt_learn_length), "Learning Length")
    ("period_save", po::value<unsigned int>(&_opt_period_save)->default_value(_opt_period_save), "Saving Period")
	("testing", "Testing Mode")
	("nb_test", po::value<unsigned int>(&_opt_nb_test)->default_value(_opt_nb_test), "Nomber of test-run")
    ("seqlog_threshold", po::value<double>(&_opt_seqlog_threshold)->default_value(_opt_seqlog_threshold), "Seqlog, input threshold for activation")
    ("seqlog_size", po::value<unsigned int>(&_opt_seqlog_size)->default_value(_opt_seqlog_size), "Seqlog, size of pattern")
    ("seqlog_nb", po::value<unsigned>(&_opt_seqlog_nb)->default_value(_opt_seqlog_nb), "Seqlog, display nb mist frequent")
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
  // RESULTS
  if (vm.count("save_result")) {
    _opt_filesave_result = make_unique<std::string>(vm["save_result"].as< std::string>());
  }

  // Options
  if( vm.count("graph") ) {
    _opt_graph = true;
  }
  if( vm.count("figerror") ) {
    _opt_figerror = true;
  }
  
  if( vm.count("testing") ) {
    _opt_test = true;
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
      _ite_cur += learn_length;
      
      update_graphic();
    }
  }
  // increase / decrease learning length P/M
  else if( key == GLFW_KEY_P && action == GLFW_PRESS) {
    if( _learn_length_multiplier == 0 ) {
      _learn_length_multiplier = 1;
    }
    else {
    _learn_length_multiplier = _learn_length_multiplier * 10;
    }
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
  // On/Off Mean mode for error Z
  else if( key == GLFW_KEY_W && action == GLFW_PRESS) {
    // toggle _mean
	if( _c_error_input->get_mean_mode() ) {
		std::cout << "  Normal mode" << std::endl;
		_c_error_input->set_mean_mode( false );
		_c_error_rec->set_mean_mode( false );
		_c_error_pred->set_mean_mode( false );
	}
	else {
	  std::cout << "  MEAN mode" << std::endl;
	  _c_error_input->set_mean_mode( true );
	  _c_error_input->recompute_means();
	  _c_error_rec->set_mean_mode( true );
	  _c_error_rec->recompute_means();
	  _c_error_pred->set_mean_mode( true );
	  _c_error_pred->recompute_means();
	}
  }
  // Force mean update if in mean_mode
  else if( key == GLFW_KEY_W && action == GLFW_PRESS) {
	if( _c_error_input->get_mean_mode() ) {
	  _c_error_input->recompute_means();
	  _c_error_rec->recompute_means();
	  _c_error_pred->recompute_means();
	}
  }
  else if( key == GLFW_KEY_F && action == GLFW_PRESS) {
    std::stringstream count;
    count << std::setw(6) << std::setfill('0') <<  _ite_cur << ".png";
    
    save_figseq( "key_figseq"+count.str(), "KEY figseq", false);
    save_figweight( "key_figweight"+count.str(), "KEY figweight", false);
    save_figerror( "key_figerror"+count.str(), "KEY figerror", false);
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

  _c_hist_act->update();
  _c_hist_input->update();
  
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
}
Problem load_hmm( const std::string& filename )
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
// ******************************************************************** SeqLog
// ***************************************************************************
/**
 * Save the '_opt_seqlog_nb' most frequent sequences.
 * Format: id id id ... -> nb
 * in filename
 */
void save_seqlog( const std::string& filename, const SequenceMap& smap )
{
  auto ofile = std::ofstream( filename );
  std::cout << "__SEQLOG save" << filename << std::endl;

  // sort SequencePair and print most frequent
  SequenceVec seqvec;
  // Build from SequencePairs of seqencemap
  for (auto it = smap.begin(); it != smap.end(); ++it) {
    seqvec.push_back(*it);
  }
  // sort using pairs->second (lambda function)
  std::sort( seqvec.begin(), seqvec.end(),
	     [] (SequencePair& a, SequencePair& b) {
	       return a.second > b.second;
	     }
	     );
  for (auto it = seqvec.begin();
	 it != std::min(seqvec.end(), seqvec.begin()+_opt_seqlog_nb);
	 ++it) {
      ofile << utils::str_vec(it->first) << " -> " << it->second << std::endl;
    }
  ofile.close();
}

// ***************************************************************************
// *************************************************************** save_figseq
// ***************************************************************************
/**
 * Save a PNJ image of the last _opt_queue_size neurons, using circles and
 * arrows on a nearly cirle for linear position of neurons.
 *
 * GLOBAL : _rdsom, _winner_queue
 */
void save_figseq( const std::string& filename,
                  const std::string& title,
                  bool verb)
{
  // OFFSCREEN saving.
  if( verb ) {
    std::cout << "  Saving PNG file=" << filename << std::endl;
  }

  Figure* fig_rdsom = new Figure( title, 450, 450, true /*offscreen */ );
  RDSOMViewer* rdsom_viewer = new RDSOMViewer( *_rdsom, *_winner_queue );
  fig_rdsom->add_curve( rdsom_viewer );
  fig_rdsom->set_draw_axes( false );
  fig_rdsom->render( true );
  // std::this_thread::sleep_for(std::chrono::seconds(1));
  fig_rdsom->save( filename );
  // std::this_thread::sleep_for(std::chrono::seconds(5));
  delete fig_rdsom;
  delete rdsom_viewer;
}
/**
 * Save a PNJ image of the weights, input (black) and recurrent (blue).
 *
 * GLOBAL : _rdsom
 */
void save_figweight( const std::string& filename,
                     const std::string& title,
                     bool verb)
{
  Figure* fig_weight = new Figure( title, 800, 350, true /*offscreen*/, 800, 50,
			       {0.0, (double) _rdsom->v_neur.size(),10,2},
			       {0.0, 1.0, 10, 2} );
  Curve* c_weight = new Curve();
  c_weight->set_color( {0.0, 0.0, 0.0} );
  c_weight->set_width( 1 );
  Curve* c_rweight = new Curve();
  c_rweight->set_color( {0.0, 0.0, 1.0} );
  c_rweight->set_width( 1 );
  for( unsigned int i = 0; i < _rdsom->v_neur.size(); ++i) {
    c_weight->add_sample( {(double)i, _rdsom->v_neur[i]->weights(0), 0.0} );
    c_rweight->add_sample( {(double)i, _rdsom->v_neur[i]->r_weights(0), 0.0} ); 
  }

  fig_weight->add_curve( c_weight );
  fig_weight->add_curve( c_rweight );

  fig_weight->render( true );
  fig_weight->save( filename );

  delete c_weight;
  delete c_rweight;
  delete fig_weight;
}
/**
 * Save a PNJ image of errors: input (black), recurrent (blue), pred(red).
 *
 * GLOBAL : _rdsom, _c_error_input, _c_error_rec, _c_error_pred
 */
void save_figerror( const std::string& filename,
                     const std::string& title,
                     bool verb)
{
  Figure* fig_error = new Figure( title, 800, 350, true /*offscreen*/, 400, 430,
                                  {0.0, 100 ,10,2},
                                  {0.0, 1.2, 10, 2});

  CurveMean* c_error_input = new CurveMean( *_c_error_input );
  if( c_error_input->get_mean_mode() == false ) {
    c_error_input->set_mean_mode( true );
    c_error_input->recompute_means();
  }
  CurveMean* c_error_rec = new CurveMean( *_c_error_rec );
    if( c_error_rec->get_mean_mode() == false ) {
    c_error_rec->set_mean_mode( true );
    c_error_rec->recompute_means();
  }
  CurveMean* c_error_pred = new CurveMean( *_c_error_pred );
  if( c_error_pred->get_mean_mode() == false ) {
    c_error_pred->set_mean_mode( true );
    c_error_pred->recompute_means();
  }
  
  fig_error->add_curve( c_error_input );
  fig_error->add_curve( c_error_rec );
  fig_error->add_curve( c_error_pred );

  fig_error->render( true );
  // std::this_thread::sleep_for(std::chrono::seconds(1));
  fig_error->save( filename );
  // std::this_thread::sleep_for(std::chrono::seconds(5));
  delete c_error_input;
  delete c_error_rec;
  delete c_error_pred;
  delete fig_error;
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
	if( _act_queue ) {
	  _act_queue->push_front( static_cast<double>( rdsom.get_winner() ));
	  _input_queue->push_front( input[0] * rdsom.get_size_grid() );
	}
				 
  }
}
/**
 * step_learn: (usually called before graphic update)
 * learns with 'length' learning data considered as a circular array,
 * begin at '_ite_step' and come back to 'input_start' after 'input_end'. 
 *
 * :Global: : _ite_step, iterator on the learning data.
 */
void step_learn( RDSOM& rdsom,
		 const Traj::iterator::difference_type& length,
		 const Traj::iterator& input_start,
		 const Traj::iterator& input_end)
{
  // DEBUG
  FixedQueue<unsigned int>  winqueue(_opt_seqlog_size);
  _seqmap_learn.clear();
  // DEBUG
  
  for( unsigned int i = 0; i < length; ++i) {
    // Forward new input and update network
    Eigen::VectorXd input(1);
    input << (double) _ite_step->id_o;
    if( _opt_verb ) {
      std::cout << "__STEP LEARN _nb_step=" << _nb_step << std::endl;
    }
    rdsom.forward( input, _opt_beta,
		   _opt_sig_input, _opt_sig_recur, _opt_sig_convo,
		   _opt_verb);
    rdsom.deltaW( input, _opt_eps, _opt_ela, _opt_ela_rec, _opt_verb);

    // store winner for SeqMAP
    winqueue.push_front( rdsom.get_winner() );
    // if "activated", retrieve sequence and add to _seqmap
    if( input[0] > _opt_seqlog_threshold ) {
      // sequence is reversed from winqueue
      Sequence seq(winqueue.rbegin(), winqueue.rend());
      _seqmap_learn[seq]++;
    }
    
    if( _winner_queue ) {
      _winner_queue->push_front( rdsom.get_winner() );
    }
    if( _act_queue ) {
      _act_queue->push_front( static_cast<double>( rdsom.get_winner() ));
      _input_queue->push_front( input[0] * rdsom.get_size_grid() );
    }
    

    // Add error to figure
    if( _opt_graph or _opt_figerror ) {
      _c_error_input->add_sample( {(double)_nb_step, rdsom.get_winner_dist_input(), 0.0 } );
      _c_error_rec->add_sample( {(double)_nb_step, rdsom.get_winner_dist_rec(), 0.0 } );
      _c_error_pred->add_sample( {(double)_nb_step, rdsom.get_winner_dist_pred(), 0.0} );
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
/**
 * step_test:
 * compute mean error_input, rec and pred on all given data
 * when input "activates" (input > _opt_seqlog_threshold)
 *      store last _opt_seqlog_size winners as sequence
 *      increase sequence frequency in map
 * output first _opt_seqlog_nb frequence sequences
 */
void step_test( RDSOM& rdsom,
		const Traj::iterator& input_start,
		const Traj::iterator& input_end)
{
  Model::DSOM::RNetwork::TNumber err_input = 0;
  Model::DSOM::RNetwork::TNumber err_rec = 0;
  Model::DSOM::RNetwork::TNumber err_pred = 0;

  _seqmap.clear();
  FixedQueue<unsigned int>  winqueue(_opt_seqlog_size);

  if( _opt_verb ) {
    std::cout << "__STEP TEST " << std::endl;
  }
  for (auto it = input_start; it != input_end; ++it) {
    // Forward new input BUT do not update network
    Eigen::VectorXd input(1);
    input << (double) it->id_o;
    // if( _opt_verb ) {
    //   std::cout << "  in=" << input << std::endl;g
    // }
    rdsom.forward( input, _opt_beta,
		   _opt_sig_input, _opt_sig_recur, _opt_sig_convo,
		   false/*_opt_verb*/);

    // store winner
    winqueue.push_front( rdsom.get_winner() );
    
    // if( _winner_queue ) {
    //   _winner_queue->push_front( rdsom.get_winner() );
    // }

    err_input += rdsom.get_winner_dist_input();
    err_rec += rdsom.get_winner_dist_rec();
    err_pred += rdsom.get_winner_dist_pred();

    // if "activated", retrieve sequence and add to _seqmap
    if( input[0] > _opt_seqlog_threshold ) {
      // sequence is reversed from winqueue
      Sequence seq(winqueue.rbegin(), winqueue.rend());
      _seqmap[seq]++;
    }
  }

  // mean
  Model::DSOM::RNetwork::TNumber length = input_end - input_start;
  _v_err_input.push_back(err_input / length );
  _v_err_rec.push_back(err_rec / length );
  _v_err_pred.push_back(err_pred / length );

  if( _opt_verb ) {
    // sort SequencePair and print most frequent
    SequenceVec seqvec;
    // Build from SequencePairs of _seqmap
    for (auto it = _seqmap.begin(); it != _seqmap.end(); ++it) {
      seqvec.push_back(*it);
    }
    // sort using pairs->second (lambda function)
    std::sort( seqvec.begin(), seqvec.end(),
	       [] (SequencePair& a, SequencePair& b) {
		 return a.second > b.second;
	       }
	       );

    std::cout << "__FREQ SEQUENCE" << std::endl;
    for (auto it = seqvec.begin();
	 it != std::min(seqvec.end(), seqvec.begin()+_opt_seqlog_nb);
	 ++it) {
      std::cout << utils::str_vec(it->first) << " -> " << it->second << std::endl;
    }
  }
  
}
// ***************************************************************************
// ********************************************************************** main
// ***************************************************************************
int main(int argc, char *argv[])
{
  // init random by default
  // Generate seed
  unsigned int seed = utils::random::rnd_int<unsigned int>();
  std::srand( seed );  
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
   // GRAPHIC or LEARN or TEST will use _winner_queue
   _winner_queue = new FixedQueue<unsigned int>( _opt_queue_size);
   _act_queue = new FixedQueue<double>( _opt_hist_size);
   _input_queue = new FixedQueue<double>( _opt_hist_size);

   // GRAPHIC or LEARN or TEST will use _c_error_*
   _c_error_input = new CurveMean();
   _c_error_input->set_color( {0.0, 0.0, 0.0} );
   _c_error_rec = new CurveMean();
   _c_error_rec->set_color( {0.0, 0.0, 1.0} );
   _c_error_pred = new CurveMean();
   _c_error_pred->set_color( {1.0, 0.0, 0.0} );
   
   // Graphic before Learning (as Learn will depend on it)
   if( _opt_graph and !_opt_test ) {
     if( _opt_verb) {
       std::cout << "__INIT GRAPHIC" << std::endl;
     }
     _fig_rdsom = new Figure( "RDSOM: r-network", 450, 450, false, 340, 0 );
     _rdsom_viewer = new RDSOMViewer( *_rdsom, *_winner_queue );
     _fig_rdsom->add_curve( _rdsom_viewer );
     _fig_rdsom->set_draw_axes( false );

     _fig_weight = new Figure( "Input/Weights", 800, 350, false, 800, 50,
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
  
     _fig_rweight = new Figure( "Recurrent/RWeights", 800, 350, false, 800, 430,
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

     // History
     _fig_hist = new Figure( "History", 800, 350, false, -1, -1,
			     {0.0, (double) _opt_hist_size * 1.1, 10, 2},
			     {0.0, (double) _rdsom->get_size_grid() * 1.1, 10, 2} );
     _c_hist_act = new CurveDyn<FixedQueue<double>>( *_act_queue );
     _c_hist_input = new CurveDyn<FixedQueue<double>>( *_input_queue );
     _c_hist_input->set_color( {0.0, 1.0, 0.0} );
     _fig_hist->add_curve( _c_hist_act );
     _fig_hist->add_curve( _c_hist_input );
     
     _fig_rdsom->render( true );
     _fig_weight->render();
     _fig_rweight->render();
     _fig_hist->render(),

     // Errors
     _fig_error = new Figure( "Error", 800, 350, false, 400, 430,
				{0.0, 100 ,10,2},
				{0.0, 1.2, 10, 2});
     _fig_error->add_curve( _c_error_input );
     _fig_error->add_curve( _c_error_rec );
     _fig_error->add_curve( _c_error_pred );
     
     _ite_cur = 0;
     while( not _end_render ) {
       // update if _run_updata
       if( _run_update ) {
	 auto learn_length = _opt_queue_size * _learn_length_multiplier;
	 if( learn_length == 0 ) learn_length = 1;
	 step_learn( *_rdsom, learn_length,
		     _data->begin(), _data->end() );
         _ite_cur += learn_length;

	 update_graphic();
       }       
       _fig_rdsom->render();
       _fig_weight->render();
       _fig_rweight->render();
       _fig_error->render( true, false ); // Update axes x, y
       _fig_hist->render();
     }
   }
   else if( !_opt_test ) {
	 // Learn_________________________
	 std::cout << "__LEARN" << std::endl;
	   
     _ite_cur = 0;
     // Fig with weights is saved at start
     std::stringstream count;
     count << std::setw(6) << std::setfill('0') <<  _ite_cur << ".png";
     save_figweight( *_opt_filesave_result+"_figweight_"+count.str(),
                     "FigWeight", false );
     
	 while( _ite_cur < _opt_learn_length ) {
	   step_learn( *_rdsom, _opt_period_save,
				   _data->begin(), _data->end() );
	   
	   _ite_cur += _opt_period_save;
	   
	   // Test a copy of rdsom on all data
	   RDSOM tmp_rdsom{ *_rdsom };
	   tmp_rdsom.reset();
 	   step_test( tmp_rdsom, _data->begin(), _data->end() );
	   
	   std::cout << "  IT=" << _ite_cur << ", saving..." << std::endl;
	   // std::cout << "  err_in=" << _v_err_input.back() << std::endl;
	   
	   // Save rdsom
	   if( _opt_filesave_result ) {
		 std::stringstream f_rdsomsave;
		 f_rdsomsave << *_opt_filesave_result;
		 f_rdsomsave << "_rdsom_" << _ite_cur;
		 save_rdsom( f_rdsomsave.str(), *_rdsom );

		 // Save most frequent test patterns
		 std::stringstream f_seqlogsave;
		 f_seqlogsave << *_opt_filesave_result;
		 f_seqlogsave << "_seqlog_" << _ite_cur;
		 save_seqlog( f_seqlogsave.str(), _seqmap );

		 // Save most frequent learn patterns
		 std::stringstream f_seqlogsave_learn;
		 f_seqlogsave_learn << *_opt_filesave_result;
		 f_seqlogsave_learn << "_seqlog_learn_" << _ite_cur;
		 save_seqlog( f_seqlogsave_learn.str(), _seqmap_learn );

                 // OFFSCREEN saving
                 std::cout << "  PNG IT=" << _ite_cur << ", saving ong..." << std::endl;
                 std::stringstream filename_png;
                 filename_png << *_opt_filesave_result;
                 filename_png << "_figseq_" << std::setw(6) << std::setfill('0') <<  _ite_cur << ".png";
                 std::stringstream title_png;
                 title_png << "Queue: it=" << _ite_cur;
                 save_figseq( filename_png.str(), title_png.str(), /*verb*/ false );
                 
	   }
	   
	   // 	// Some kind of criteria
	 }

     // At the end, save errors
	 std::cout << "  END IT=" << _ite_cur << ", saving..." << std::endl;
	 std::stringstream filename_error;
	 filename_error << *_opt_filesave_result;
	 filename_error << "_errors";
	 auto ofile = std::ofstream( filename_error.str() );
	 
	 // Header comments
	 ofile << "## \"traj_name\" : \"" << *_opt_fileload_traj << "\"," << std::endl;
	 ofile << "## \"rdsom_name\": \"" << *_opt_fileload_rdsom << "\"," << std::endl;
	 // @todo: parameters
	 ofile << "## \"beta\"; \"" << _opt_beta << "\"," << std::endl;
	 ofile << "## \"sigma_input\"; \"" << _opt_sig_input << "\"," << std::endl;
	 ofile << "## \"sigma_recur\"; \"" << _opt_sig_recur << "\"," << std::endl;
	 ofile << "## \"sigma_convo\"; \"" << _opt_sig_convo << "\"," << std::endl;
	 ofile << "## \"epsilon\"; \"" << _opt_eps << "\"," << std::endl;
	 ofile << "## \"ela_input\"; \"" << _opt_ela << "\"," << std::endl;
	 ofile << "## \"ela_rec\"; \"" << _opt_ela_rec << "\"," << std::endl;
	 // Header col names
	 ofile << "ite\terr_in\terr_rec\terr_pred" << std::endl;
	 // Data
	 _ite_cur = 0;
	 unsigned int idx = 0;
     while( _ite_cur < _opt_learn_length ) {
       _ite_cur += _opt_period_save;
       std::cout << "__SAVING for ite="<< _ite_cur << " idx=" << idx << std::endl;
       ofile << _ite_cur << "\t";
       ofile << _v_err_input[idx] << "\t";
       ofile << _v_err_rec[idx] << "\t";
       ofile << _v_err_pred[idx];
       ofile << std::endl;
       
       ++idx;
     }
     ofile.close();

     // Seqlog most frequent
     

     // OFFSCREEN saving.
     // Fig with weights is saved at start
     std::stringstream count_end;
     count_end << std::setw(6) << std::setfill('0') <<  _ite_cur << ".png";
     save_figweight( *_opt_filesave_result+"_figweight_"+count_end.str(),
                     "FigWeight", false );
     save_figseq( *_opt_filesave_result+"_figseq_"+count_end.str(),
                     "FigSeq", false );
     save_figerror( *_opt_filesave_result+"_figerror_"+count_end.str(),
                     "FigError", false );

     // DEBUG write queue
     std::cout << str_queue() << std::endl;
     
   }
   else {
	 // Test____________________________
	 std::cout << "__TEST"  << std::endl;

	 for( unsigned int i = 0; i < _opt_nb_test; ++i) {
	   step_test( *_rdsom, _data->begin(), _data->end() );
	   std::cout << "  IT=" << i << std::endl;
	 }

	 // At the end, save errors
	 std::stringstream filename_error;
	 filename_error << *_opt_filesave_result;
	 filename_error << "_test";
	 auto ofile = std::ofstream( filename_error.str() );
	 
	 // Header comments
	 ofile << "## \"traj_name\" : \"" << *_opt_fileload_traj << "\"," << std::endl;
	 ofile << "## \"rdsom_name\": \"" << *_opt_fileload_rdsom << "\"," << std::endl;
	 // @todo: parameters
	 ofile << "## \"beta\"; \"" << _opt_beta << "\"," << std::endl;
	 ofile << "## \"sigma_input\"; \"" << _opt_sig_input << "\"," << std::endl;
	 ofile << "## \"sigma_recur\"; \"" << _opt_sig_recur << "\"," << std::endl;
	 ofile << "## \"sigma_convo\"; \"" << _opt_sig_convo << "\"," << std::endl;
	 ofile << "## \"epsilon\"; \"" << _opt_eps << "\"," << std::endl;
	 ofile << "## \"ela_input\"; \"" << _opt_ela << "\"," << std::endl;
	 ofile << "## \"ela_rec\"; \"" << _opt_ela_rec << "\"," << std::endl;
	 // Header col names
	 ofile << "ite\terr_in\terr_rec\terr_pred" << std::endl;
	 // Data
	 for( unsigned int idx = 0; idx < _opt_nb_test; ++idx) {
	   ofile << idx << "\t";
	   ofile << _v_err_input[idx] << "\t";
	   ofile << _v_err_rec[idx] << "\t";
	   ofile << _v_err_pred[idx];
	   ofile << std::endl;

	   ++idx;
	 }
	 ofile.close();

	 // OFFSCREEN saving
	 // And save a PNG image of the last _opt_queue_size neurons
	 std::stringstream filename_png;
	 filename_png << *_opt_filesave_result;
	 filename_png << "_rdsom.png";

	 _fig_rdsom = new Figure( "RDSOM: r-network", 450, 450, true /* offscreen */ );
	 _rdsom_viewer = new RDSOMViewer( *_rdsom, *_winner_queue );
	 _fig_rdsom->add_curve( _rdsom_viewer );
	 _fig_rdsom->set_draw_axes( false );
	 _fig_rdsom->render( true );
	 // std::this_thread::sleep_for(std::chrono::seconds(1));
	 _fig_rdsom->save( filename_png.str() );
	 // std::this_thread::sleep_for(std::chrono::seconds(5));
	 delete _fig_rdsom;
	 
   }
   return 0;
}

	 unsigned int ite_test = 0;
