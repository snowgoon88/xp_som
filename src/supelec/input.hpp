#pragma once

#include <iterator>
#include <functional>
#include <algorithm>
#include <utility>

#include "hmm.hpp"

namespace bica {
  
  namespace sampler {
    
    
    class Base {
    public:
      virtual ~Base() {};
      virtual double input() const = 0;
      virtual int input_id() const = 0;
      virtual void shift() = 0;
    };

    /**
     * Sample un HMM en gardant en memoire le state courant.
     */
    class HMM : public Base {
    private:
      int state;
      hmm::T toss_next_state;
      hmm::O toss_observation;
    public:

      template<typename TOSS_S,typename TOSS_O>
      HMM(const TOSS_S& ts, const TOSS_O& to)
	: state(0), toss_next_state(ts), toss_observation(to) {}
      HMM(const HMM& cp) = default;
      HMM& operator=(const HMM& cp) = default;

      virtual double input() const override {
	return toss_observation(state);
      }

      virtual int input_id() const override {
	return state;
      }

      virtual void shift() override {
	state = toss_next_state(state);
      }
    };


  }
}
