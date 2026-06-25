#include <m2/thirdparty/audio/Device.h>
#include <m2/common/Constants.h>
#include <m2/common/Error.h>
#include <SDL3/SDL.h>
#include <vector>

using namespace m2::thirdparty::audio;

namespace m2::thirdparty::audio {
	struct CallbackState {
		BufferFillerCallback engineCallback{};
		void* engineUserData{};
		std::vector<uint8_t> scratch;
	};
}

namespace {
	constexpr int AUDIO_CALLBACK_FREQUENCY = 80;

	void Callback(void* userData, SDL_AudioStream* stream, int additionalBytes, int /*totalBytes*/) {
		if (additionalBytes <= 0) {
			return;
		}
		auto* adapterState = static_cast<CallbackState*>(userData);
		if (static_cast<int>(adapterState->scratch.size()) < additionalBytes) {
			adapterState->scratch.resize(additionalBytes);
		}
		// additionalBytes is frame-aligned for F32-stereo, matching the engine's sizeof(AudioSample).
		adapterState->engineCallback(adapterState->engineUserData, adapterState->scratch.data(), additionalBytes);
		SDL_PutAudioStreamData(stream, adapterState->scratch.data(), additionalBytes);
	}
}

Device Device::Open(BufferFillerCallback callback, void* userData) {
	auto adapterState = std::make_unique<CallbackState>();
	adapterState->engineCallback = callback;
	adapterState->engineUserData = userData;

	SDL_AudioSpec spec{};
	spec.format = SDL_AUDIO_F32;
	spec.channels = 2;
	spec.freq = DEFAULT_AUDIO_SAMPLE_RATE;
	auto* stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, &Callback, adapterState.get());
	if (not stream) {
		throw M2_ERROR("SDL_OpenAudioDeviceStream error: " + std::string{SDL_GetError()});
	}
	const auto obtainedBufferSizeInSamples = static_cast<uint16_t>(spec.freq / AUDIO_CALLBACK_FREQUENCY);
	return Device{stream, std::move(adapterState), obtainedBufferSizeInSamples};
}

Device::Device(void* stream, std::unique_ptr<CallbackState> callbackState, uint16_t obtainedBufferSizeInSamples)
	: _stream(stream), _adapterState(std::move(callbackState)), _obtainedBufferSizeInSamples(obtainedBufferSizeInSamples) {}

Device::Device(Device&& other) noexcept
	: _stream(other._stream), _adapterState(std::move(other._adapterState)),
	  _obtainedBufferSizeInSamples(other._obtainedBufferSizeInSamples) {
	other._stream = nullptr;
}
Device& Device::operator=(Device&& other) noexcept {
	std::swap(_stream, other._stream);
	std::swap(_adapterState, other._adapterState);
	std::swap(_obtainedBufferSizeInSamples, other._obtainedBufferSizeInSamples);
	return *this;
}
Device::~Device() {
	if (_stream) {
		// Destroying the stream stops the callback thread (and joins it) before _adapterState is freed below.
		SDL_PauseAudioStreamDevice(static_cast<SDL_AudioStream*>(_stream));
		SDL_DestroyAudioStream(static_cast<SDL_AudioStream*>(_stream));
		_stream = nullptr;
	}
}

void Device::Pause()  { SDL_PauseAudioStreamDevice(static_cast<SDL_AudioStream*>(_stream)); }
void Device::Resume() { SDL_ResumeAudioStreamDevice(static_cast<SDL_AudioStream*>(_stream)); }
