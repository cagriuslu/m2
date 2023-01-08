#include <m2/AudioManager.h>
#include <m2/Exception.h>
#include <m2/Game.hh>

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
	auto out_length = (size_t) length / 8; // 2 channels, 4 byte samples

	// TODO play multiple tracks
	// TODO play mono tracks
	// TODO support loop playback

	auto& pb = am._playbacks.front();

	size_t o = 0;
	while (o < out_length && pb.next_sample < pb.sample_count) {
		out_stream[o++] = pb.samples[pb.next_sample++];
	}
	while (o < out_length) {
		out_stream[o++] = {};
	}
}
