/* -*- coding: utf-8 -*- */

#pragma once

/** 
 * Création d'un réservoir random et uniform, seed depends on std::time().
 *
 * @param _input_scaling : input weights in [-input_scaling,input_sclaing]
 * @todo : autres params
 */

#include <iostream>                     // std::cout
#include <sstream>                      // std::stringstream

#include <gsl/gsl_rng.h>                // gsl random generator
#include <gsl/gsl_matrix.h>             // gsl Matrices
#include <gsl/gsl_eigen.h>              // gsl EigenValues
#include <gsl/gsl_complex_math.h>       // gsl complex
#include <gsl/gsl_blas.h>               // gsl matrix . vector multiplication

#include <ctime>                        // std::time
#include <vector>                       // std::vector
#include <math.h>                       // tanh

#include "rapidjson/prettywriter.h"     // rapidjson
#include "rapidjson/document.h"         // rapidjson's DOM-style API
#include <json_wrapper.hpp>             // JSON::OStreamWrapper et IStreamWrapper

#include <utils_gsl.hpp>                // utils::str_vec, utils::str_mat ...
using namespace utils;
// ***************************************************************************
// ***************************************************************** Reservoir
// ***************************************************************************
class Reservoir
{
public:
  typedef std::vector<double> Tinput;
  typedef unsigned int        Tinput_size;
  typedef std::vector<double> Toutput;
  typedef unsigned int        Toutput_size;
  typedef gsl_matrix*         Tweights;
  typedef gsl_vector*         Tstate;

  // **************************************************************** creation
  /** Creation */
  Reservoir( Tinput_size input_size, Toutput_size output_size,
	     double input_scaling = 1.0,
	     double spectral_radius= 0.99,
	     double leaking_rate = 0.1 ) :
    _input_scaling(input_scaling), _spectral_radius(spectral_radius),
    _leaking_rate(leaking_rate),
    _w_in(nullptr), _w_res(nullptr), _x_res(nullptr), _rnd(nullptr)
  {
    // Random generator with seed = time
    _rnd = gsl_rng_alloc( gsl_rng_taus );
    gsl_rng_set( _rnd, std::time( NULL ) );
    
    // INPUT_WEIGHTS : Matrix _output_size lines of _input_size+1 columns
    // in [- _input_scaling, _input_scaling]
    _w_in = gsl_matrix_alloc( output_size, input_size+1);
    for( unsigned int i = 0; i < _w_in->size1; ++i) {
      for( unsigned int j = 0; j < _w_in->size2; ++j) {
	gsl_matrix_set( _w_in, i, j, (gsl_rng_uniform_pos(_rnd)-0.5) * _input_scaling / 0.5 );
      }
    }
    // RESERVOIR_WEIGHTS Matrix _output_size lines of _output_size columns
    // in [-0.5, 0.5] before spectral radius
    _w_res = gsl_matrix_alloc( output_size, output_size);
    for( unsigned int i = 0; i < _w_res->size1; ++i) {
      for( unsigned int j = 0; j < _w_res->size2; ++j) {
	gsl_matrix_set( _w_res, i, j, gsl_rng_uniform_pos(_rnd)-0.5 );
      }
    }
    set_spectral_radius( _spectral_radius );
    
    // RESERVOIR STATE : initialisé à 0
    _x_res = gsl_vector_calloc( output_size );
  };
  /** 
   * Creation à partir d'un fichier contenant uniquement JSON format
   * of ONE Reservoir.
   */
  Reservoir( std::istream& is ) :
    _w_in(nullptr), _w_res(nullptr), _x_res(nullptr), _rnd(nullptr)
  {
    // Wrapper pour lire document
    JSON::IStreamWrapper instream(is);
    // Parse into a document
    rapidjson::Document doc;
    doc.ParseStream( instream );

    std::cout << "Document read" << std::endl;
    for (rapidjson::Value::ConstMemberIterator itr = doc.MemberBegin();
	 itr != doc.MemberEnd(); ++itr) {
      std::cout << "Doc has " << itr->name.GetString() << std::endl;
    }

    unserialize( doc );
  };
  /** Creation from a JSON Object in a Document */
  Reservoir( const rapidjson::Value& obj ) :
    _w_in(nullptr), _w_res(nullptr), _x_res(nullptr), _rnd(nullptr)
  {
    unserialize( obj );
  }
  /** Destruction */
  ~Reservoir()
  {
    if( _rnd ) gsl_rng_free( _rnd );
    gsl_matrix_free( _w_res );
    gsl_matrix_free( _w_in );
    gsl_vector_free( _x_res );
  }
  // *************************************************************** operation
  Toutput forward( const Tinput& in )
  {
    // Verifie bonne taille
    if( (in.size()+1) != _w_in->size2 ) {
      std::cerr << "Reservoir.forward() : Wrong input size !" << std::endl;
      std::cerr << "                      in.size=" << in.size() << " != " << _w_in->size2 << std::endl;
    }
    // Into GSL : colomn matrix of in.size()+& row (for bias)
    Tstate v_input = gsl_vector_alloc(in.size()+1 );
    for( unsigned int i = 0; i< in.size(); ++i) {
      gsl_vector_set( v_input, i, in[i]);
    }
    gsl_vector_set( v_input, in.size(), 1.0);

    // std::cout<< "IN= {" << str_vec(v_input) << std::endl;
    
    // Computation
    Tstate v_tmp = gsl_vector_calloc( _x_res->size );
    // std::cout<< "TMP= {" << str_vec(v_tmp) << std::endl;
    // _w_in * input + _w_res * _x_res
    gsl_blas_dgemv(CblasNoTrans, 1.0, _w_res, _x_res, 0.0, v_tmp);  // tmp <- w_res * x_res
    // std::cout<< "TMP= {" << str_vec(v_tmp) << std::endl;
    gsl_blas_dgemv(CblasNoTrans, 1.0, _w_in, v_input, 1.0, v_tmp); // tmp <- _w_in * v_in + tmp
    // std::cout<< "XN= {" << str_vec(v_tmp) << std::endl;
    // tanh
    for( unsigned int i = 0; i< v_tmp->size; ++i) {
      gsl_vector_set( v_tmp, i, tanh( gsl_vector_get( v_tmp, i)));
    }
    // std::cout<< "tanh(XN)= {" << str_vec(v_tmp) << std::endl;
    // x = (1-alpha) x + alpha xtilde
    gsl_vector_scale( v_tmp, _leaking_rate );
    gsl_vector_scale( _x_res, 1.0 - _leaking_rate );
    gsl_vector_add( _x_res, v_tmp);
    // std::cout<< "X= {" << str_vec(_x_res) << std::endl;

    // Libère la mémoire
    gsl_vector_free(v_input);
    gsl_vector_free(v_tmp);

    // Et prépare output
    _output.clear();
    for( unsigned int i = 0; i < _x_res->size; ++i) {
      _output.push_back( gsl_vector_get( _x_res, i) );
    }
    return _output;
  }
  
