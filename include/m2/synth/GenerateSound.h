#ifndef M2_GENERATESOUND_H
#define M2_GENERATESOUND_H

#include "../Synth.h"
#include <m2/Exception.h>
#include <type_traits>

namespace m2::synth {
	/// Returns the phase of the last sample.
	/// mix_factor = How many signals already exist in the buffer
	template <typename ForwardIterator, unsigned SampleRate = 48000>
	float generate_sound(ForwardIterator first, ForwardIterator last, unsigned mix_factor, const pb::SynthSound& sound, float frequency, float volume = 1.0f, float last_phase = 0.0f) {
		static_assert(std::is_same<SynthSample, std::remove_cvref_t<decltype(*first)>>(), "ForwardIterator does not point to SynthesizerSample or derivative");
		static_assert(std::is_same<SynthSample, std::remove_cvref_t<decltype(*last)>>(), "ForwardIterator does not point to SynthesizerSample or derivative");
		internal::validate(sound, frequency);
		// TODO floating point operations in this function accumulate and disturb the sound

		auto fraction_of_cycle = [=](float t) -> float {
			return fmodf(frequency * t, 1.0f);
		};

		MAYBE auto raw_sample = [=](float t) -> float {
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
				case pb::NOISE:
					return 2.0f * randf() - 1.0f;
				default:
					throw M2ERROR("Unknown sound wave shape");
			}
		};

		auto sample = [=](float t) -> SynthSample {
			auto rs = raw_sample(t);
			return sound.amplitude() * volume * rs;
		};

		auto initial_phase = fmodf(fmodf(last_phase + sound.phase(), PI_MUL2) + PI_MUL2, PI_MUL2); // Initial phase of the signal, combination of sound.phase() and initial_phase. Add 2PI and re-modulate in case the first result is negative.
		auto initial_t = initial_phase / PI_MUL2 / frequency; // Initial `t` of the signal
		auto t = initial_t;
		auto t_step = 1.0f / SampleRate;

		if (mix_factor == 0) {
			for (; first != last; ++first, t += t_step) {
				*first = sample(t);
			}
		} else {
			for (; first != last; ++first, t += t_step) {
				auto old_sample = *first;
				auto old_sample_scaled_up = old_sample * static_cast<float>(mix_factor);
				auto new_sample = sample(t);
				auto mixed_sample_scaled_up = old_sample_scaled_up + new_sample;
				auto mixed_sample = mixed_sample_scaled_up / static_cast<float>(mix_factor + 1);
				*first = mixed_sample;
			}
		}
		return PI_MUL2 * fraction_of_cycle(t);
	}
}

#endif //M2_GENERATESOUND_H
