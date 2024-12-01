#include <m2/audio/AudioManager.h>
#include <m2/Error.h>
#include <m2/Game.h>
#include <m2/Options.h>

/// It's not possible to queue audio during audio callback
/// Thus having a high callback frequency allows new audio to be queued faster
constexpr int AUDIO_CALLBACK_FREQUENCY = 30;

m2::AudioManager::Playback::Playback(const Song* _song, float _volume, PlayPolicy _play_policy) : song(_song), volume(_volume), play_policy(_play_policy) {}

m2::AudioManager::AudioManager() {
	SDL_AudioSpec want{};
	want.freq = DEFAULT_AUDIO_SAMPLE_RATE;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.samples = static_cast<uint16_t>(want.freq / AUDIO_CALLBACK_FREQUENCY);
	want.callback = audio_callback;
	want.userdata = nullptr; // Passing `this` won't work, object may be moved/copied
	sdl_audio_device_id = SDL_OpenAudioDevice(nullptr, 0, &want, &sdl_audio_spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
	if (!sdl_audio_device_id) {
		throw M2_ERROR("SDL_OpenAudioDevice error: " + std::string{SDL_GetError()});
	}
	if (want.format != sdl_audio_spec.format) {
		throw M2_ERROR("Undesired audio format");
	}
}
m2::AudioManager::~AudioManager() {
	SDL_PauseAudioDevice(sdl_audio_device_id, 1);
}

m2::PlaybackId m2::AudioManager::play(const Song* song, PlayPolicy policy, float volume) {
	if (song->sample_count() < sdl_audio_spec.samples) {
		throw M2_ERROR("Playing short audio is not supported");
	}

	std::unique_lock<std::mutex> lock{playbacks_mutex};
	auto it = playbacks.emplace(song, volume, policy);
	SDL_PauseAudioDevice(sdl_audio_device_id, 0);
	return it.id();
}

void m2::AudioManager::stop(PlaybackId id) {
	std::unique_lock<std::mutex> lock{playbacks_mutex};
	playbacks.free(id);
	if (playbacks.size() == 0) {
		SDL_PauseAudioDevice(sdl_audio_device_id, 1);
	}
}

bool m2::AudioManager::has_playback(PlaybackId id) {
	std::unique_lock<std::mutex> lock{playbacks_mutex};
	return playbacks.get(id) != nullptr;
}
void m2::AudioManager::set_playback_volume(PlaybackId id, float volume) {
	std::unique_lock<std::mutex> lock{playbacks_mutex};
	if (auto* playback = playbacks.get(id); playback) {
		playback->volume = volume;
	}
}
void m2::AudioManager::set_playback_left_volume(PlaybackId id, float volume) {
	std::unique_lock<std::mutex> lock{playbacks_mutex};
	if (auto* playback = playbacks.get(id); playback) {
		playback->left_volume = volume;
	}
}
void m2::AudioManager::set_playback_right_volume(PlaybackId id, float volume) {
	std::unique_lock<std::mutex> lock{playbacks_mutex};
	if (auto* playback = playbacks.get(id); playback) {
		playback->right_volume = volume;
	}
}

void m2::AudioManager::audio_callback(MAYBE void* user_data, uint8_t* stream, int length) {
	auto& audio_manager = *M2_GAME.audio_manager;
	auto* out_stream = reinterpret_cast<AudioSample*>(stream);
	auto out_length = (size_t) length / sizeof(AudioSample); // in samples

	// Clear buffer
	std::fill(out_stream, out_stream + out_length, AudioSample{});

	auto copy = [=](Playback* playback, size_t copy_count) {
		const auto* begin = playback->song->data() + playback->next_sample;
		const auto* end = begin + copy_count;

		for (auto it = begin; it != end; ++it) {
			if (silent) {
				out_stream[it - begin] = {};
			} else {
				const auto& playback_sample = *it;
				auto l_playback_sample = playback_sample.l * playback->volume * playback->left_volume;
				auto r_playback_sample = playback_sample.r * playback->volume * playback->right_volume;
				out_stream[it - begin].mutable_mix(l_playback_sample, r_playback_sample);
			}
		}

		playback->next_sample = (playback->next_sample + copy_count) % playback->song->sample_count();
	};

	std::unique_lock<std::mutex> lock{audio_manager.playbacks_mutex};
	for (auto it = audio_manager.playbacks.begin(); it != audio_manager.playbacks.end(); ++it) {
		// Copy the samples left in the playback buffer
		auto samples_left_playback_buffer = it->song->sample_count() - it->next_sample;
		auto min_len = std::min(samples_left_playback_buffer, out_length);
		copy(&*it, min_len);

		if (it->play_policy == PlayPolicy::ONCE && it->next_sample == 0) {
			// Playback finished
			audio_manager.playbacks.free(it.id());
		} else {
			// Playback wrapped around and there is space in output buffer
			copy(&*it, out_length - min_len);
		}
	}
}
