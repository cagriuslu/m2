#include <m2/thirdparty/audio/Device.h>
#include <m2/audio/synthesizer/Detail.h>
#include <m2/common/Error.h>
#include <SDL2/SDL.h>

using namespace m2::thirdparty::audio;

namespace {
	/// It's not possible to queue audio during the audio callback. A high callback frequency lets
	/// new audio be queued faster.
	constexpr int AUDIO_CALLBACK_FREQUENCY = 90;
}

Device Device::Open(BufferCallback callback, void* userData) {
	SDL_AudioSpec want{};
	want.freq = m2::audio::synthesizer::gDefaultAudioSampleRate;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.samples = static_cast<uint16_t>(want.freq / AUDIO_CALLBACK_FREQUENCY);
	want.callback = callback;
	want.userdata = userData;

	SDL_AudioSpec obtained{};
	const auto deviceId = SDL_OpenAudioDevice(nullptr, 0, &want, &obtained,
		SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
	if (not deviceId) {
		throw M2_ERROR("SDL_OpenAudioDevice error: " + std::string{SDL_GetError()});
	}
	if (want.format != obtained.format) {
		SDL_CloseAudioDevice(deviceId);
		throw M2_ERROR("Undesired audio format");
	}
	return Device{deviceId, obtained.samples};
}

Device::Device(Device&& other) noexcept
	: _deviceId(other._deviceId), _obtainedBufferSizeInSamples(other._obtainedBufferSizeInSamples) {
	other._deviceId = 0;
}
Device& Device::operator=(Device&& other) noexcept {
	std::swap(_deviceId, other._deviceId);
	std::swap(_obtainedBufferSizeInSamples, other._obtainedBufferSizeInSamples);
	return *this;
}
Device::~Device() {
	if (_deviceId) {
		SDL_PauseAudioDevice(_deviceId, 1);
		SDL_CloseAudioDevice(_deviceId);
		_deviceId = 0;
	}
}

void Device::Pause()  { SDL_PauseAudioDevice(_deviceId, 1); }
void Device::Resume() { SDL_PauseAudioDevice(_deviceId, 0); }
