/* -*- coding: utf-8 -*- */

/** 
 * test-009-mackey.cpp
 *
 * Generate and print Mackeyglass sequence.
 */

#include <iostream>       // std::cout

#include <mackeyglass.hpp>

#include <gaml.hpp>       // GAML Library

#include "rapidjson/document.h"         // rapidjson's DOM-style API

// ******************************************************************** Global
#define DATA_FILE "mackeyglass.data"

/** Parser for Mackeyglass */
struct SimpleParser {
  typedef double value_type;
  void read(std::ostream& os, value_type& x) const {
    // Not needed
  }
  void write(std::ostream& os, const value_type& x) const {
    os << x;
  }
};

//******************************************************************************
int main( int argc, char *argv[] )
{
  // Deux façons de générer des séquences
  // UN : un passant par un créateur, ce qui permet ensuite de serialiser
  //      les paramètres.
  MackeyGlass mackey( 20, 1.0, 0.1, 0.5, 2.0, 5 );
  MackeyGlass::Data seq = mackey.create_sequence();
  // Les param sous forme de JSON
  rapidjson::StringBuffer buffer;
  mackey.serialize( buffer );
  std::cout << buffer.GetString() << std::endl;

  // DEUX : en passant par la fonction statique qui crée une séquence.
  MackeyGlass::Data data = MackeyGlass::create_sequence( 20, 1.0, 0.1, 0.5, 2.0, 5 );
  

  std::cout << "SEQ={ ";
  for( auto& x: data) {
    std::cout << x << "; ";
  }
  std::cout << "}" << std::endl;

  // Try to use GAML output_streams => pas très utile ici au vu des données.
  // [Q] et pour ajouter des commentaires en haut du fichier ??
  SimpleParser my_parser;
  auto output_stream = gaml::make_output_data_stream(std::cout, my_parser);
  auto out1 = gaml::make_output_iterator(output_stream);
  std::cout << std::endl << "GAML built-in output parser" << std::endl
	    << std::endl;
  std::copy(data.begin(), data.end(), out1);

  std::cout << "MackeyGlass Writer" << std::endl;
  MackeyGlass::write( std::cout, data);

  // Essai de sérialisation
  std::ofstream ofile(DATA_FILE);
  MackeyGlass::write( ofile, data);
  ofile.close();

  // Et maintenant on relis
  MackeyGlass::Data data_read;
  std::ifstream ifile(DATA_FILE);
  MackeyGlass::read( ifile, data_read);
  ifile.close();

  // Compare les deux
  std::cout << std::endl << "COMPARAISON data =?= read " << std::endl;
  for( unsigned int i = 0; i < data.size(); ++i) {
    std::cout << data[i] << "  =?= " << data_read[i] << std::endl;
  }
  
  return 0;
}

