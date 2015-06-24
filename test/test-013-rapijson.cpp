/* -*- coding: utf-8 -*- */

/** 
 * Produit un document JSON avec 2 objets, met dans fichier
 * et relis.
 */

#include <iostream>                   // std::cout
#include <fstream>                    // std::ofstream

#include "rapidjson/prettywriter.h"   // rapidjson
#include "rapidjson/document.h"       // rapidjson's DOM-style API
#include <json_wrapper.hpp>           // JSON::OStreamWrapper et IStreamWrapper
namespace rj = rapidjson;

#define FILENAME "test-013.data"

//******************************************************************************
int main( int argc, char *argv[] )
{
  rj::Document doc; // initially Null
  doc.SetObject();

  // first object
  rj::Value ob1;
  ob1.SetObject();
  rj::Value m1(123);
  rj::Value m2(55555);
  ob1.AddMember( "m1", m1, doc.GetAllocator() );
  ob1.AddMember( "m2", m2, doc.GetAllocator() );
  doc.AddMember( "ob1", ob1, doc.GetAllocator());
  
  // second object
  rj::Value ob2(rj::kObjectType);
  ob2.AddMember( "m1", m2, doc.GetAllocator() );
  doc.AddMember( "ob2", ob2, doc.GetAllocator());
  
  // output
  rj::StringBuffer buffer;
  rj::PrettyWriter<rj::StringBuffer> writer(buffer);
  doc.Accept( writer );

  std::cout << buffer.GetString() << std::endl;

  // Ecrit dans un fichier
  std::ofstream ofile(FILENAME);
  ofile << buffer.GetString() << std::endl;
  ofile.close();

  // Relis
  std::ifstream ifile(FILENAME);
  JSON::IStreamWrapper instream(ifile);
  // Parse into a document
  rj::Document doc_read;
  doc_read.ParseStream( instream );
  ifile.close();
  
  std::cout << "***** READ_JSON ***" << "\n";
  for (rj::Value::ConstMemberIterator itr = doc_read.MemberBegin();
       itr != doc_read.MemberEnd(); ++itr) {
    std::cout << "Doc has " << itr->name.GetString() << std::endl;
    if( itr->value.IsObject() ) {
      for (rj::Value::ConstMemberIterator itr2 = itr->value.MemberBegin();
	   itr2 != itr->value.MemberEnd(); ++itr2) {
	std::cout << "OBJ has " << itr2->name.GetString() << std::endl;
      }
    }
  }
  
  return 0;
}
