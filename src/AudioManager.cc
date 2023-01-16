#include <m2/AudioManager.h>
#include <m2/Exception.h>
#include <m2/Game.h>

m2::AudioManager::AudioManager() {
	SDL_AudioSpec want{};
	want.freq = 48000;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.samples = 48000;
	want.callback = audio_callback;
	want.userdata = nullptr; // Passing `this` won't work, object may be moved/copied
	sdl_audio_device_id = SDL_OpenAudioDevice(nullptr, 0, &want, &sdl_audio_spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
	if (!sdl_audio_device_id) {
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	if (want.format != sdl_audio_spec.format) {
		throw M2FATAL("Undesired audio format");
	}
}

m2::PlaybackId m2::AudioManager::loop(const AudioSample* samples, size_t sample_count, size_t start_sample) {
	if (0 < _playbacks.size()) {
		throw M2ERROR("Playing multiple tracks not supported");
	}

	_playbacks.push_back({.playback_id = _next_playback_id++, .samples = samples, .sample_count = sample_count, .play_policy = LOOP, .next_sample = start_sample});
	SDL_PauseAudioDevice(sdl_audio_device_id, 0);
	return _playbacks.back().playback_id;
}

void m2::AudioManager::stop(PlaybackId id) {
	std::erase_if(_playbacks, [=](const Playback& pb) {
		return pb.playback_id == id;
	});
	if (_playbacks.empty()) {
		SDL_PauseAudioDevice(sdl_audio_device_id, 1);
	}
}

void m2::AudioManager::audio_callback(MAYBE void* user_data, uint8_t* stream, int length) {
	auto& am = *GAME.audio_manager;
	auto* out_stream = reinterpret_cast<AudioSample*>(stream);
	auto out_length = (size_t) length / sizeof(AudioSample);

	auto& pb = am._playbacks.front();
	if (pb.play_policy == ONCE) {
		throw M2ERROR("Playing audio once not supported");
	}
	if (pb.sample_count < out_length) {
		throw M2ERROR("Playing short audio is not supported");
	}
	if (pb.sample_count < pb.next_sample) {
		throw M2FATAL("Implementation error");
	}

	auto copy = [&](size_t len) {
		memcpy(out_stream, pb.samples + pb.next_sample, len * sizeof(AudioSample));
		pb.next_sample += len;
		if (pb.sample_count == pb.next_sample) {
			pb.next_sample = 0;
		}
	};

	// First copy
	auto min_len = std::min(pb.sample_count - pb.next_sample, out_length);
	copy(min_len);
	// In case playback wrapped around
	copy(out_length - min_len);
}
