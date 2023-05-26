#pragma once
#include "Synth.h"
#include <Song.pb.h>
#include <vector>
#include <filesystem>

namespace m2 {
	class Song {
		std::vector<AudioSample> _samples;

	public:
		Song() = default;
		explicit Song(const pb::Song& song);

		[[nodiscard]] inline size_t sample_count() const { return _samples.size(); }
		[[nodiscard]] inline const AudioSample* data() const { return _samples.data(); }
	};

	std::vector<Song> load_songs(const std::filesystem::path& path);
}
