#pragma once
#include "Detail.h"
#include "MixNote.h"
#include <m2/math/Rational.h>
#include <m2/Error.h>
#include <Synth.pb.h>
#include <type_traits>

namespace m2::audio::synthesizer {
	template <typename ForwardIterator, unsigned SampleRate = 48000>
	void MixTrack(ForwardIterator first, ForwardIterator last, const pb::SynthTrack& track, const BeatsPerMinute bpm) {
		if (bpm == 0 || 10000 < bpm) {
			throw M2_ERROR("BPM out-of-bounds");
		}

		for (const auto& note : track.notes()) {
			if (to_float(note.start_beat()) < 0.0f) {
				throw M2_ERROR("Start out-of-bounds");
			}
			if (to_float(note.duration()) < 0.0f) {
				throw M2_ERROR("Duration out-of-bounds");
			}

			const auto noteFirst = first + NoteSampleCount(note.start_beat(), bpm, SampleRate);
			const auto noteLength = NoteSampleCount(note.duration(), bpm, SampleRate); // in samples
			const auto bufferLength = static_cast<size_t>(last - noteFirst); // samples left in buffer
			const auto noteLast = noteFirst + std::min(noteLength, bufferLength);

			MixNote<ForwardIterator,SampleRate>(noteFirst, noteLast, track.shape(), note.frequency(), note.volume(), track.volume(), track.balance());
			if (noteLast == last) {
				break;
			}
		}
	}
}
