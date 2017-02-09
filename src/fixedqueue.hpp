/* -*- coding: utf-8 -*- */

#ifndef FIXEDQUEUE_HPP
#define FIXEDQUEUE_HPP

/** 
 * A FIFO queue that has a maximum size.
 * If pushing_front a new element makes size too big, elements are popped_back.
 */
#include <list>
#include <sstream>

// ***************************************************************************
// **************************************************************** FixedQueue
// ***************************************************************************
template<class T>
class FixedQueue : public std::list<T>
{
public:
  // **************************************************** FixedQueue::creation
  FixedQueue( const typename std::list<T>::size_type& max_size ) :
    std::list<T>(), _max_size(max_size)
  {
  }
  // ************************************************** FixedQueue::push_front
  void push_front( const T& val)
  {
    std::list<T>::push_front(val);
    while( std::list<T>::size() > _max_size ) {
      std::list<T>::pop_back();
    }
  }
  // ********************************************************* FixedQueue::str
  std::string str_display()
  {
    std::stringstream disp;
    disp << "{";
    for (auto it = this->begin(); it != this->end(); ++it) {
      disp << *it << ", ";
    }
    disp << "}";

    return disp.str();
  }
private:
  // *************************************************** FixedQueue::attributs
  const typename std::list<T>::size_type _max_size;
};

#endif // FIXEDQUEUE_HPP
