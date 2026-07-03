#include <gtest/gtest.h>
#include <m2/audio/synthesizer/MixNote.h>
#include <m2/audio/synthesizer/MixTrack.h>
#include <m2/audio/synthesizer/MixSong.h>
#include <m2/common/Error.h>
#include <optional>
#include <vector>

using namespace m2::audio::synthesizer;

namespace {
	void SetRational(m2::pb::Rational* rational, int64_t nominator, int64_t denominator) {
		rational->set_n(nominator);
		rational->set_d(denominator);
	}

	bool HasNonZeroSample(const std::vector<AudioSample>& buffer) {
		for (const auto& sample : buffer) {
			if (sample.l != 0.0f || sample.r != 0.0f) {
				return true;
			}
		}
		return false;
	}

	// Builds a note whose duration far exceeds any small test buffer, so every
	// written sample falls in the envelope's sustain region (no release div-by-zero).
	m2::pb::SynthNote MakeAudibleNote(float frequency) {
		m2::pb::SynthNote note;
		SetRational(note.mutable_start_beat(), 0, 1);
		SetRational(note.mutable_duration(), 1, 1); // 48000 samples @ 60 BPM
		note.set_frequency(frequency);
		note.set_volume(1.0f);
		return note;
	}
}

TEST(MixNote, WritesNonZeroSamples) {
	std::vector<AudioSample> buffer(100);
	const auto note = MakeAudibleNote(440.0f);
	m2::pb::SoundWaveShape shape = m2::pb::SINE;
	MixNote(buffer.begin(), buffer.end(), 60, shape, 1.0f, 0.0f, std::optional<m2::pb::Envelope>{}, note);
	EXPECT_TRUE(HasNonZeroSample(buffer));
}

TEST(MixNote, BalanceRoutesToRightChannel) {
	std::vector<AudioSample> buffer(50);
	const auto note = MakeAudibleNote(440.0f);
	// SQUARE gives a deterministic +/-1 unit sample; balance +1 silences the left.
	m2::pb::SoundWaveShape shape = m2::pb::SQUARE;
	MixNote(buffer.begin(), buffer.end(), 60, shape, 1.0f, 1.0f, std::optional<m2::pb::Envelope>{}, note);

	bool allLeftZero = true;
	bool anyRightNonZero = false;
	for (const auto& sample : buffer) {
		if (sample.l != 0.0f) {
			allLeftZero = false;
		}
		if (sample.r != 0.0f) {
			anyRightNonZero = true;
		}
	}
	EXPECT_TRUE(allLeftZero);
	EXPECT_TRUE(anyRightNonZero);
}

TEST(MixNote, ThrowsOnFrequencyOutOfBounds) {
	std::vector<AudioSample> buffer(10);
	auto note = MakeAudibleNote(25000.0f); // > 24000 upper bound
	m2::pb::SoundWaveShape shape = m2::pb::SINE;
	EXPECT_THROW(
		MixNote(buffer.begin(), buffer.end(), 60, shape, 1.0f, 0.0f, std::optional<m2::pb::Envelope>{}, note),
		m2::Error);
}

TEST(MixTrack, WritesNonZeroSamples) {
	std::vector<AudioSample> buffer(200);
	m2::pb::SynthTrack track;
	track.set_shape(m2::pb::SINE);
	track.set_volume(1.0f);
	track.set_balance(0.0f);
	*track.add_notes() = MakeAudibleNote(440.0f);
	MixTrack(buffer.begin(), buffer.end(), track, 60);
	EXPECT_TRUE(HasNonZeroSample(buffer));
}

TEST(MixTrack, ThrowsOnBpmOutOfBounds) {
	std::vector<AudioSample> buffer(10);
	m2::pb::SynthTrack track;
	track.set_shape(m2::pb::SINE);
	EXPECT_THROW(MixTrack(buffer.begin(), buffer.end(), track, 0), m2::Error);
}

TEST(MixSong, WritesNonZeroSamples) {
	std::vector<AudioSample> buffer(200);
	m2::pb::SynthSong song;
	song.set_bpm(60);
	auto* track = song.add_tracks();
	track->set_shape(m2::pb::SINE);
	track->set_volume(1.0f);
	track->set_balance(0.0f);
	*track->add_notes() = MakeAudibleNote(440.0f);
	MixSong(buffer.begin(), buffer.end(), song);
	EXPECT_TRUE(HasNonZeroSample(buffer));
}
