#ifndef M2_AUDIOMANAGER_H
#define M2_AUDIOMANAGER_H

#include "Audio.h"
#include <SDL.h>
#include <vector>
#include <memory>

namespace m2 {
	using PlaybackId = size_t;

	class AudioManager {
		enum PlayPolicy {
			ONCE = 0,
			LOOP
		};
		struct Playback {
			PlaybackId playback_id{};
			const AudioSample* samples{};
			size_t sample_count{};
			PlayPolicy play_policy{};
			size_t next_sample{};
		};

		SDL_AudioDeviceID sdl_audio_device_id{};
		SDL_AudioSpec sdl_audio_spec{};
		PlaybackId _next_playback_id{};
		std::vector<Playback> _playbacks;

	public:
		AudioManager();

		PlaybackId loop(const AudioSample* samples, size_t sample_count, size_t start_sample);
		void stop(PlaybackId id);

	private:
		static void audio_callback(void* user_data, uint8_t* stream, int length);
	};
}

#endif //M2_AUDIOMANAGER_H
