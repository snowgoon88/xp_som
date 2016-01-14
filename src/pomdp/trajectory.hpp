/* -*- coding: utf-8 -*- */

#ifndef POMDP_TRAJECTORY_HPP
#define POMDP_TRAJECTORY_HPP

/** 
 * Trajectoire::POMDP is a sequence of Trajectory::POMDP::Item(s,o,a,s',o',r).
 */

// **************************************************************** Trajectory
namespace Trajectory
{
// ***************************************************************************
// ********************************************************************* POMDP
// ***************************************************************************
class POMDP
{
public:
  // ************************************************************* POMDP::Item
  typedef struct {
    unsigned int id_s;
    unsigned int id_o;
    unsigned int id_a;
    unsigned int id_next_s;
    unsigned int id_next_o;
    double r;
  } Item;
  // ************************************************************* POMDP::Data
  typedef std::vector<Item> Data;
  // *********************************************************** POMDP::parser
  static void read( std::istream& is, Data& data )
  {
    data.clear();
    Item item;
    while( !is.eof() ) {
      is >> item.id_s >> item.id_o >> item.id_a >> item.id_next_s >> item.id_next_o >> item.r;
      // en testant !eof, on évite de lire le dernier endl comme un float...
      if ( !is.eof()) {
	data.push_back( item );
      }
    }
  }
};
// ********************************************************************* POMDP
};


#endif // POMDP_TRAJECTORY_HPP
