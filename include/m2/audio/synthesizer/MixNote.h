#pragma once
#include "Detail.h"
#include <m2/Math.h>
#include <m2/Error.h>
#include <Synth.pb.h>
#include <type_traits>

namespace m2::audio::synthesizer {
	template <typename ForwardIterator, unsigned SampleRate = 48000>
	void MixNote(ForwardIterator first, ForwardIterator last, const pb::SoundWaveShape& noteShape,
			const float noteFrequency, const float noteVolume, const float trackVolume, const float trackBalance) {
		static_assert(std::is_same<AudioSample, std::remove_cvref_t<decltype(*first)>>(), "ForwardIterator does not belong to an AudioSample or a derivative");
		static_assert(std::is_same<AudioSample, std::remove_cvref_t<decltype(*last)>>(), "ForwardIterator does not belong to an AudioSample or a derivative");

		if (noteShape != pb::NOISE) {
			if (noteFrequency < 0.0f || 24000.0f < noteFrequency) {
				throw M2_ERROR("Frequency out-of-bounds");
			}
		}

		const Rational frequency{noteFrequency};
		auto sample = [=](const Rational t) -> SampleType {
			const auto foc = (frequency * t).mod(Rational::one()).to_float();
			switch (noteShape) {
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

		const float left_volume = noteVolume * trackVolume * std::fabs((1.0f - trackBalance) / 2.0f);
		const float right_volume = noteVolume * trackVolume * std::fabs(1.0f + trackBalance / 2.0f);
		const auto t_step = Rational{1,1} / SampleRate; // Step size in time axis
		for (Rational t; first != last; ++first, t += t_step) {
			auto f = sample(t);
			first->l += f * left_volume;
			first->r += f * right_volume;
		}
	}
}
