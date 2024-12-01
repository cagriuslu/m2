#pragma once
#include "math/Rational.h"
#include "M2.h"
#include <Synth.pb.h>
#include <vector>

namespace m2 {
	constexpr unsigned DEFAULT_AUDIO_SAMPLE_RATE = 48000;

	using SynthBpm = uint32_t;
	using SynthSample = float;

	struct AudioSample {
		SynthSample l{}, r{};

		inline void mutable_mix(SynthSample _l, SynthSample _r) { l += _l; r += _r; }
	};

	namespace synth {
		/// Returns the number of samples for the given number of beats for a given BPM
		/// Integer division is used internally, thus rounding occurs towards down
		size_t note_sample_count(const pb::Rational& beats, SynthBpm bpm, unsigned sample_rate);

		/// Returns the number of beats in the track
		Rational track_beat_count(const pb::SynthTrack& track);
		/// Returns the number of samples for the given track for a given BPM
		size_t track_sample_count(SynthBpm bpm, const pb::SynthTrack& track, unsigned sample_rate);

		/// Returns the number of samples for the given song
		size_t song_sample_count(const pb::SynthSong& song, unsigned sample_rate);
	}
}

#include "synth/MixNote.h"
#include "synth/MixTrack.h"
#include "synth/MixSong.h"
