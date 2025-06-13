#pragma once
#include "Detail.h"
#include <m2/Meta.h>
#include <m2/Math.h>
#include <m2/Error.h>
#include <Synth.pb.h>
#include <type_traits>

namespace m2::audio::synthesizer {
	template <typename SampleIterator, unsigned SampleRate = 48000>
	void MixNote(const SampleIterator first, const SampleIterator last, const BeatsPerMinute bpm, const pb::SoundWaveShape& noteShape,
			const float trackVolume, const float balance, const std::optional<pb::Envelope>& amplitudeEnv, const pb::SynthNote& note) {
		static_assert(std::is_same<AudioSample, std::remove_cvref_t<decltype(*first)>>(), "SampleIterator does not belong to an AudioSample or a derivative");
		static_assert(std::is_same<AudioSample, std::remove_cvref_t<decltype(*last)>>(), "SampleIterator does not belong to an AudioSample or a derivative");

		if (noteShape != pb::NOISE) {
			if (note.frequency() < 0.0f || 24000.0f < note.frequency()) {
				throw M2_ERROR("Frequency out-of-bounds");
			}
		}

		const auto attackDuration = amplitudeEnv ? Rational{amplitudeEnv->attack_duration()} : Rational::Zero(); // in beats
		const auto decayDuration = amplitudeEnv ? Rational{amplitudeEnv->decay_duration()} : Rational::Zero(); // in beats
		const auto activeDuration = Rational{note.duration()}; // in beats, includes attack + decay + sustain
		const auto releaseDuration = amplitudeEnv ? Rational{amplitudeEnv->release_duration()} : Rational::Zero(); // in beats

		const auto attackNoteLength = NoteSampleCount(attackDuration, bpm, SampleRate); // in samples
		const auto decayNoteLength = NoteSampleCount(decayDuration, bpm, SampleRate); // in samples
		const auto activeNoteLength = NoteSampleCount(activeDuration, bpm, SampleRate); // in samples
		const auto releaseNoteLength = NoteSampleCount(releaseDuration, bpm, SampleRate); // in samples

		const auto sustainVolume = amplitudeEnv ? amplitudeEnv->sustain_volume() : 1.0f;

		const auto SampleScaler = [&attackNoteLength, &decayNoteLength, &activeNoteLength, &releaseNoteLength, sustainVolume](const size_t writtenSampleCount, const SampleType unscaledSample) {
			if (writtenSampleCount < attackNoteLength) {
				// Attack
				return Lerp(0.0f, unscaledSample, F(writtenSampleCount) / F(attackNoteLength));
			} else if (writtenSampleCount < attackNoteLength + decayNoteLength) {
				// Decay
				return Lerp(unscaledSample, unscaledSample * sustainVolume, F(writtenSampleCount - attackNoteLength) / F(decayNoteLength));
			} else if (writtenSampleCount < activeNoteLength) {
				// Sustain
				return unscaledSample * sustainVolume;
			} else {
				// Release
				return Lerp(unscaledSample * sustainVolume, 0.0f, F(writtenSampleCount - activeNoteLength) / F(releaseNoteLength));
			}
		};

		const float left_volume = trackVolume * note.volume() * std::fabs((1.0f - balance) / 2.0f);
		const float right_volume = trackVolume * note.volume() * std::fabs((1.0f + balance) / 2.0f);
		const auto SampleBalancer = [&left_volume, &right_volume](const SampleType unbalancedSample) -> AudioSample {
			return AudioSample{.l = unbalancedSample * left_volume, .r = unbalancedSample * right_volume};
		};

		const Rational frequency{note.frequency()};
		auto CreateUnitSample = [=](const Rational t) -> SampleType {
			const auto foc = (frequency * t).Mod(Rational::One()).ToFloat(); // fraction of (the sound wave) cycle
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

		auto it = first;
		const auto t_step = Rational::One() / SampleRate; // Step size in time axis
		for (Rational t; it != last; ++it, t += t_step) {
			const auto unitSample = CreateUnitSample(t);
			auto sampleScaled = SampleScaler(it - first, unitSample);
			auto sampleBalanced = SampleBalancer(sampleScaled);
			(*it) += sampleBalanced;
		}
	}
}
