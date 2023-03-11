#include <m2/Song.h>
#include <m2/protobuf/Utils.h>
#include <m2/Log.h>

m2::Song::Song(const pb::Song &song) {
	auto song_size = synth::song_sample_count(song.synth_song(), DEFAULT_AUDIO_SAMPLE_RATE);
	LOG_DEBUG("Song size in samples", song_size);
	_samples.resize(song_size);
	synth::mix_song(_samples.begin(), _samples.end(), song.synth_song());
}

std::vector<m2::Song> m2::load_songs(const std::string& path) {
	auto songs = proto::json_file_to_message<pb::Songs>(path);
	if (!songs) {
		throw M2ERROR(songs.error());
	}

	const auto* song_type_desc = m2g::pb::SongType_descriptor();
	std::vector<Song> songs_vector(song_type_desc->value_count());
	std::vector<bool> is_loaded(song_type_desc->value_count());

	for (const auto& song : songs->songs()) {
		LOGF_DEBUG("Loading song %s...", song_type_desc->FindValueByNumber(song.type())->name().c_str());
		auto index = song_type_desc->FindValueByNumber(song.type())->index();
		// Check if the song is already loaded
		if (is_loaded[index]) {
			throw M2ERROR("Song has duplicate definition: " + std::to_string(song.type()));
		}
		// Load song
		songs_vector[index] = Song{song};
		is_loaded[index] = true;
		LOG_DEBUG("Loaded song");
	}

	// Check if every item is loaded
	for (int e = 0; e < song_type_desc->value_count(); ++e) {
		if (!is_loaded[e]) {
			throw M2ERROR("Song is not defined: " + std::to_string(song_type_desc->value(e)->number()));
		}
	}

	return songs_vector;
}
