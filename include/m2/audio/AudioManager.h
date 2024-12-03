#pragma once
#include "../Song.h"
#include "../Pool.h"
#include <SDL.h>
#include <mutex>

namespace m2 {
	using PlaybackId = Id;

	struct AudioManager {
		enum PlayPolicy {
			ONCE = 0,
			LOOP
		};

	private:
		struct Playback {
			const Song* song{};
			float volume{1.0f};
			PlayPolicy play_policy{};
			size_t next_sample{};
			float left_volume{1.0f}, right_volume{1.0f};

			Playback() = default;
			Playback(const Song* _song, float _volume, PlayPolicy _play_policy);
		};

	public:
		SDL_AudioDeviceID sdl_audio_device_id{};
		SDL_AudioSpec sdl_audio_spec{};
		Pool<Playback, 32> playbacks;
		std::mutex playbacks_mutex;

		AudioManager();
		~AudioManager();

		PlaybackId play(const Song* song, PlayPolicy policy, float volume = 1.0f);
		void stop(PlaybackId id);

		bool has_playback(PlaybackId id);
		void set_playback_volume(PlaybackId id, float volume);
		void set_playback_left_volume(PlaybackId id, float volume);
		void set_playback_right_volume(PlaybackId id, float volume);

	private:
		static void audio_callback(void* count, uint8_t* stream, int length);
	};
}
