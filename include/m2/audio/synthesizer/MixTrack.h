#pragma once
#include "Detail.h"
#include "MixNote.h"
#include <m2/math/Rational.h>
#include <m2/Error.h>
#include <Synth.pb.h>
#include <type_traits>

namespace m2::audio::synthesizer {
	template <typename SampleIterator, unsigned SampleRate = 48000>
	void MixTrack(const SampleIterator first, const SampleIterator last, const pb::SynthTrack& track, const BeatsPerMinute bpm) {
		if (bpm == 0 || 10000 < bpm) {
			throw M2_ERROR("BPM out-of-bounds");
		}

		std::optional<pb::Envelope> amplitudeEnv = track.has_amplitude_envelope() ? track.amplitude_envelope() : std::optional<pb::Envelope>{};

		for (const auto& note : track.notes()) {
			if (ToFloat(note.start_beat()) < 0.0f) {
				throw M2_ERROR("Start out-of-bounds");
			}
			if (ToFloat(note.duration()) < 0.0f) {
				throw M2_ERROR("Duration out-of-bounds");
			}

			// Calculate how much space is left in the buffer
			const auto noteFirstIt = first + NoteSampleCount(note.start_beat(), bpm, SampleRate);
			const auto noteLength = NoteSampleCount(note.duration(), bpm, SampleRate)
				+ NoteSampleCount(track.amplitude_envelope().release_duration(), bpm, SampleRate); // in samples
			const auto bufferLength = static_cast<size_t>(last - noteFirstIt); // samples left in buffer
			const auto noteLastIt = noteFirstIt + std::min(noteLength, bufferLength);

			MixNote<SampleIterator,SampleRate>(noteFirstIt, noteLastIt, bpm, track.shape(), track.volume(), track.balance(), amplitudeEnv, note);
			if (noteLastIt == last) {
				break;
			}
		}
	}
}
