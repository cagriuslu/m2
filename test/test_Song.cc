#include <gtest/gtest.h>
#include <m2/audio/Song.h>
#include <m2/audio/synthesizer/Detail.h>

TEST(Song, DefaultIsEmpty) {
	m2::Song song;
	EXPECT_EQ(song.SampleCount(), 0u);
}

TEST(Song, SampleBufferHasExpectedLength) {
	m2::pb::Song pbSong;
	auto* synthSong = pbSong.mutable_synth_song();
	synthSong->set_bpm(60);
	auto* track = synthSong->add_tracks();
	track->set_shape(m2::pb::SINE);
	track->set_volume(1.0f);
	track->set_balance(0.0f);
	auto* note = track->add_notes();
	note->mutable_start_beat()->set_n(0);
	note->mutable_start_beat()->set_d(1);
	// A tiny 1/1000-beat note keeps the resulting buffer small and predictable:
	// 48000 * 1 * 60 / 1000 / 60 = 48 samples.
	note->mutable_duration()->set_n(1);
	note->mutable_duration()->set_d(1000);
	note->set_frequency(440.0f);
	note->set_volume(1.0f);

	const m2::Song song{pbSong};
	EXPECT_EQ(song.SampleCount(), 48u);
	// The buffer length must match SongSampleCount at the engine's default sample rate.
	EXPECT_EQ(song.SampleCount(), m2::audio::synthesizer::SongSampleCount(pbSong.synth_song(), 48000));
	EXPECT_NE(song.Data(), nullptr);
}
