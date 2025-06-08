#pragma once
#include "MixTrack.h"
#include <Synth.pb.h>

namespace m2::audio::synthesizer {
	template <typename SampleIterator, unsigned SampleRate = 48000>
	void MixSong(const SampleIterator first, const SampleIterator last, const pb::SynthSong& song) {
		for (const auto& track : song.tracks()) {
			MixTrack<SampleIterator,SampleRate>(first, last, track, song.bpm());
		}
	}
}
