#ifndef M2_SYNTH_H
#define M2_SYNTH_H

#include "Audio.h"
#include "Rational.h"
#include "M2.h"
#include <Synth.pb.h>
#include <vector>

namespace m2 {
	using SynthSample = float;
	using SynthBpm = uint32_t;

	namespace synth {
		/// Returns the number of samples for the given number of beats for a given BPM.
		/// Integer division is used internally, thus rounding occurs towards down.
		size_t beat_sample_count(SynthBpm bpm, const pb::Rational& beats, unsigned sample_rate = 48000);
		size_t beat_step_count(const pb::Rational& beats, int64_t max_denominator);

		Rational track_beat_count(const pb::SynthTrack& track);
		int64_t track_max_denominator(const pb::SynthTrack& track);
		size_t track_step_count(const pb::SynthTrack& track);
		size_t track_sample_count(SynthBpm bpm, const pb::SynthTrack& track, unsigned sample_rate = 48000);

		Rational song_beat_count(const pb::SynthSong& song);
		int64_t song_max_denominator(const pb::SynthSong& song);
		size_t song_step_count(const pb::SynthSong& song);
		size_t song_sample_count(const pb::SynthSong& song, unsigned sample_rate = 48000);

		namespace internal {
			void validate(const pb::SynthSound& sound, float frequency);
			void validate(const pb::SynthNote& note);
			void validate(const pb::SynthTrack& track, SynthBpm bpm);
			void validate(const pb::SynthSong& song);
		}
	}
}

#include "synth/GenerateSound.h"
#include "synth/GenerateTrack.h"
#include "synth/GenerateSong.h"
#include "synth/Interleave.h"

#endif //M2_SYNTH_H
