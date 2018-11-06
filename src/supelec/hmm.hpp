#pragma once

#include <cstdlib>
#include <type_traits>
#include <string>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <numeric> // std::accumulate

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

    /**
     * PO is a density of probabilities for O
     */
    using PO = std::map<std::string,double>;

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
        //std::cout << "\n__PERIODIC with s=" << s << std::endl;
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

    // (private) return a double from a char O
    double _from_string(const char& c) {
      double res;
      switch(c) {
        case 'A': res = 0.0; break;
	case 'B': res = 0.2; break;
	case 'C': res = 0.4; break;
        case 'c': res = 0.5; break;
	case 'D': res = 0.6; break;
	case 'E': res = 0.8; break;
	case 'F': res = 1.0; break;
	case '*': res = random::uniform(0,1); break;
	default : res = 0;
	}
      return res;
    }
    
    // 0 a partir d'un symbole ABCDEF, *=uniform, 0
    O from_string(const std::string& seq) {
      return [seq](int s) -> double {
        //std::cout << "__FROM_STRING with s=" << s  << std::endl;
        return _from_string(seq[s]);
      };
    }

    // O a partir d'une map de <proba,string>, i;e. density
    O from_map( const PO& density ) {
      return [density](int s) -> double {
        double dsum = 0.0;
        double proba = random::uniform(0,1);
        for( auto& po: density) {
          // probability sum
          dsum += po.second;
          if (proba <= dsum ) {
            return _from_string( po.first[s] );
          }
        }
        // default returns 0
        return _from_string( '0' );
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
        //std::cout << "\n__CONCAT with s=" << s << std::endl;
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

    /**
     * at the end of hmm_start, fork to one of the HMM in the list.
     * at the end of a sub_HMM, jump to the end of the block
     * i.e: state -> sum(nb_states)
     */
    std::pair<T,O> fork( const std::pair<T,O>& hmm_start,
                         unsigned int nb_state_start,
                         std::vector<std::pair<T,O>> l_hmm,
                         std::vector<unsigned int>  l_nb_state,
                         std::vector<double>        l_proba ) {
      
      auto t = [hmm_start,nb_state_start,
                l_hmm,l_nb_state,l_proba](int s) -> int {
        //std::cout << "\n__FORK with s=" << s  << std::endl;
        if (s < nb_state_start-1) {
          // still in hmm_start
          //std::cout << "  in start" << std::endl;
          return hmm_start.first(s);
        }
        else if (s == nb_state_start-1) {
          // end of hmm_start, need to fork
          //std::cout << "  end of hmm_start" << std::endl;
          double psum = 0.0;
          double proba = random::uniform(0,1);
          unsigned int nb_sum = 0;
          for( unsigned int i = 0; i < l_hmm.size(); ++i) {
            psum += l_proba[i];
            if (proba <= psum) {
              //std::cout << "  use hmm[" << i << "]" << std::endl;
              return nb_state_start + nb_sum;
            }
            nb_sum += l_nb_state[i];
          }
          // default
          //std::cout << "  **DEF" << std::endl;
          return nb_state_start;
        }
        else {
          // find the proper HMM
          //std::cout << "  s >= nb_state_start as s=" << s << std::endl;
          unsigned int nb_sum = 0;
          for( unsigned int i = 0; i < l_hmm.size(); ++i) {
            nb_sum += l_nb_state[i];
            if (s < int(nb_state_start+nb_sum-1)) {
              // in this HMM
              //std::cout << "  use hmm[" << i << "]" << std::endl;
              return nb_state_start+nb_sum-l_nb_state[i]+l_hmm[i].first(s-nb_state_start-nb_sum+l_nb_state[i]);
            }
            else if (s == int(nb_state_start+nb_sum-1)) {
              // at the end
              //std::cout << "  at end of " << i << std::endl;
              //return nb_state_start+std::accumulate( l_nb_state.begin(), l_nb_state.end(), 0 );
              // go back to start
              return 0;
            }
          }
        }
        //std::cout << "  **DEF" << std::endl;
        return nb_state_start+std::accumulate( l_nb_state.begin(), l_nb_state.end(), 0 );
      };

      auto o = [hmm_start,nb_state_start,
                l_hmm,l_nb_state,l_proba](int s) -> double {
        //std::cout << "  from_list 0 with s=" << s << std::endl;
        if (s < int(nb_state_start)) {
          // in hmm_start
          return hmm_start.second(s);
        }
        else {
          // in one of the sub HMM
          unsigned int nb_sum = 0;
          for( unsigned int i = 0; i < l_hmm.size(); ++i) {
            //std::cout << "  nb_sum=" << nb_sum << " and l_nb_state[" << i << "]=" << l_nb_state[i] << std::endl;
            if (s < int(nb_state_start+l_nb_state[i]+nb_sum)) { // in this HMM
              //std::cout << "  s=" << s << " => hmm[" << i << "]" << std::endl;
              return l_hmm[i].second(s-nb_sum-nb_state_start);
            }
            nb_sum += l_nb_state[i];
          }
          //std::cout << "  **DEF" << std::endl;
          return 0.0;
        }
      };
      
      return {t,o};
    }

    // Grammar:
    // 
    // HMM := SEQ | FORK | ALT | CONCAT | NOISE
    // FORK := [ HMM , _END_FORK
    // _END_FORK := float HMM , _END_FORK | ]
    // STO := '<' <float> HMM _STO_END
    // _STO_END := <float> HMM _STO_END | '>'
    // CONCAT := '+' HMM '&' HMM
    // ALT := '|' <float> HMM <float> HMM
    // NOISE := '!' <float> HMM
    // SEQ := ELEM _SEQ_END
    // SEQ_END := ^ | ELEM _SEQ_END
    // ELEM := 'A' | 'B' | 'C' | 'c' |'D' | 'E' | 'F' | '*'

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

      PO densityO;
      double proba;
      std::pair<std::pair<T,O>,unsigned int> hmmO;
      
      is >> c;

      // fork
      if (c == '[') {
        //std::cout << "__read FORK" << std::endl;
        std::pair<std::pair<T,O>,unsigned int> hmm_start;
        std::pair<std::pair<T,O>,unsigned int> hmm_sub;
        std::vector<std::pair<T,O>> l_hmm;
        std::vector<unsigned int>  l_nb_state;
        std::vector<double>        l_proba;
        hmm_start = make(is);

        is >> c;
        do {
          is >> proba;
          hmm_sub = make(is);

          l_hmm.push_back( hmm_sub.first );
          l_nb_state.push_back( hmm_sub.second );
          l_proba.push_back( proba );

          is >> c;
          //std::cout << "c=" << c << "-" << std::endl;
        } while ( c != ']' );
        return { bica::hmm::fork( hmm_start.first, hmm_start.second,
                                  l_hmm, l_nb_state, l_proba ),
            hmm_start.second + std::accumulate( l_nb_state.begin(),
                                                l_nb_state.end(),
                                                0 ) };
      }
      
      // density of proba for O
      else if (c == '<') {
        densityO.clear();

        do {
          is >> proba;
          is >> s;
          densityO[s] = proba;
          //std::cout << "__densityO" << std::endl;
          // for( auto& po: densityO) {
          //   std::cout << "  " << po.first << " : " << po.second << std::endl;
          // }
          is >> c;
          //std::cout << "c=" << c << "-" << std::endl;
        } while ( c != '>' );
        return { {bica::hmm::uniform(1), bica::hmm::from_map( densityO )}, 1};
      }
      
      switch(c) {
      case 'A':
      case 'B':
      case 'C':
      case 'c':
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
