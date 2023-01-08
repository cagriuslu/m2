#ifndef M2_INTERLEAVE_H
#define M2_INTERLEAVE_H

#include "../Synth.h"
#include <type_traits>

namespace m2::synth {
	template <typename InputForwardIterator, typename OutputForwardIterator>
	void interleave(InputForwardIterator left_first, InputForwardIterator right_first, OutputForwardIterator out_first, OutputForwardIterator out_last) {
		static_assert(std::is_same<SynthSample, std::remove_cvref_t<decltype(*left_first)>>(), "InputForwardIterator does not point to SynthesizerSample or derivative");
		static_assert(std::is_same<SynthSample, std::remove_cvref_t<decltype(*right_first)>>(), "InputForwardIterator does not point to SynthesizerSample or derivative");
		static_assert(std::is_base_of<AudioSample, std::remove_cvref_t<decltype(*out_first)>>(), "OutputForwardIterator does not point to AudioSample or derivative");
		static_assert(std::is_base_of<AudioSample, std::remove_cvref_t<decltype(*out_last)>>(), "OutputForwardIterator does not point to AudioSample or derivative");

		for (; out_first != out_last; ++out_first, ++left_first, ++right_first) {
			out_first->l = *left_first;
			out_first->r = *right_first;
		}
	}
}

#endif //M2_INTERLEAVE_H
