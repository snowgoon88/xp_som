/* -*- coding: utf-8 -*- */

#ifndef HMM_JSON_HPP_HPP
#define HMM_JSON_HPP_HPP

/** 
 * Create/Save/Write HMM from JSON
 */

#include <hmm.hpp>

#include "rapidjson/prettywriter.h"  // rapidjson
#include "rapidjson/document.h"      // rapidjson's DOM-style API
//#include <json_wrapper.hpp>          // JSON::OStreamWrapper et IStreamWrapper
namespace rj = rapidjson;

// ***************************************************************************
// ***************************************************************** bica::hmm
// ***************************************************************************
namespace bica {
namespace hmm {

typedef std::pair<std::pair<T,O>, unsigned int> THMM;
  
// ****************************************************** bica::hmm::serialize
rj::Value serialize( rj::Document& doc, const std::string hmm_expr )
{
  // rj::Object qui contient les données
  rj::Value obj;
  obj.SetObject();

  // "hmm" field
  obj.AddMember( "expr", rj::StringRef( hmm_expr.c_str() ), doc.GetAllocator() );
  
  return obj;
}
// **************************************************** bica::hmm::unserialize
std::string unserialize( const rj::Value& obj )
{
  // string expression of HMM
  return std::string(obj["expr"].GetString());
}

// *********************************************************************** end

}; // namespace hmm
}; // namespace bica

#endif // HMM_JSON_HPP_HPP
