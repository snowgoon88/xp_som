/* -*- coding: utf-8 -*- */

/**
 * test-017-plot.cpp
 * 
 * Plot many Curves
 */

#include <gl_plot.hpp>
#include <utils.hpp>

int main(int argc, char *argv[])
{
  GLPlot plot( "Essai de Plot", 800, 600 );

  // Add some curves
  std::cout << "__C1" << std::endl;
  auto c1 = make_unique<Curve>();
  c1->add_sample( {0,0,0} );
  c1->add_sample( {2,1,0} );
  plot.add_curve( std::move(c1) );

  // Add some curves
  std::cout << "__C2" << std::endl;
  auto c2 = make_unique<Curve>();
  c2->set_color( {0,0,1} );
  c2->add_sample( {1,1,0} );
  c2->add_sample( {3,-1,0} );
  plot.add_curve( std::move(c2) );

  // Add some curves
  std::cout << "__C3" << std::endl;
  std::vector<double> xdata{0.5, 1, 1.5, 2.0, 2.5, 3, 3.5};
  std::vector<double> ydata{1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0};
  auto c3 = make_unique<Curve>();
  c3->add_sample( xdata.begin(), xdata.end(),
		  ydata.begin(), ydata.end() );
  c3->set_color( {0,1,0} );
  plot.add_curve( std::move(c3) );

  plot.show();
  
  return 0;
}
