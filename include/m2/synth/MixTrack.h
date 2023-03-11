#ifndef M2_MIXTRACK_H
#define M2_MIXTRACK_H

#include "../Synth.h"
#include "../Rational.h"
#include <Synth.pb.h>
#include <vector>
#include <type_traits>

namespace m2::synth {
	template <typename ForwardIterator, unsigned SampleRate = 48000>
	void mix_track(ForwardIterator first, ForwardIterator last, const pb::SynthTrack& track, SynthBpm bpm) {
		static_assert(std::is_same<AudioSample, std::remove_cvref_t<decltype(*first)>>(), "ForwardIterator does not point to AudioSample or derivative");
		static_assert(std::is_same<AudioSample, std::remove_cvref_t<decltype(*last)>>(), "ForwardIterator does not point to AudioSample or derivative");
		if (bpm == 0 || 10000 < bpm) {
			throw M2ERROR("BPM out-of-bounds");
		}

		for (const auto& note : track.notes()) {
			if (to_float(note.start_beat()) < 0.0f) {
				throw M2ERROR("Start out-of-bounds");
			}
			if (to_float(note.duration()) < 0.0f) {
				throw M2ERROR("Duration out-of-bounds");
			}

			auto note_first = first + note_sample_count(note.start_beat(), bpm, SampleRate);
			auto note_length = note_sample_count(note.duration(), bpm, SampleRate); // in samples
			auto buffer_length = static_cast<size_t>(last - note_first); // samples left in buffer
			auto note_last = note_first + std::min(note_length, buffer_length);

			mix_note(note_first, note_last, track.shape(), note.frequency(), note.volume(), track.volume(), track.balance());
			if (note_last == last) {
				break;
			}
		}
	}
}

#endif //M2_MIXTRACK_H
