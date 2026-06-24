#include <m2/audio/Song.h>
#include <m2/protobuf/Detail.h>
#include <m2/audio/synthesizer/MixSong.h>
#include <m2/common/Constants.h>
#include <m2/Log.h>

m2::Song::Song(const pb::Song &song) {
	auto song_size = audio::synthesizer::SongSampleCount(song.synth_song(), DEFAULT_AUDIO_SAMPLE_RATE);
	LOG_DEBUG("Song size in samples", song_size);
	_samples.resize(song_size);
	MixSong(_samples.begin(), _samples.end(), song.synth_song());
}
