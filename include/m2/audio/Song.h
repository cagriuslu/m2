#pragma once
#include "synthesizer/Detail.h"
#include <Song.pb.h>
#include <vector>
#include <filesystem>

namespace m2 {
	class Song {
		std::vector<audio::synthesizer::AudioSample> _samples;

	public:
		Song() = default;
		explicit Song(const pb::Song& song);

		[[nodiscard]] size_t SampleCount() const { return _samples.size(); }
		[[nodiscard]] const audio::synthesizer::AudioSample* Data() const { return _samples.data(); }
	};
}
