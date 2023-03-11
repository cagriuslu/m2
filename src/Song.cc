#include <m2/Song.h>
#include <m2/protobuf/Utils.h>

m2::Song::Song(const pb::Song &song) {
	_samples.resize(synth::song_sample_count(song.synth_song(), DEFAULT_AUDIO_SAMPLE_RATE));
	synth::mix_song(_samples.begin(), _samples.end(), song.synth_song());
}

std::vector<m2::Song> m2::load_songs(const std::string& path) {
	auto songs = proto::json_file_to_message<pb::Songs>(path);
	if (!songs) {
		throw M2ERROR(songs.error());
	}

	std::vector<Song> songs_vector(m2g::pb::SongType_ARRAYSIZE);
	std::vector<bool> is_loaded(m2g::pb::SongType_ARRAYSIZE);

	const auto* song_type_desc = m2g::pb::SongType_descriptor();
	for (const auto& song : songs->songs()) {
		auto index = song_type_desc->FindValueByNumber(song.type())->index();
		// Check if the song is already loaded
		if (is_loaded[index]) {
			throw M2ERROR("Song has duplicate definition: " + std::to_string(song.type()));
		}
		// Load song
		songs_vector[index] = Song{song};
		is_loaded[index] = true;
	}

	// Check if every item is loaded
	for (int e = 0; e < song_type_desc->value_count(); ++e) {
		if (!is_loaded[e]) {
			throw M2ERROR("Song is not defined: " + std::to_string(song_type_desc->value(e)->number()));
		}
	}

	return songs_vector;
}
