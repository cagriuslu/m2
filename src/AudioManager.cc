#include <m2/AudioManager.h>
#include <m2/Exception.h>
#include <m2/Game.h>

/// It's not possible to queue audio during audio callback
/// Thus having a high callback frequency allows new audio to be queued faster
constexpr int AUDIO_CALLBACK_FREQUENCY = 30;

m2::AudioManager::Playback::Playback(const AudioSample* _samples, size_t _sample_count, float _volume, PlayPolicy _play_policy, size_t _next_sample) : samples(_samples), sample_count(_sample_count), volume(_volume), play_policy(_play_policy), next_sample(_next_sample) {}

m2::AudioManager::AudioManager() {
	SDL_AudioSpec want{};
	want.freq = 48000;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.samples = want.freq / AUDIO_CALLBACK_FREQUENCY;
	want.callback = audio_callback;
	want.userdata = nullptr; // Passing `this` won't work, object may be moved/copied
	sdl_audio_device_id = SDL_OpenAudioDevice(nullptr, 0, &want, &sdl_audio_spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
	if (!sdl_audio_device_id) {
		throw M2FATAL("SDL_OpenAudioDevice error: " + std::string{SDL_GetError()});
	}
	if (want.format != sdl_audio_spec.format) {
		throw M2FATAL("Undesired audio format");
	}
}

m2::PlaybackId m2::AudioManager::play(const AudioSample* samples, size_t sample_count, PlayPolicy policy, float volume, size_t start_sample) {
	if (sample_count < sdl_audio_spec.samples) {
		throw M2ERROR("Playing short audio is not supported");
	}

	std::unique_lock<std::mutex> lock{playbacks_mutex};
	auto id = playbacks.emplace(samples, sample_count, volume, policy, start_sample);
	SDL_PauseAudioDevice(sdl_audio_device_id, 0);
	return id;
}

m2::AudioManager::Playback* m2::AudioManager::get_playback(PlaybackId id) {
	return playbacks.get(id);
}

void m2::AudioManager::stop(PlaybackId id) {
	std::unique_lock<std::mutex> lock{playbacks_mutex};
	playbacks.free(id);
	if (playbacks.size() == 0) {
		SDL_PauseAudioDevice(sdl_audio_device_id, 1);
	}
}

void m2::AudioManager::audio_callback(MAYBE void* user_data, uint8_t* stream, int length) {
	auto& audio_manager = *GAME.audio_manager;
	auto* out_stream = reinterpret_cast<AudioSample*>(stream);
	auto out_length = (size_t) length / sizeof(AudioSample); // in samples
	memset(stream, 0, length);

	auto copy = [=](Playback* playback, size_t copy_count) {
		auto begin = playback->samples + playback->next_sample;
		auto end = begin + copy_count;
		std::transform(begin, end, out_stream, [=](const AudioSample& sample) -> AudioSample {
			return AudioSample{.l = out_stream->l + sample.l * playback->volume * playback->left_volume(), .r = out_stream->r + sample.r * playback->volume * playback->right_volume()};
		});
		playback->next_sample = (playback->next_sample + copy_count) % playback->sample_count;
	};

	std::unique_lock<std::mutex> lock{audio_manager.playbacks_mutex};
	for (auto [playback, id] : audio_manager.playbacks) {
		// Copy the samples left in the playback buffer
		auto samples_left_playback_buffer = playback->sample_count - playback->next_sample;
		auto min_len = std::min(samples_left_playback_buffer, out_length);
		copy(playback, min_len);

		if (playback->play_policy == PlayPolicy::ONCE && playback->next_sample == 0) {
			// Playback finished
			audio_manager.playbacks.free(id);
		} else {
			// Playback wrapped around and there is space in output buffer
			copy(playback, out_length - min_len);
		}
	}
}
