#include <m2/audio/Song.h>
#include <m2/protobuf/Detail.h>
#include <m2/Log.h>

m2::Song::Song(const pb::Song &song) {
	auto song_size = synth::song_sample_count(song.synth_song(), DEFAULT_AUDIO_SAMPLE_RATE);
	LOG_DEBUG("Song size in samples", song_size);
	_samples.resize(song_size);
	synth::mix_song(_samples.begin(), _samples.end(), song.synth_song());
}
