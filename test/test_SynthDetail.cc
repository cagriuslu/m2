#include <gtest/gtest.h>
#include <m2/audio/synthesizer/Detail.h>

using namespace m2::audio::synthesizer;

namespace {
	// Sets the nominator/denominator of a pb::Rational field.
	void SetRational(m2::pb::Rational* rational, int64_t nominator, int64_t denominator) {
		rational->set_n(nominator);
		rational->set_d(denominator);
	}
}

TEST(SynthDetail, AudioSampleAddsChannelsIndependently) {
	const AudioSample lhs{1.0f, 2.0f};
	const AudioSample rhs{10.0f, 20.0f};
	const AudioSample sum = lhs + rhs;
	EXPECT_FLOAT_EQ(sum.l, 11.0f);
	EXPECT_FLOAT_EQ(sum.r, 22.0f);
}

TEST(SynthDetail, AudioSampleCompoundAdd) {
	AudioSample accumulator{1.0f, -2.0f};
	const AudioSample& returned = (accumulator += AudioSample{0.5f, -1.0f});
	EXPECT_FLOAT_EQ(accumulator.l, 1.5f);
	EXPECT_FLOAT_EQ(accumulator.r, -3.0f);
	// operator+= returns a reference to the modified sample.
	EXPECT_EQ(&returned, &accumulator);
}

TEST(SynthDetail, NoteSampleCountFromPbRational) {
	m2::pb::Rational beats;
	// 1 beat @ 60 BPM @ 48000 Hz => 1 second => 48000 samples.
	SetRational(&beats, 1, 1);
	EXPECT_EQ(NoteSampleCount(beats, 60, 48000), 48000u);
	// 1 beat @ 120 BPM => half a second => 24000 samples.
	EXPECT_EQ(NoteSampleCount(beats, 120, 48000), 24000u);
}

TEST(SynthDetail, NoteSampleCountFromRational) {
	// Half a beat @ 60 BPM @ 48000 Hz => 24000 samples.
	EXPECT_EQ(NoteSampleCount(m2::Rational{1, 2}, 60, 48000), 24000u);
	EXPECT_EQ(NoteSampleCount(m2::Rational{1, 1}, 60, 48000), 48000u);
}

TEST(SynthDetail, NoteSampleCountTruncatesDown) {
	m2::pb::Rational beats;
	SetRational(&beats, 1, 7);
	// 1000 * 1 * 60 / 7 / 60 = 60000 / 7 (=8571) / 60 = 142 (truncated twice).
	EXPECT_EQ(NoteSampleCount(beats, 60, 1000), 142u);
}

TEST(SynthDetail, TrackBeatCountIsLatestNoteEnd) {
	m2::pb::SynthTrack track;
	// Note 1 ends at start(0) + duration(2) = 2 beats.
	auto* firstNote = track.add_notes();
	SetRational(firstNote->mutable_start_beat(), 0, 1);
	SetRational(firstNote->mutable_duration(), 2, 1);
	// Note 2 ends at start(1) + duration(3) = 4 beats (the later end).
	auto* secondNote = track.add_notes();
	SetRational(secondNote->mutable_start_beat(), 1, 1);
	SetRational(secondNote->mutable_duration(), 3, 1);

	const auto beatCount = TrackBeatCount(track);
	EXPECT_EQ(beatCount.GetN(), 4);
	EXPECT_EQ(beatCount.GetD(), 1);
}

TEST(SynthDetail, TrackSampleCount) {
	m2::pb::SynthTrack track;
	auto* note = track.add_notes();
	SetRational(note->mutable_start_beat(), 0, 1);
	SetRational(note->mutable_duration(), 1, 1);
	// 1 beat @ 60 BPM @ 48000 Hz => 48000 samples.
	EXPECT_EQ(TrackSampleCount(60, track, 48000), 48000u);
}

TEST(SynthDetail, SongSampleCountIsLongestTrack) {
	m2::pb::SynthSong song;
	song.set_bpm(60);
	// Track A: 1 beat => 48000 samples.
	auto* shortTrack = song.add_tracks();
	auto* shortNote = shortTrack->add_notes();
	SetRational(shortNote->mutable_start_beat(), 0, 1);
	SetRational(shortNote->mutable_duration(), 1, 1);
	// Track B: 2 beats => 96000 samples.
	auto* longTrack = song.add_tracks();
	auto* longNote = longTrack->add_notes();
	SetRational(longNote->mutable_start_beat(), 0, 1);
	SetRational(longNote->mutable_duration(), 2, 1);

	EXPECT_EQ(SongSampleCount(song, 48000), 96000u);
}
