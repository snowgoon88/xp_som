#include "input.hpp"
#include "hmm.hpp"
#include <iomanip>
#include <iostream>
#include <utility>

void print(const std::string& name, bica::sampler::HMM& h, int nb) {
  std::cout << name << " : " << std::fixed;
  for(int i = 0; i < nb; ++i, h.shift())
    std::cout << std::setw(5) << std::setprecision(2) << h.input() << '('
	      << std::setw(2) << h.input_id() << ") ";
  std::cout << std::endl;
  std::cout << std::endl;
}


int main(int argc, char* argv[]) {
  
  std::srand(std::time(0));
  bica::hmm::T t;
  bica::hmm::O o;

  // unpack the std::pair returned by hmm
  std::tie(t,o) = bica::hmm::periodic("ABCDEF");
  bica::sampler::HMM h1(t,o);
  print("h1",h1,30);

  // Soit periodic, soit uniforme
  auto t_eps = bica::hmm::epsilon(t,bica::hmm::uniform(6),.3);
  auto o_eps = bica::hmm::epsilon(o,                      .3);

  bica::sampler::HMM h2(t,    o_eps);
  bica::sampler::HMM h3(t_eps,o    );
  bica::sampler::HMM h4(t_eps,o_eps);
  print("h2",h2,30);
  print("h3",h3,30);
  print("h4",h4,30);

  // periodic avec du bruit uniform
  auto o_noise = bica::hmm::add_noise(o,.05);
  bica::sampler::HMM h5(t,o_noise);
  print("h5",h5,30);

  // periodoc avec bruit gaussien
  auto o_wnoise = bica::hmm::add_gaussian_noise(o,.05);
  bica::sampler::HMM h6(t,o_noise);
  print("h6",h6,30);


  // alterne entre 2 HMM
  bica::hmm::T t1,t2,t3;
  bica::hmm::O o1,o2,o3;

  std::tie(t1,o1) = bica::hmm::periodic("AB");
  std::tie(t2,o2) = bica::hmm::periodic("DEF");
  std::tie(t3,o3) = bica::hmm::join({t1,o1},2,
  				    {t2,o2},3,
   				    .05,.1);
  bica::sampler::HMM h7(t3,o3);
  print("h7",h7,30);
  // Alterne entre les 2 HMM en ajoutant un bruit uniforme aux observations
  bica::sampler::HMM h8(t3,bica::hmm::add_noise(o3,.05));
  print("h8",h8,30);
  

  std::vector<std::string> exprs = {{
      std::string("ABCD"),           // periodic
      "AB*D",                        // periodic, mais avec *=uniform obs
      "+ ABC & DEF",                 // suite de 2 HMM
      "! .05 ABCD",                  // bruit gaussien sur obs
      "+ ! .05 ABC & DEF",           // suite gaussien puis deterministe
      "| .05 ABC .05 DEF",           // alterne entre deux deterministes
      "| .05 ! .05 ABCD .01 *",      // alterne entre 1) Gaussien 2) random O
      "| 0.03 ! 0.05 AAAAAAAAAAF 0.1 | 0.5 A 0.5 F"
                                     // alterne 1) AAAAAAAAAAAF gaussien
                                     //         2) soit A, soit F
     }};
  for(auto& expr : exprs) {
    bica::hmm::T t;
    bica::hmm::O o;

    auto hmm = bica::hmm::make(expr);
    std::tie(t, o) = hmm.first;
    auto nb_states = hmm.second;

    bica::sampler::HMM h(t,o);
    std::cout << "__" << expr << "__ with " << nb_states << " states" << std::endl;
    print(expr,h,100);
  }


  return 0;
}
