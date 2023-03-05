#ifndef M2_AUDIOMANAGER_H
#define M2_AUDIOMANAGER_H

#include "Value.h"
#include <SDL.h>
#include <list>
#include <memory>

namespace m2 {
	using PlaybackId = size_t;

	struct AudioSample {
		float l{}, r{};
	};

	class AudioManager {
		enum PlayPolicy {
			ONCE = 0,
			LOOP
		};

	public:
		struct Playback {
			const AudioSample* samples{};
			size_t sample_count{};
			float volume{1.0f};
			PlayPolicy play_policy{};
			size_t next_sample{};

			Playback() = default;
			Playback(const AudioSample* _samples, size_t _sample_count, float _volume, PlayPolicy _play_policy, size_t _next_sample);

			inline void set_left_volume(float v) { _left_volume = v; }
			[[nodiscard]] inline float left_volume() const { return _left_volume; }
			inline void set_right_volume(float v) { _right_volume = v; }
			[[nodiscard]] inline float right_volume() const { return _right_volume; }
		private:
			float _left_volume{1.0f}, _right_volume{1.0f};
		};

	private:
		SDL_AudioDeviceID sdl_audio_device_id{};
		SDL_AudioSpec sdl_audio_spec{};
		PlaybackId _next_playback_id{};
		std::unordered_map<PlaybackId, Playback> _playbacks;

	public:
		AudioManager();

		PlaybackId loop(const AudioSample* samples, size_t sample_count, float volume = 1.0f, size_t start_sample = 0);
		Playback* get_playback(PlaybackId id);
		void stop(PlaybackId id);

	private:
		static void audio_callback(void* count, uint8_t* stream, int length);
	};
}

#endif //M2_AUDIOMANAGER_H
