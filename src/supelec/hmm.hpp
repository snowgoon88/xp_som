#pragma once

#include <cstdlib>
#include <type_traits>
#include <string>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <sstream>

namespace bica {
  namespace random {

    inline double uniform(double min,double max) {
      return min + (max-min)*(std::rand()/(RAND_MAX+1.0));
    }

    /**
     * @return a random integer in [0,max[
     */
    template<typename VALUE>
    inline VALUE uniform(VALUE max) {
      return (VALUE)(max*(std::rand()/(RAND_MAX+1.0)));
    }


    /**
     * @param p in [0,1]
     * @return true with the probability proba.
     */
    inline bool proba(double p) {
      return random::uniform(0,1)<p;
    }

    inline double normal(double std) {
      double u1 = uniform(0.0, 1.0);
      double u2 = uniform(0.0, 1.0);
      double z1 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
      return z1 * std;
    }

  }

  namespace hmm {
    /**
     * T et O sont des fonction qui prennent un entier en paramètre
     */
    typedef std::function<int(int)>    T;
    typedef std::function<double(int)> O;

    // random transitions entre 0 et nb_states
    T uniform(unsigned int nb_states) {
      return [nb_states](int) -> int {return random::uniform(nb_states);};
    }

    // random observations entre 0 et 1
    O uniform() {
      return [](int) -> double {return random::uniform(0,1);};
    }

    // proba (epsilon)   de renvoyer 'uniform_fun'
    //       (1-epsilon)             'fun'
    T epsilon(const T& fun, const T& uniform_fun, double epsilon) {
      return [epsilon,fun,uniform_fun](int s) -> int {
	if(random::proba(epsilon)) return uniform_fun(s);
	else                       return fun(s);
      };
    }
    O epsilon(const O& fun, double epsilon) {
      return [epsilon,fun](int s) -> double {
	if(random::proba(epsilon)) return hmm::uniform()(s);
	else                       return fun(s);
      };
    }

    // periodic dernier etat -> 0
    T periodic(unsigned int nb_states) {
      return [nb_states](int s) -> int {
	if(s == int(nb_states-1)) return 0;
	else                 return s+1;
      };
    }

    // O plus un bruit uniforme dans [-amplitude, amplitude]
    O add_noise(const O& fun, double amplitude) {
      return [amplitude,fun](int s) -> double {
	return fun(s) + random::uniform(-amplitude,amplitude);
      };
    }

    // O plus un bruit gaussien de standar deviation 'std'
    O add_gaussian_noise(const O& fun, double std) {
      return [std,fun](int s) -> double {
	return fun(s) + random::normal(std);
      };
    }

    // 0 a partir d'un symbole 1BCDEF, *=uniform, 0
    O from_string(const std::string& seq) {
      return [seq](int s) -> double {
	double res;
	switch(seq[s]){
	case 'A': res = 0.0; break;
	case 'B': res = 0.2; break;
	case 'C': res = 0.4; break;
	case 'D': res = 0.6; break;
	case 'E': res = 0.8; break;
	case 'F': res = 1.0; break;
	case '*': res = random::uniform(0,1); break;
	default : res = 0;
	}
	return res;
      };
    }

    /**
     * HMM deterministe et periodic 
     */
    std::pair<T,O> periodic(const std::string& seq) {
      return std::make_pair(periodic((unsigned int)(seq.size())), from_string(seq));
    }


    // Useful ?
    std::pair<T,O> direct_join(const std::pair<T,O>& hmm1,
			       unsigned int nb_state_hhmm1,
			       const std::pair<T,O>& hmm2,
			       unsigned int nb_state_hhmm2,
			       double pb_1_2, double pb_2_1) {
      auto t = [nb_state_hhmm1,nb_state_hhmm2,hmm1,hmm2,pb_1_2,pb_2_1](int s) -> int {
	if(s == int(nb_state_hhmm1 - 1))
	  if(random::proba(pb_1_2))
	    return nb_state_hhmm1; // Transit to hmm2
	  else
	    return hmm1.first(s);
	else if(s == int(nb_state_hhmm1 + nb_state_hhmm2 - 1)) {
	  if(random::proba(pb_2_1))
	    return 0; // Transit to hmm1
	  else
	    return nb_state_hhmm1 + hmm2.first(s-nb_state_hhmm1);
	}
	else 
	  if(s < int(nb_state_hhmm1))
	    return hmm1.first(s);
	  else
	    return nb_state_hhmm1 + hmm2.first(s-nb_state_hhmm1);
      };
     
      auto o = [nb_state_hhmm1,nb_state_hhmm2,hmm1,hmm2](int s) -> double {
	if(s < int(nb_state_hhmm1)) return hmm1.second(s);
	else                   return hmm2.second(s-nb_state_hhmm1);
      };

      return {t,o};
    }

