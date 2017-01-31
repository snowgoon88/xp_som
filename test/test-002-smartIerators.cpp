/* -*- coding: utf-8 -*- */

/** 
 * Try : iterator and lambda function to solve iterating to some 
 *       inner members.
 */
#include <string>
#include <list>
#include <iostream>
#include <functional>

// ********************************************************************** Data
class Data
{
public:
  Data( int val, std::string msg ) : one(val), two(msg)
  {}
  //private:
  int one;
  std::string two;
};
// ******************************************************************** Member
template< typename T >
class Member
{
public:
  using Inside = typename T::value_type;
  Member( T& container ) : _truc( container.front() )
  {}
  
  Inside& _truc;
};

// ********************************************************************** Doer
template< typename T >
class Doer
{
public:
  //using TData = typename T::value_type;
  using Func = std::function< int (const typename T::value_type& )  >;
  Doer( T& container, Func fun=nullptr ) :
	_data(container), _globfun(fun)
  {}

  void run_forit()
  {
	for ( auto i = _data.begin(); i != _data.end(); ++i ) {
	  std::cout << i->one << std::endl;
	}	
  }
  template<typename Func>
  void run_forit( Func fun )
  {
	for ( auto i = _data.begin(); i != _data.end(); ++i ) {
	  std::cout << fun(*i) << std::endl;
	}	
  }
  void run_global()
  {
	if( _globfun ) {
	  for ( auto i = _data.begin(); i != _data.end(); ++i ) {
		std::cout << _globfun(*i) << std::endl;
	  }
	}
	else {
	  std::cout << "_globfun not defined" << std::endl;
	}
  }
private:
  const T& _data;
  Func _globfun;
};

template<typename T,typename F>
void test_iteration(const T& it_begin, const T& it_end, F fun)
{
  for( auto it=it_begin; it != it_end; ++it ) {
	std::cout << fun(it) << std::endl;
  }
}
// ********************************************************************** main
int main(int argc, char *argv[])
{
  using MyContainer = std::list<Data>;
  MyContainer d = {{1,"UN"},{2,"DEUX"},{3,"TROIS"}};
  MyContainer::value_type d1 = {0,"zero"};
  auto funtwo = [] (const Data& e) {return e.two;};
  auto funone = [] (const Data& e) {return e.one;};
  //Only c++14 auto funthree = [] (const auto& e)->int { return e.one;};
  
  Doer<MyContainer> actif(d, funone);

  std::cout << "__FORIT" << std::endl;
  actif.run_forit();
  std::cout << "__GLOBAL" << std::endl;
  actif.run_global();
  std::cout << "__FORIT Lambda with string" << std::endl;
  actif.run_forit( funtwo );

  using IntContainer = std::list<int>;
  std::cout << "__LIST of INT" << std::endl;
  IntContainer di = {11,12,13};
  Doer<IntContainer> displayer( di, [] (const int& e) {return e;} );
  std::cout << "__GLOBAL" << std::endl;
  displayer.run_global();


  Member<MyContainer> member( d );
  std::cout << "__First in MyContainer" << std::endl;
  std::cout << member._truc.two << std::endl;

  std::cout << "__IT INT" << std::endl;
  test_iteration<int,std::function<int(int)>>( 0, 4, [] (int i) {return i;} );
  std::cout << "__IT DATA" << std::endl;
  test_iteration<MyContainer::iterator,std::function<std::string(MyContainer::iterator)>>( d.begin(), d.end(), [] (MyContainer::iterator it) {return it->two;} );
  return 0;
}




