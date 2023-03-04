#ifndef M2_GENERATESOUND_H
#define M2_GENERATESOUND_H

#include "../Synth.h"
#include <m2/Exception.h>
#include <type_traits>

namespace m2::synth {
	/// Returns the fraction of cycle of the next sample.
	/// mix_factor = How many signals already exist in the buffer
	template <typename ForwardIterator, unsigned SampleRate = 48000>
	Rational generate_sound(ForwardIterator first, ForwardIterator last, const pb::SynthSound& sound, float frequency, float volume = 1.0f, Rational next_fraction_of_cycle = {}) {
		static_assert(std::is_same<SynthSample, std::remove_cvref_t<decltype(*first)>>(), "ForwardIterator does not point to SynthesizerSample or derivative");
		static_assert(std::is_same<SynthSample, std::remove_cvref_t<decltype(*last)>>(), "ForwardIterator does not point to SynthesizerSample or derivative");
		internal::validate(sound, frequency);

		const Rational frequency_r{frequency};
		auto fraction_of_cycle = [=](Rational t) -> Rational {
			return (frequency_r * t).mod(Rational{1,1});
		};

		MAYBE auto raw_sample = [=](Rational t) -> float {
			auto foc = fraction_of_cycle(t).to_float();
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

		auto sample = [=](Rational t) -> SynthSample {
			auto rs = raw_sample(t);
			return sound.amplitude() * volume * rs;
		};

		auto initial_fraction_of_cycle = (Rational{sound.fraction_of_cycle()} + next_fraction_of_cycle).mod(Rational{1, 1}); // Initial fraction of cycle
		auto t = initial_fraction_of_cycle / frequency_r; // Initial `t` of the signal
		auto t_step = Rational{1,1} / SampleRate; // Step size in time axis
		for (; first != last; ++first, t += t_step) {
			*first += sample(t);
		}
		return fraction_of_cycle(t);
	}
}

#endif //M2_GENERATESOUND_H