    /**
     * Suite de 2 HMM : fin de l'au -> debut de l'autre
     */
    std::pair<T,O> concat(const std::pair<T,O>& hmm1, unsigned int nb_state_hhmm1,
			  const std::pair<T,O>& hmm2, unsigned int nb_state_hhmm2) {
      auto t = [nb_state_hhmm1,nb_state_hhmm2,hmm1,hmm2](int s) -> int {
	if(s < int(nb_state_hhmm1-1))
	    return hmm1.first(s);
	else if(s == int(nb_state_hhmm1-1))
	  return nb_state_hhmm1;
	else if(s == int(nb_state_hhmm1+nb_state_hhmm2-1))
	  return 0;
	else
	  return nb_state_hhmm1 + hmm2.first(s-nb_state_hhmm1);
      };

      auto o = [nb_state_hhmm1,nb_state_hhmm2,hmm1,hmm2](int s) -> double {
	if(s < int(nb_state_hhmm1)) return hmm1.second(s);
	else                   return hmm2.second(s-nb_state_hhmm1);
      };

      return {t,o};
    }

    /**
     * Alterne entre 2 HMM
     *  - passe de HMM1 a HMM2 avec proba pb_1_2
     *  - passe de HMM2 a HMM1 avec proba pb_2_1
     * Quand on alterne, on commence n'importe ou dans l'autre HMM
     */
    std::pair<T,O> join(const std::pair<T,O>& hmm1,
			unsigned int nb_state_hhmm1,
			const std::pair<T,O>& hmm2,
			unsigned int nb_state_hhmm2,
			double pb_1_2, double pb_2_1) {
      auto t = [nb_state_hhmm1,nb_state_hhmm2,hmm1,hmm2,pb_1_2,pb_2_1](int s) -> int {
	if(s < int(nb_state_hhmm1))
	  if(random::proba(pb_1_2))
	    return nb_state_hhmm1+random::uniform(nb_state_hhmm2);
	  else
	    return hmm1.first(s);
	else 
	  if(random::proba(pb_2_1))
	    return random::uniform(nb_state_hhmm1);
	  else
	    return nb_state_hhmm1 + hmm2.first(s-nb_state_hhmm1);
      };
     
      auto o = [nb_state_hhmm1,nb_state_hhmm2,hmm1,hmm2](int s) -> double {
	if(s < int(nb_state_hhmm1)) return hmm1.second(s);
	else                   return hmm2.second(s-nb_state_hhmm1);
      };

      return {t,o};
    }


    // Grammar:
    // 
    // HMM := SEQ | ALT | CONCAT | NOISE
    // CONCAT := '+' HMM '&' HMM
    // ALT := '|' <float> HMM <float> HMM
    // NOISE := '!' <float> HMM
    // SEQ := ELEM _SEQ_END
    // SEQ_END := ^ | ELEM _SEQ_END
    // ELEM := 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | '*'

    /**
     * Cree un HMM decrit par une chaine de caracter en utilisant la grammaire
     * ci-dessus.
     *
     * @return pair< pair<function etat, function obs>, nb_states>
     */
    std::pair<std::pair<T,O>,unsigned int> make(std::istream& is) {
      char c;
      bool stop;
      std::string s;
      double sigma;
      double p12,p21;
      std::pair<std::pair<T,O>,unsigned int> hmm,hmm1,hmm2;
      is >> c;

      switch(c) {
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case '*':
	s = c;
	stop = false;
	do {
	  is.get(c);
	  if(is.eof())
	    stop = true;
	  else if((c > 'F' || c < 'A') && (c != '*')) {
	    stop = true;
	    is.putback(c);
	  }
	  else 
	    s += c;
	} while(!stop);
	// Un HMM periodic
	return {periodic(s), (unsigned int) (s.size())};
	break;
      case '!':
	is >> sigma;
	hmm = make(is);
	// Un HMM avec un bruit Gaussien sur observations
	return {
	  {hmm.first.first, 
	      bica::hmm::add_gaussian_noise(hmm.first.second,sigma)},
	    hmm.second};
	break;
      case '|':
	is >> p12;
	hmm1 = make(is);
	is >> p21;
	hmm2 = make(is);
	// Alterne entre 2 HMM avec les proba donnees
	return {join(hmm1.first,hmm1.second,
		     hmm2.first,hmm2.second,
		     p12,p21),
	    hmm1.second+hmm2.second};
	break;
      case '+':
	hmm1 = make(is);
	is >> c;
	if(c != '&')
	  throw std::runtime_error("HMM parse error");
	hmm2 = make(is);
	// Suite des deux HMM
	return {concat(hmm1.first,hmm1.second,
		     hmm2.first,hmm2.second),
	    hmm1.second+hmm2.second};
	break;
      default:
	throw std::runtime_error("HMM parse error");
      }
    }
    
    /**
     * Cree un HMM decrit par une chaine de caracter en utilisant la grammaire
     * ci-dessus.
     *
     * @return pair< pair<function etat, function obs>, nb_states>
     */
    std::pair<std::pair<T,O>, unsigned int> make(const std::string& s) {
      std::istringstream is(s);
      return make(is);
    }
    
  }
}
