#pragma once
#include "Song.h"
#include <m2/containers/Pool.h>
#include <m2/thirdparty/audio/Device.h>
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
			PlayPolicy playPolicy{};
			size_t nextSample{};
			float leftVolume{1.0f}, rightVolume{1.0f};

			Playback() = default;
			Playback(const Song* _song, float _volume, PlayPolicy _play_policy);
		};

	public:
		thirdparty::audio::Device device;
		Pool<Playback, 32> playbacks;
		std::mutex playbacksMutex;

		AudioManager();

		PlaybackId Play(const Song* song, PlayPolicy policy, float volume = 1.0f);
		void Stop(PlaybackId id);

		bool HasPlayback(PlaybackId id);
		void SetPlaybackVolume(PlaybackId id, float volume);
		void SetPlaybackLeftVolume(PlaybackId id, float volume);
		void SetPlaybackRightVolume(PlaybackId id, float volume);

	private:
		static void AudioCallback(void* count, uint8_t* stream, int length);
	};
}
