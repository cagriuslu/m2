#ifndef M2_MIXSONG_H
#define M2_MIXSONG_H

#include "../Synth.h"
#include "../Rational.h"
#include <Synth.pb.h>
#include <vector>

namespace m2::synth {
	template <typename ForwardIterator, unsigned SampleRate = 48000>
	void mix_song(ForwardIterator first, ForwardIterator last, const pb::SynthSong& song) {
		static_assert(std::is_same<AudioSample, std::remove_cvref_t<decltype(*first)>>(), "ForwardIterator does not point to AudioSample or derivative");
		static_assert(std::is_same<AudioSample, std::remove_cvref_t<decltype(*last)>>(), "ForwardIterator does not point to AudioSample or derivative");

		for (const auto& track : song.tracks()) {
			mix_track(first, last, track, song.bpm());
		}
	}
}

#endif //M2_MIXSONG_H
