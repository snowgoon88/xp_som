/* -*- coding: utf-8 -*- */

/** 
 * test-009-mackey.cpp
 *
 * Generate and print Mackeyglass sequence.
 */

#include <iostream>       // std::cout

#include <mackeyglass.hpp>

#include <gaml.hpp>       // GAML Library

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

  return 0;
}

