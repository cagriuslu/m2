#ifndef M2_AUDIOMANAGER_H
#define M2_AUDIOMANAGER_H

#include "Value.h"
#include "Song.h"
#include "Synth.h"
#include "Pool.hh"
#include <SDL.h>
#include <list>
#include <memory>
#include <mutex>
#include <array>

namespace m2 {
	using PlaybackId = Id;

	struct AudioManager {
		enum PlayPolicy {
			ONCE = 0,
			LOOP
		};

		struct Playback {
			const Song* song{};
			float volume{1.0f};
			PlayPolicy play_policy{};
			size_t next_sample{};

			Playback() = default;
			Playback(const Song* _song, float _volume, PlayPolicy _play_policy);

			inline void set_left_volume(float v) { _left_volume = v; }
			[[nodiscard]] inline float left_volume() const { return _left_volume; }
			inline void set_right_volume(float v) { _right_volume = v; }
			[[nodiscard]] inline float right_volume() const { return _right_volume; }
		private:
			float _left_volume{1.0f}, _right_volume{1.0f};
		};

		SDL_AudioDeviceID sdl_audio_device_id{};
		SDL_AudioSpec sdl_audio_spec{};
		Pool<Playback, 32> playbacks;
		std::mutex playbacks_mutex;

		AudioManager();

		PlaybackId play(const Song* song, PlayPolicy policy, float volume = 1.0f);
		Playback* get_playback(PlaybackId id);
		void stop(PlaybackId id);

	private:
		static void audio_callback(void* count, uint8_t* stream, int length);
	};
}

#endif //M2_AUDIOMANAGER_H
