#include <m2/audio/AudioManager.h>
#include <m2/Error.h>
#include <m2/Game.h>
#include <m2/Options.h>

/// It's not possible to queue audio during audio callback
/// Thus having a high callback frequency allows new audio to be queued faster
constexpr int AUDIO_CALLBACK_FREQUENCY = 90;

m2::AudioManager::Playback::Playback(const Song* _song, float _volume, PlayPolicy _play_policy) : song(_song), volume(_volume), playPolicy(_play_policy) {}

m2::AudioManager::AudioManager() {
	SDL_AudioSpec want{};
	want.freq = audio::synthesizer::gDefaultAudioSampleRate;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.samples = static_cast<uint16_t>(want.freq / AUDIO_CALLBACK_FREQUENCY);
	want.callback = AudioCallback;
	want.userdata = nullptr; // Passing `this` won't work, object may be moved/copied
	sdlAudioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &want, &sdlAudioSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
	if (!sdlAudioDeviceId) {
		throw M2_ERROR("SDL_OpenAudioDevice error: " + std::string{SDL_GetError()});
	}
	if (want.format != sdlAudioSpec.format) {
		throw M2_ERROR("Undesired audio format");
	}
}
m2::AudioManager::~AudioManager() {
	SDL_PauseAudioDevice(sdlAudioDeviceId, 1);
}

m2::PlaybackId m2::AudioManager::Play(const Song* song, PlayPolicy policy, float volume) {
	if (song->SampleCount() < sdlAudioSpec.samples) {
		throw M2_ERROR("Playing short audio is not supported");
	}

	std::unique_lock<std::mutex> lock{playbacksMutex};
	auto it = playbacks.Emplace(song, volume, policy);
	SDL_PauseAudioDevice(sdlAudioDeviceId, 0);
	return it.GetId();
}

void m2::AudioManager::Stop(PlaybackId id) {
	std::unique_lock<std::mutex> lock{playbacksMutex};
	playbacks.Free(id);
	if (playbacks.Size() == 0) {
		SDL_PauseAudioDevice(sdlAudioDeviceId, 1);
	}
}

bool m2::AudioManager::HasPlayback(PlaybackId id) {
	std::unique_lock<std::mutex> lock{playbacksMutex};
	return playbacks.Get(id) != nullptr;
}
void m2::AudioManager::SetPlaybackVolume(PlaybackId id, float volume) {
	std::unique_lock<std::mutex> lock{playbacksMutex};
	if (auto* playback = playbacks.Get(id); playback) {
		playback->volume = volume;
	}
}
void m2::AudioManager::SetPlaybackLeftVolume(PlaybackId id, float volume) {
	std::unique_lock<std::mutex> lock{playbacksMutex};
	if (auto* playback = playbacks.Get(id); playback) {
		playback->leftVolume = volume;
	}
}
void m2::AudioManager::SetPlaybackRightVolume(PlaybackId id, float volume) {
	std::unique_lock<std::mutex> lock{playbacksMutex};
	if (auto* playback = playbacks.Get(id); playback) {
		playback->rightVolume = volume;
	}
}

void m2::AudioManager::AudioCallback(MAYBE void* user_data, uint8_t* stream, int length) {
	auto& audio_manager = *M2_GAME.audio_manager;
	auto* out_stream = reinterpret_cast<audio::synthesizer::AudioSample*>(stream);
	auto out_length = (size_t) length / sizeof(audio::synthesizer::AudioSample); // in samples

	// Clear buffer
	std::fill(out_stream, out_stream + out_length, audio::synthesizer::AudioSample{});

	auto copy = [=](Playback* playback, size_t copy_count) {
		const auto* begin = playback->song->Data() + playback->nextSample;
		const auto* end = begin + copy_count;

		for (auto it = begin; it != end; ++it) {
			if (silent) {
				out_stream[it - begin] = {};
			} else {
				const auto& playback_sample = *it;
				auto l_playback_sample = playback_sample.l * playback->volume * playback->leftVolume;
				auto r_playback_sample = playback_sample.r * playback->volume * playback->rightVolume;
				out_stream[it - begin].MutableMix(l_playback_sample, r_playback_sample);
			}
		}

		playback->nextSample = (playback->nextSample + copy_count) % playback->song->SampleCount();
	};

	std::unique_lock<std::mutex> lock{audio_manager.playbacksMutex};
	for (auto it = audio_manager.playbacks.begin(); it != audio_manager.playbacks.end(); ++it) {
		// Copy the samples left in the playback buffer
		auto samples_left_playback_buffer = it->song->SampleCount() - it->nextSample;
		auto min_len = std::min(samples_left_playback_buffer, out_length);
		copy(&*it, min_len);

		if (it->playPolicy == PlayPolicy::ONCE && it->nextSample == 0) {
			// Playback finished
			audio_manager.playbacks.Free(it.GetId());
		} else {
			// Playback wrapped around and there is space in output buffer
			copy(&*it, out_length - min_len);
		}
	}
}
