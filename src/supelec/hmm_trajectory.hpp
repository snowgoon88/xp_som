/* -*- coding: utf-8 -*- */

#ifndef HMM_TRAJECTORY_HPP_HPP
#define HMM_TRAJECTORY_HPP_HPP

/** 
 * Trajectory::HMM is a sequence of Trajectory::HMM:Item(s,o)
 */

#include <vector>

// **************************************************************** Trajectory
namespace Trajectory
{
// ***************************************************************************
// *********************************************************************** HMM
// ***************************************************************************
class HMM
{
public:
  // *************************************************************** HMM::Item
  typedef struct {
    int    id_s;
    double id_o;
  } Item;
  // *************************************************************** HMM::Data
  typedef std::vector<Item> Data;
  // *************************************************************** HMM::save
  static void save( std::ostream& os, const Data& data )
  {
    // header
    os << "## \"length\": " << data.size() << std::endl;

    for( const auto& item: data) {
      os << item.id_s << "\t" << item.id_o << std::endl;
    }
  };
  // ************************************************************* HMM::parser
  static void read( std::istream& is, Data& data )
  {
    data.clear();
    Item item;

    // Lit en omettant les lignes commençant par '#'
    std::string line;
    while (!is.eof()) {
      std::getline( is, line );
      if( line.front() != '#' ) {
	std::stringstream iss( line );
	iss >> item.id_s >> item.id_o;
	
	// en testant !eof, on évite de lire le dernier endl comme un float...
	if( !is.eof()) {
	  data.push_back( item );
	}
      }
    }
  };
};
// *********************************************************************** HMM
}; // namespace Trajectory

#endif // HMM_TRAJECTORY_HPP_HPP
