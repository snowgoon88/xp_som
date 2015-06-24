/* -*- coding: utf-8 -*- */

/**
 * test-010-json.cpp
 * 
 * Crée un Reservoir et le serialise.
 */

#include <iostream>       // std::cout
#include <fstream>        // std::ofstream
#include <reservoir.hpp>

#include "rapidjson/document.h"         // rapidjson's DOM-style API
#include <json_wrapper.hpp>             // JSON::OStreamWrapper et IStreamWrapper

// ******************************************************************** Global
#define RES_FILE "reservoir.data"

//******************************************************************************
int main( int argc, char *argv[] )
{
  Reservoir res( 2, 3, 0.2);
  res.set_spectral_radius( 0.1 );
  std::cout << "***** RESERVOIR **" << "\n";
  std::cout << res.str_dump() << std::endl;

  std::cout << "***** JSON *******" << std::endl;
  rapidjson::StringBuffer buffer;
  res.serialize( buffer );
  std::cout << buffer.GetString() << std::endl;

  // Write un a file
  std::ofstream ofile(RES_FILE);
  ofile << buffer.GetString() << std::endl;
  ofile.close();

  // Read from file - method ONE
  // The file contains ONLY ONE JSON Object => Reservoir
  std::ifstream ifile(RES_FILE);
  Reservoir res_read( ifile );
  std::cout << "***** RES_READ_JSON ***" << "\n";
  std::cout << res_read.str_dump() << std::endl;

  // Read from file - method TWO
  // The file may contain many JSON Object, at least one reservoir
  std::ifstream jfile(RES_FILE);
  // Wrapper pour lire document
  JSON::IStreamWrapper instream( jfile );
  // Parse into a document
  rapidjson::Document doc;
  doc.ParseStream( instream );
  // Begin to parse document, and for each ReservoirJSONObject obj
  // (here, doc IS a ReservoirJSONObject )
  Reservoir res_json( doc );
  std::cout << "***** RES_READ_DOC ***" << "\n";
  std::cout << res_json.str_dump() << std::endl;
  
  
  std::cout << "***** RES_end ****" << "\n";
  
  return 0;
}
