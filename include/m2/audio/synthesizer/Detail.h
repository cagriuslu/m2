#pragma once
#include <m2/math/Rational.h>
#include <Synth.pb.h>
#include <cstdint>

namespace m2::audio::synthesizer {
	constexpr uint32_t gDefaultAudioSampleRate = 48000;

	using BeatsPerMinute = uint32_t;
	using SampleType = float;

	/// This struct is used by AudioManager for reinterpret cast the sample array. Do not modify the ordering and the
	/// packing of the struct.
	struct AudioSample {
		SampleType l{}, r{};

		AudioSample operator+(const AudioSample& rhs) const;
		AudioSample& operator+=(const AudioSample& rhs);
	};

	/// Returns the number of samples for the given number of beats for a given BPM.
	/// Internally, integer division is used, thus rounding occurs towards down.
	size_t NoteSampleCount(const pb::Rational& beats, BeatsPerMinute bpm, unsigned sample_rate);
	size_t NoteSampleCount(const Rational& beats, BeatsPerMinute bpm, unsigned sample_rate);

	/// Returns the number of beats in the track
	Rational TrackBeatCount(const pb::SynthTrack& track);
	/// Returns the number of samples for the given track for a given BPM
	size_t TrackSampleCount(BeatsPerMinute bpm, const pb::SynthTrack& track, unsigned sample_rate);

	/// Returns the number of samples for the given song
	size_t SongSampleCount(const pb::SynthSong& song, unsigned sample_rate);
}
