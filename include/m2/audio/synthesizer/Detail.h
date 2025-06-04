#pragma once
#include <m2/math/Rational.h>
#include <Synth.pb.h>
#include <cstdint>

namespace m2::audio::synthesizer {
	constexpr uint32_t gDefaultAudioSampleRate = 48000;

	using BeatsPerMinute = uint32_t;
	using SampleType = float;

	struct AudioSample {
		SampleType l{}, r{};

		void MutableMix(const SampleType l_, const SampleType r_) { l += l_; r += r_; }
	};

	/// Returns the number of samples for the given number of beats for a given BPM
	/// Integer division is used internally, thus rounding occurs towards down
	size_t NoteSampleCount(const pb::Rational& beats, BeatsPerMinute bpm, unsigned sample_rate);

	/// Returns the number of beats in the track
	Rational TrackBeatCount(const pb::SynthTrack& track);
	/// Returns the number of samples for the given track for a given BPM
	size_t TrackSampleCount(BeatsPerMinute bpm, const pb::SynthTrack& track, unsigned sample_rate);

	/// Returns the number of samples for the given song
	size_t SongSampleCount(const pb::SynthSong& song, unsigned sample_rate);
}
