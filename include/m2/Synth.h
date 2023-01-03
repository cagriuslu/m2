#ifndef M2_SYNTH_H
#define M2_SYNTH_H

#include "Audio.h"
#include "Exception.h"
#include "M2.h"
#include <Synth.pb.h>
#include <vector>
#include <type_traits>

namespace m2 {
	using SynthSample = int16_t;
	constexpr SynthSample SYNTH_SAMPLE_MAX = INT16_MAX;
	constexpr SynthSample SYNTH_SAMPLE_MIN = INT16_MIN;

	class Synth {
	public:
		/// Returns the phase of the last sample.
		template <typename ForwardIterator, unsigned SampleRate = 48000>
		static float generate(ForwardIterator first, ForwardIterator last, const pb::SyntheticSound& sound, float in_initial_phase = 0.0f) {
			static_assert(std::is_same<SynthSample, std::remove_cvref_t<decltype(*first)>>(), "ForwardIterator does not point to SynthesizerSample or derivative");
			static_assert(std::is_same<SynthSample, std::remove_cvref_t<decltype(*last)>>(), "ForwardIterator does not point to SynthesizerSample or derivative");
			validate(sound);

			auto fraction_of_cycle = [=](float t) -> float {
				return fmodf(sound.frequency() * t, 1.0f);
			};

			MAYBE auto float_sample = [=](float t) -> float {
				auto foc = fraction_of_cycle(t);
				switch (sound.shape()) {
					case pb::SINE:
						return sinf(PI_MUL2 * foc);
					case pb::SQUARE:
						return foc < 0.5f ? 1.0 : -1.0f;
					case pb::TRIANGLE:
						return foc < 0.25f ? (4.0f * foc) : (foc < 0.75f ? (-4.0f * foc + 2.0f) : (4.0f * foc - 4.0f));
					case pb::SAWTOOTH:
						return foc < 0.5f ? (2.0f * foc) : (2.0f * foc - 2.0f);
					default:
						throw M2ERROR("Unknown sound wave shape");
				}
			};

			auto sample = [=](float t) -> SynthSample {
				auto sample_f = sound.amplitude() * float_sample(t);
				if (sample_f <= -1.0f) {
					return SYNTH_SAMPLE_MIN;
				} else if (1.0f <= sample_f) {
					return SYNTH_SAMPLE_MAX;
				} else {
					return (SynthSample) roundf((float) SYNTH_SAMPLE_MAX * sample_f);
				}
			};

			auto initial_phase = fmodf(fmodf(in_initial_phase + sound.phase(), PI_MUL2) + PI_MUL2, PI_MUL2); /// Initial phase of the signal, combination of sound.phase() and initial_phase. Add 2PI and re-modulate in case the first result is negative.
			auto initial_t = initial_phase / PI_MUL2 / sound.frequency(); /// Initial `t` of the signal
			auto t = initial_t;
			auto t_step = 1.0f / SampleRate;

			for (; first != last; ++first, t += t_step) {
				*first = sample(t);
			}
			return PI_MUL2 * fraction_of_cycle(t) - initial_phase;
		}

		template <typename InputForwardIterator, typename OutputForwardIterator>
		static void interleave(InputForwardIterator left_first, InputForwardIterator right_first, OutputForwardIterator out_first, OutputForwardIterator out_last) {
			static_assert(std::is_same<SynthSample, std::remove_cvref_t<decltype(*left_first)>>(), "InputForwardIterator does not point to SynthesizerSample or derivative");
			static_assert(std::is_same<SynthSample, std::remove_cvref_t<decltype(*right_first)>>(), "InputForwardIterator does not point to SynthesizerSample or derivative");
			static_assert(std::is_base_of<AudioSample, std::remove_cvref_t<decltype(*out_first)>>(), "OutputForwardIterator does not point to AudioSample or derivative");
			static_assert(std::is_base_of<AudioSample, std::remove_cvref_t<decltype(*out_last)>>(), "OutputForwardIterator does not point to AudioSample or derivative");

			for (; out_first != out_last; ++out_first, ++left_first, ++right_first) {
				out_first->l = *left_first;
				out_first->r = *right_first;
			}
		}

	private:
		static inline void validate(const pb::SyntheticSound& sound) {
			if (sound.frequency() < 0.0f || 24000.0f < sound.frequency()) {
				throw M2ERROR("Frequency out-of-bounds");
			}
		}
	};
}

#endif //M2_SYNTH_H
