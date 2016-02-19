/* -*- coding: utf-8 -*- */

/** 
 * test-015-noise.cpp
 *
 * Generate, print, write/save Noise sequence.
 */

#include <noise.hpp>

#include <iostream>       // std::cout
#include <fstream>        // std::ifile

// ******************************************************************** Global
#define NOISE_FILE "wnoise.data"

//******************************************************************************
int main( int argc, char *argv[] )
{
  // Deux façons de générer des séquences
  // UN : un passant par un créateur, ce qui permet ensuite de serialiser
  //      les paramètres.
  WNoise wnoise( 20, 0.2, 3 );
  wnoise.create_sequence();

  // // Les param sous forme de JSON
  // rapidjson::Document doc;
  // rapidjson::Value obj = wnoise.serialize( doc );
  // std::cout << "** JSON **" << std::endl;
  // std::cout << str_obj( obj ) << std::endl;

  // DEUX : en passant par la fonction statique qui crée une séquence.
  WNoise::Data wnoise_data = WNoise::create_sequence( 20, 0.4, 2 );
  
  std::cout << "** WNOISE" << std::endl;
  for( auto& vec: wnoise.data()) {
    for( auto& var: vec) {
      std::cout << var << "; ";
    }
    std::cout << std::endl;
  }
  
  std::cout << "** WNOISE_DATA" << std::endl;
  for( auto& vec: wnoise_data) {
    for( auto& var: vec) {
      std::cout << var << "; ";
    }
    std::cout << std::endl;
  }

  // Essai de sérialisation
  std::ofstream ofile(NOISE_FILE);
  WNoise::write( ofile, wnoise.data());
  ofile.close();

  // Et maintenant on relis
  WNoise::Data data_read;
  std::ifstream ifile(NOISE_FILE);
  WNoise::read( ifile, data_read);
  ifile.close();

  // Compare les deux
  std::cout << "__ COMPARISON" << std::endl;
  if( wnoise.data().size() != data_read.size() ) {
    std::cerr <<  "wnoise.data().size()=" << wnoise.data().size() << "!= data_read.size()=" << data_read.size() << std::endl;
    abort();
  }
  std::cout << std::endl << "COMPARAISON data =?= read " << std::endl;
  for( unsigned int i = 0; i < wnoise.data().size(); ++i) {
    if( wnoise.data()[i].size() != data_read[i].size() ) {
      std::cerr <<  "i="<< i << " => wnoise.data()[i].size()=" << wnoise.data()[i].size() << "!= data_read[i].size()=" << data_read[i].size() << std::endl;
      for( unsigned int j = 0; j < wnoise.data()[i].size(); ++j) {
	std::cout << "wnoise.data()[i][j]= " << wnoise.data()[i][j] << std::endl;
      }
      for( unsigned int j = 0; j < data_read[i].size(); ++j) {
	std::cout << "data_read[i][j]= " << data_read[i][j] << std::endl;
      }
    abort();
  }
    for( unsigned int j = 0; j < wnoise.data()[i].size(); ++j) {
      std::cout << wnoise.data()[i][j] << "  =?= " << data_read[i][j] << std::endl;
    }
  }

  return 0;
}
