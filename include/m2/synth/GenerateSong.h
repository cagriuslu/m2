#ifndef M2_GENERATESONG_H
#define M2_GENERATESONG_H

#include "../Synth.h"
#include "../Rational.h"
#include <Synth.pb.h>
#include <vector>

namespace m2::synth {
	/// Returns iterator to last sample
	template <typename ForwardIterator, unsigned SampleRate = 48000>
	ForwardIterator generate_song(ForwardIterator first, ForwardIterator last, const pb::SynthSong& song, float volume = 1.0f) {
		static_assert(std::is_same<SynthSample, std::remove_cvref_t<decltype(*first)>>(), "ForwardIterator does not point to SynthesizerSample or derivative");
		static_assert(std::is_same<SynthSample, std::remove_cvref_t<decltype(*last)>>(), "ForwardIterator does not point to SynthesizerSample or derivative");
		internal::validate(song);

		auto max_denominator = song_max_denominator(song);
		auto step_sample_count = beat_sample_count(song.bpm(), Rational{1, max_denominator}.to_pb(), SampleRate);
		auto real_last_it = first;

		for (const auto& track : song.tracks()) {
			for (const auto& note : track.notes()) {
				auto first_step_index_of_note = beat_step_count(note.start_beat(), max_denominator);
				auto note_duration_in_steps = beat_step_count(note.duration(), max_denominator);
				auto sample_index_of_note = beat_sample_count(song.bpm(), note.start_beat(), SampleRate);
				auto note_start_it = first + sample_index_of_note;

				ForwardIterator step_it = note_start_it;
				Rational step_fraction_of_cycle;
				for (auto i = first_step_index_of_note; i < first_step_index_of_note + note_duration_in_steps; ++i) {
					auto samples_left_in_buffer = static_cast<size_t>(last - step_it);
					auto step_last_it = step_it + std::min(step_sample_count, samples_left_in_buffer);

					step_fraction_of_cycle = generate_sound(step_it, step_last_it, track.sound(), note.frequency(), track.volume() * note.volume() * volume, step_fraction_of_cycle);
					if (step_last_it == last) {
						break; // Buffer already full
					} else {
						step_it = step_last_it;
					}
				}
				real_last_it = step_it;
			}
		}

		return real_last_it;
	}
}

#endif //M2_GENERATESONG_H
