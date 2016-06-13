/* -*- coding: utf-8 -*- */

/** 
 * WP with HMM (cmp with BICA's Paper de Jeremy and Herve
 *
 * - generate/save/load HMM : expr
 * - generate Trajectories of (S,O). inspi from Trajectory
 * - save/load Trajectories
 * - create ESN : two initialisation options
 * - save/load ESN
 *
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

#include <reservoir.hpp>
#include <layer.hpp>
#include <ridge_regression.hpp>

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
Problem _pb;

// Trajectory
using Traj = Trajectory::HMM::Data;
Traj _learn_data;

// ESN
using PtrReservoir =   std::unique_ptr<Reservoir>;
using PtrLayer     = std::unique_ptr<Layer>;
using ESN = struct {
  PtrReservoir      res = nullptr;
  PtrLayer          lay = nullptr;
  bool    input_forward = false;
};
ESN _esn;

// **************************************************************** create_hmm
Problem create_hmm( const std::string& expr = "ABCD" )
{
  Problem pb;
  pb.expr = expr;
  auto hmm = bica::hmm::make(expr);
  std::tie(pb.t, pb.o) = hmm.first;
  pb.nb_states = hmm.second;

  return pb;
}
// ****************************************************************** save_hmm
void save_hmm( const std::string& filename, const std::string hmm_expr )
{
  auto ofile = std::ofstream( filename );

  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember( "hmm",
		 bica::hmm::serialize( doc, hmm_expr ),
		 doc.GetAllocator());

  ofile << str_obj(doc) << std::endl;
  ofile.close();
};
// ****************************************************************** load_hmm
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
};
// ***************************************************************** print_hmm
void print_hmm(const std::string& name, bica::sampler::HMM& h, int nb)
{
  std::cout << name << " : " << std::fixed;
  for(int i = 0; i < nb; ++i, h.shift())
    std::cout << std::setw(5) << std::setprecision(2) << h.input() << '('
	      << std::setw(2) << h.input_id() << ") ";
  std::cout << std::endl;
  std::cout << std::endl;
}
// *************************************************************** create_traj
Traj create_traj( const unsigned int length, const Problem& pb )
{
  Traj traj;
  // Sampler pour le HMM
  bica::sampler::HMM hmm( pb.t, pb.o);

  traj.clear();
  for( unsigned int i = 0; i < length; ++i) {
    traj.push_back( Trajectory::HMM::Item{ hmm.input_id(), hmm.input()} );
    hmm.shift();
  }

  return traj;
};
// ***************************************************************** save_traj
void save_traj( const std::string& filename,
		const Traj& data,
		const std::string hmm_expr)
{
  auto ofile = std::ofstream( filename );

  // header
  ofile << "## \"hmm_expr\": \"" << hmm_expr << "\"," << std::endl;

  Trajectory::HMM::save( ofile, data );
  
  ofile.close();
};
// ***************************************************************** load_traj
Traj load_traj( const std::string& filename )
{
  Traj traj;
  auto pfile = std::ifstream( filename );
  Trajectory::HMM::read( pfile, traj );
  pfile.close();

  return traj;
};
// **************************************************************** create_esn
ESN create_esn( Reservoir::Tinput_size input_size = 1,
		Layer::Toutput_size output_size = 1,   
		Reservoir::Toutput_size reservoir_size = 10,
		bool forward_input = false,
		bool szita_input = false,
		double szita_val = 1.0,
		double input_scaling = 1.0,
		double spectral_radius= 0.99,
		double leaking_rate = 0.1
		)
{
  ESN esn;
  esn.res = make_unique<Reservoir>( input_size, reservoir_size,
				   input_scaling, spectral_radius, leaking_rate );
  if( forward_input ) {
    // layer take also input 
    esn.lay = make_unique<Layer>( input_size+reservoir_size+1, output_size );
  }
  else {
    esn.lay = make_unique<Layer>( reservoir_size+1, output_size );
  }
  esn.input_forward = forward_input;

  // Special Input Weights bu [Szita06]
  if( szita_input ) {
    esn.res->init_discrete_input( szita_val );
  }

  return esn;
};
// ****************************************************************** save_esn
void save_esn( const std::string& filename, const ESN& esn )
{
  auto ofile = std::ofstream( filename );

  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember( "res", esn.res->serialize(doc), doc.GetAllocator());
  doc.AddMember( "lay", esn.lay->serialize(doc), doc.GetAllocator());
  doc.AddMember( "input_forward", rj::Value(esn.input_forward), doc.GetAllocator() );

  ofile << str_obj(doc) << std::endl;
  ofile.close();
};
// ****************************************************************** load_esn
ESN load_esn(const std::string& filename )
{
  ESN esn;
  
  std::ifstream ifile( filename );
  // Wrapper pour lire document
  JSON::IStreamWrapper instream(ifile);
  // Parse into a document
  rj::Document doc;
  doc.ParseStream( instream );
  ifile.close();

  esn.res = make_unique<Reservoir>( doc["res"] );
  esn.lay = make_unique<Layer>( doc["lay"] );
  esn.input_forward = doc["input_forward"].GetBool();
  
  ifile.close();

  return esn;
}
// ***************************************************************************
// ********************************************************************** main
// ***************************************************************************
int main(int argc, char *argv[])
{
  // Create a new HMM
  _pb = create_hmm( "ABCDCB" );
  bica::sampler::HMM hmm1(_pb.t,_pb.o);
  std::cout << "__" << _pb.expr << "__ with " << _pb.nb_states << " states" << std::endl;  
  // print_hmm("h1",hmm1,30);
  // // Save to JSON
  // save_hmm( "tmp_hmm.json", _pb.expr );
  // // Reload from JSON
  // // first create another one
  // _pb = create_hmm( "AAAF" );
  // bica::sampler::HMM hmm2(_pb.t,_pb.o);
  // std::cout << "__" << _pb.expr << "__ with " << _pb.nb_states << " states" << std::endl;  
  // print_hmm("h2",hmm2,30);
  // // then load
  // _pb = load_hmm( "tmp_hmm.json" );
  // bica::sampler::HMM hmm3(_pb.t,_pb.o);
  // std::cout << "__" << _pb.expr << "__ with " << _pb.nb_states << " states" << std::endl;  
  // print_hmm("h3",hmm3,30);

  // // Create and save trajectory
  // _learn_data = create_traj( 100, _pb );
  // save_traj( "tmp_traj.json", _learn_data, _pb.expr );
  // // Read Trajectory
  // _learn_data.clear();
  // _learn_data = load_traj( "tmp_traj.json" );
  // for (auto it = _learn_data.begin(); it != _learn_data.begin()+10; ++it) {
  //   std::cout << "s=" << (*it).id_s << "\to=" << (*it).id_o << std::endl;    
  // }

  // Create and save ESN
  _esn = create_esn( 2, 3, 5, true, true, 4);
  save_esn( "tmp_esn.json", _esn );
  ESN esn_read = load_esn( "tmp_esn.json" );
  std::cout << "__READ RES: " << esn_read.res->str_display();
  std::cout << " LAY: " << esn_read.lay->str_display() << std::endl;
  
  return 0;
}
// ***************************************************************************



