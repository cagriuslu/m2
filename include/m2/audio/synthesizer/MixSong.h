#pragma once
#include "MixTrack.h"
#include <Synth.pb.h>

namespace m2::audio::synthesizer {
	template <typename ForwardIterator, unsigned SampleRate = 48000>
	void MixSong(ForwardIterator first, ForwardIterator last, const pb::SynthSong& song) {
		for (const auto& track : song.tracks()) {
			MixTrack<ForwardIterator,SampleRate>(first, last, track, song.bpm());
		}
	}
}