  // ******************************************************************** init
  void set_spectral_radius( double radius )
  {
    // Compute spectral radius
    // Copier la matrice des poids
    gsl_matrix* _w_tmp = gsl_matrix_alloc( _w_res->size1, _w_res->size2 );
    gsl_matrix_memcpy( _w_tmp, _w_res);
    // Espace pour valeurs propres
    gsl_vector_complex* eval = gsl_vector_complex_alloc(_w_res->size1);
    // if( eval == NULL ) {
    //   std::cerr << "set_spectral_radius: Allocation of 'eval' failed" << "\n";
    // }
    // else {
    //   std::cout << "set_spectral_radius: 'eval' allocated" << "\n";
    // }
    // Espace pour calcul valeur propres
    gsl_eigen_nonsymm_workspace* work = gsl_eigen_nonsymm_alloc(_w_res->size1);
    // if( work == NULL ) {
    //   std::cerr << "set_spectral_radius: Allocation of 'work' Failed" << "\n";
    // }
    // else {
    //   std::cout << "set_spectral_radius: 'work' allocated" << "\n";
    // }
    gsl_eigen_nonsymm( _w_tmp, eval, work);

    // Liste les valeurs pour trouver le max
    double abs_max = 0.0;
    for( unsigned int i = 0; i < _w_res->size1; ++i) {
      // double real_eval_i = GSL_REAL(gsl_vector_complex_get (eval, i));
      // double imag_eval_i = GSL_IMAG(gsl_vector_complex_get (eval, i));
      double mag = gsl_complex_abs( gsl_vector_complex_get (eval, i));
      // std::cout << "EigenVal[" << i << "]= " << real_eval_i << " +i " << imag_eval_i << " ||=" << mag << std::endl;
      if( mag > abs_max ) {
	abs_max = mag;
      }
    }
    // Et divise tous les éléments de _w_res
    gsl_matrix_scale( _w_res, radius/abs_max );
    // Libère l'espace
    gsl_eigen_nonsymm_free( work );
    gsl_vector_complex_free( eval );
    gsl_matrix_free( _w_tmp );
  }
  // ***************************************************************** display
  /** display string */
  std::string str_dump()
  {
    std::stringstream dump;
    dump << "__WEIGHTS_IN__" << std::endl;
    dump << str_mat( _w_in );

    dump << "__WEIGHTS_RESERVOIR__" << std::endl;
    dump << str_mat( _w_res );

    dump << "__STATE_RESERVOIR__" << std::endl;
    dump << str_vec( _x_res );
    
    return dump.str();
  };
  // *************************************************************** serialize
  void serialize( rapidjson::StringBuffer& buffer )
  {
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    // Start
    writer.StartObject();
    // nb_in, nb_out
    writer.String("nb_input"); writer.Uint( _w_in->size2 - 1 );
    writer.String("nb_out"); writer.Uint( _w_in->size1 );
    // Parameters
    writer.String("input_scaling"); writer.Double( _input_scaling );
    writer.String("spectral_radius"); writer.Double( _spectral_radius );
    writer.String("leaking_rate"); writer.Double( _leaking_rate );

    // Weights
    writer.String("w_in");
    writer.StartArray();
    for( unsigned int i = 0; i < _w_in->size1; ++i) {
      for( unsigned int j = 0; j < _w_in->size2; ++j) {
	writer.Double( gsl_matrix_get( _w_in, i, j) );
      }
    }
    writer.EndArray();

    writer.String("w_res");
    writer.StartArray();
    for( unsigned int i = 0; i < _w_res->size1; ++i) {
      for( unsigned int j = 0; j < _w_res->size2; ++j) {
	writer.Double( gsl_matrix_get( _w_res, i, j) );
      }
    }
    writer.EndArray();

    // State
    writer.String("x_res");
    writer.StartArray();
    for( unsigned int i = 0; i < _x_res->size; ++i) {
      writer.Double( gsl_vector_get( _x_res, i) );
    }
    writer.EndArray();

    writer.EndObject();
  };
  void unserialize( const rapidjson::Value& obj )
  {
    // size
    Tinput_size nb_in =  obj["nb_input"].GetUint();
    Toutput_size nb_out =  obj["nb_out"].GetUint();
    // Matrix
    _w_in = gsl_matrix_alloc( nb_out, nb_in+1 );
    //std::std::cout <<  << std::endl; << "  read w_in" << std::endl;
    const rapidjson::Value& win = obj["w_in"];
    assert( win.IsArray() );
    rapidjson::SizeType idx = 0;
    for( unsigned int i = 0; i < _w_in->size1; ++i) {
      for( unsigned int j = 0; j < _w_in->size2; ++j) {
	assert(win[idx].IsNumber());
	gsl_matrix_set( _w_in, i, j, win[idx].GetDouble() );
	idx++;
      }
    }
    _w_res = gsl_matrix_alloc( nb_out, nb_out );
    const rapidjson::Value& wres = obj["w_res"];
    idx = 0;
    for( unsigned int i = 0; i < _w_res->size1; ++i) {
      for( unsigned int j = 0; j < _w_res->size2; ++j) {
    	gsl_matrix_set( _w_res, i, j, wres[idx].GetDouble() );
    	idx++;
      }
    }
    _x_res = gsl_vector_calloc( nb_out );
    const rapidjson::Value& xres = obj["x_res"];
    idx = 0;
    for( unsigned int i = 0; i < _x_res->size; ++i) {
      gsl_vector_set( _x_res, i, xres[idx].GetDouble() );
    	idx++;
    }

    // parameters
    _input_scaling = obj["input_scaling"].GetDouble();
    _spectral_radius = obj["spectral_radius"].GetDouble();
    _leaking_rate = obj["leaking_rate"].GetDouble();
  };
  // ************************************************************** attributes
private:
  //Tinput_size _input_size;
  //Toutput_size _output_size;
  /** Parameters */
  double _input_scaling;
  double _spectral_radius;
  double _leaking_rate;
  /** Input weights */
  Tweights _w_in;
  /** Reservoir weights */
  Tweights _w_res;
  /** Reservoir state */
  Tstate _x_res;
  Toutput _output;

  /** Random generator */
  gsl_rng* _rnd;
};
