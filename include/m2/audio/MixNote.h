#pragma once
#include "Synth.h"
#include <m2/Error.h>
#include <type_traits>
#include <m2/Math.h>

namespace m2::synth {
	template <typename ForwardIterator, unsigned SampleRate = 48000>
	void mix_note(ForwardIterator first, ForwardIterator last, const pb::SoundWaveShape& note_shape, float note_frequency, float note_volume, float track_volume, float track_balance) {
		static_assert(std::is_same<AudioSample, std::remove_cvref_t<decltype(*first)>>(), "ForwardIterator does not point to AudioSample or derivative");
		static_assert(std::is_same<AudioSample, std::remove_cvref_t<decltype(*last)>>(), "ForwardIterator does not point to AudioSample or derivative");
		if (note_shape != pb::NOISE) {
			if (note_frequency < 0.0f || 24000.0f < note_frequency) {
				throw M2_ERROR("Frequency out-of-bounds");
			}
		}

		const Rational frequency{note_frequency};
		auto sample = [=](Rational t) -> SynthSample {
			auto foc = (frequency * t).mod(Rational::one()).to_float();
			switch (note_shape) {
				case pb::SINE:
					return sinf(PI_MUL2 * foc);
				case pb::SQUARE:
					return foc < 0.5f ? 1.0f : -1.0f;
				case pb::TRIANGLE:
					return foc < 0.25f ? (4.0f * foc) : (foc < 0.75f ? (-4.0f * foc + 2.0f) : (4.0f * foc - 4.0f));
				case pb::SAWTOOTH:
					return foc < 0.5f ? (2.0f * foc) : (2.0f * foc - 2.0f);
				case pb::NOISE:
					return 2.0f * RandomF() - 1.0f;
				default:
					throw M2_ERROR("Unknown sound wave shape");
			}
		};

		const float left_volume = note_volume * track_volume * std::fabs((1.0f - track_balance) / 2.0f);
		const float right_volume = note_volume * track_volume * std::fabs(1.0f + track_balance / 2.0f);
		const auto t_step = Rational{1,1} / SampleRate; // Step size in time axis
		for (Rational t; first != last; ++first, t += t_step) {
			auto f = sample(t);
			first->l += f * left_volume;
			first->r += f * right_volume;
		}
	}
}
