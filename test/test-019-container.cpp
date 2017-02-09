/* -*- coding: utf-8 -*- */

/** 
 * Test FixedQueue
 */

#include <fixedqueue.hpp>
#include <iostream>

// **************************************************************** FixedQueue
void test_fq_creation()
{
  FixedQueue<int> fq(5);
  std::cout << "__CREATION" << std::endl;
  std::cout << fq.str_display() << std::endl;

  std::cout << "__ADD 2 elements" << std::endl;
  fq.push_front( 2 );
  fq.push_front( 4 );
  std::cout << fq.str_display() << std::endl;

  std::cout << "__ADD 4 more" << std::endl;
  fq.push_front( 5 );
  fq.push_front( 6 );
  fq.push_front( 7 );
  fq.push_front( 10 );
  std::cout << fq.str_display() << std::endl;
}  

// ***************************************************************************
// ********************************************************************** main
int main(int argc, char *argv[])
{
  test_fq_creation();
  return 0;
}

