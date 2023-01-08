#include <m2/Synth.h>
#include <m2/Rational.h>
#include <m2/Exception.h>

size_t m2::synth::beat_sample_count(SynthBpm bpm, const pb::Rational& beats, unsigned sample_rate) {
	// SampleRate * beats * SecondsPerBeat
	return sample_rate * beats.n() * 60 / beats.d() / bpm;
}
size_t m2::synth::beat_step_count(const pb::Rational& beats, int32_t max_denominator) {
	auto step_count = (Rational{beats} * max_denominator).simplify();
	if (step_count.d() != 1) {
		throw M2FATAL("Implementation error");
	}
	return step_count.n();
}

m2::Rational m2::synth::track_beat_count(const pb::SynthTrack& track) {
	Rational total_duration;
	for (const auto& note : track.notes()) {
		auto sum = Rational{note.start_beat()} + Rational{note.duration()};
		total_duration = total_duration < sum ? sum : total_duration;
	}
	return total_duration;
}
int32_t m2::synth::track_max_denominator(const pb::SynthTrack& track) {
	int32_t max_denominator = 1;
	for (const auto& note : track.notes()) {
		auto start_denominator = Rational{note.start_beat()}.simplify().d();
		auto duration_denominator = Rational{note.duration()}.simplify().d();
		max_denominator = std::max(max_denominator, std::max(start_denominator, duration_denominator));
	}
	return max_denominator;
}
size_t m2::synth::track_step_count(const pb::SynthTrack& track) {
	return beat_step_count(track_beat_count(track).to_pb(), track_max_denominator(track));
}
size_t m2::synth::track_sample_count(SynthBpm bpm, const pb::SynthTrack& track, unsigned sample_rate) {
	return beat_sample_count(bpm, track_beat_count(track).to_pb(), sample_rate);
}

m2::Rational m2::synth::song_beat_count(const pb::SynthSong& song) {
	Rational max_beat_count;
	for (const auto& track : song.tracks()) {
		auto beat_count_of_track = track_beat_count(track);
		max_beat_count = max_beat_count < beat_count_of_track ? beat_count_of_track : max_beat_count;
	}
	return max_beat_count;
}
int32_t m2::synth::song_max_denominator(const pb::SynthSong& song) {
	int32_t max_max_denominator = 1;
	for (const auto& track : song.tracks()) {
		max_max_denominator = std::max(max_max_denominator, track_max_denominator(track));
	}
	return max_max_denominator;
}
size_t m2::synth::song_step_count(const pb::SynthSong& song) {
	size_t max_step_count = 0;
	for (const auto& track : song.tracks()) {
		max_step_count = std::max(max_step_count, track_step_count(track));
	}
	return max_step_count;
}
size_t m2::synth::song_sample_count(const pb::SynthSong& song, unsigned sample_rate) {
	size_t max_sample_count = 0;
	for (const auto& track : song.tracks()) {
		max_sample_count = std::max(max_sample_count, track_sample_count(song.bpm(), track, sample_rate));
	}
	return max_sample_count;
}

void m2::synth::internal::validate(MAYBE const pb::SynthSound& sound, float frequency) {
	if (frequency == 0.0f && sound.shape() != pb::NOISE) {
		throw M2ERROR("Frequency out-of-bounds");
	}
	if (frequency < 0.0f || 24000.0f < frequency) {
		throw M2ERROR("Frequency out-of-bounds");
	}
}

void m2::synth::internal::validate(const pb::SynthNote& note) {
	if (to_float(note.start_beat()) < 0.0f) {
		throw M2ERROR("Start out-of-bounds");
	}
	if (to_float(note.duration()) < 0.0f) {
		throw M2ERROR("Duration out-of-bounds");
	}
	if (note.frequency() < 0.0f || 24000.0f < note.frequency()) {
		throw M2ERROR("Frequency out-of-bounds");
	}
	if (note.volume() < 0.0f || 1.0f < note.volume()) {
		throw M2ERROR("Volume out-of-bounds");
	}
}

void m2::synth::internal::validate(const pb::SynthTrack& track, SynthBpm bpm) {
	validate(track.sound(), 1.0f);

	Rational time;
	for (const auto& note : track.notes()) {
		validate(note);

		Rational start{note.start_beat()};
		if (start < time) {
			throw M2ERROR("Track notes are not monotonically increasing");
		}
		time = start;
	}

	if (bpm == 0 || 1000 < bpm) {
		throw M2ERROR("BPM out-of-bounds");
	}
}

void m2::synth::internal::validate(const pb::SynthSong& song) {
	for (const auto& track : song.tracks()) {
		validate(track, song.bpm());
	}
}
