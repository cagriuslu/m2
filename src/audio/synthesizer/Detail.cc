#include <m2/audio/synthesizer/Detail.h>

using namespace m2::audio;

synthesizer::AudioSample synthesizer::AudioSample::operator+(const AudioSample& rhs) const {
	return AudioSample{
		.l = this->l + rhs.l,
		.r = this->r + rhs.r
	};
}
synthesizer::AudioSample& synthesizer::AudioSample::operator+=(const AudioSample& rhs) {
	*this = *this + rhs;
	return *this;
}

size_t m2::audio::synthesizer::NoteSampleCount(const pb::Rational& beats, const BeatsPerMinute bpm, const unsigned sample_rate) {
	return NoteSampleCount(Rational{beats}, bpm, sample_rate);
}
size_t m2::audio::synthesizer::NoteSampleCount(const Rational& beats, BeatsPerMinute bpm, unsigned sample_rate) {
	// SampleRate * beats * SecondsPerBeat
	return sample_rate * beats.n() * 60 / beats.d() / bpm;
}

m2::Rational m2::audio::synthesizer::TrackBeatCount(const pb::SynthTrack& track) {
	Rational total_duration;
	for (const auto& note : track.notes()) {
		auto sum = Rational{note.start_beat()} + Rational{note.duration()} + Rational{track.amplitude_envelope().release_duration()};
		total_duration = total_duration < sum ? sum : total_duration;
	}
	return total_duration;
}
size_t m2::audio::synthesizer::TrackSampleCount(const BeatsPerMinute bpm, const pb::SynthTrack& track, const unsigned sample_rate) {
	return NoteSampleCount(TrackBeatCount(track).to_pb(), bpm, sample_rate);
}

size_t m2::audio::synthesizer::SongSampleCount(const pb::SynthSong& song, const unsigned sample_rate) {
	size_t max_sample_count = 0;
	for (const auto& track : song.tracks()) {
		max_sample_count = std::max(max_sample_count, TrackSampleCount(song.bpm(), track, sample_rate));
	}
	return max_sample_count;
}
