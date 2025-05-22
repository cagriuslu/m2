#include <m2/audio/Synth.h>
#include <m2/math/Rational.h>
#include <m2/Error.h>

size_t m2::synth::note_sample_count(const pb::Rational& beats, SynthBpm bpm, unsigned sample_rate) {
	// SampleRate * beats * SecondsPerBeat
	return sample_rate * beats.n() * 60 / beats.d() / bpm;
}

m2::Rational m2::synth::track_beat_count(const pb::SynthTrack& track) {
	Rational total_duration;
	for (const auto& note : track.notes()) {
		auto sum = Rational{note.start_beat()} + Rational{note.duration()};
		total_duration = total_duration < sum ? sum : total_duration;
	}
	return total_duration;
}
size_t m2::synth::track_sample_count(SynthBpm bpm, const pb::SynthTrack& track, unsigned sample_rate) {
	return note_sample_count(track_beat_count(track).to_pb(), bpm, sample_rate);
}

size_t m2::synth::song_sample_count(const pb::SynthSong& song, unsigned sample_rate) {
	size_t max_sample_count = 0;
	for (const auto& track : song.tracks()) {
		max_sample_count = std::max(max_sample_count, track_sample_count(song.bpm(), track, sample_rate));
	}
	return max_sample_count;
}
