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
	auto songs = protobuf::json_file_to_message<pb::Songs>(path);
	if (!songs) {
		throw M2ERROR(songs.error());
	}

	std::vector<Song> songs_vector(protobuf::enum_value_count<m2g::pb::SongType>());
	std::vector<bool> is_loaded(protobuf::enum_value_count<m2g::pb::SongType>());

	for (const auto& song : songs->songs()) {
		LOGF_DEBUG("Loading song %s...", protobuf::enum_name(song.type()).c_str());
		auto index = protobuf::enum_index(song.type());
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
	for (int e = 0; e < protobuf::enum_value_count<m2g::pb::SongType>(); ++e) {
		if (!is_loaded[e]) {
			throw M2ERROR("Song is not defined: " + protobuf::enum_name<m2g::pb::SongType>(e));
		}
	}

	return songs_vector;
}
