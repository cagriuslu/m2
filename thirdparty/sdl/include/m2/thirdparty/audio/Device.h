#pragma once
#include <cstdint>
#include <memory>

namespace m2::thirdparty::audio {
	using BufferFillerCallback = void(*)(void* userData, uint8_t* stream, int lengthBytes);
	struct CallbackState;

	class Device {
		void* _stream{};
		std::unique_ptr<CallbackState> _adapterState;
		uint16_t _obtainedBufferSizeInSamples{};

		Device(void* stream, std::unique_ptr<CallbackState> callbackState, uint16_t obtainedBufferSizeInSamples);

	public:
		/// Opens the default audio output device with the engine's fixed format
		/// (float-stereo at the synthesizer's sample rate). Throws on failure.
		static Device Open(BufferFillerCallback callback, void* userData);

		/// Copy not allowed
		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		/// Move allowed
		Device(Device&&) noexcept;
		Device& operator=(Device&&) noexcept;
		/// Destructor: pauses and destroys the device stream
		~Device();

		[[nodiscard]] uint16_t ObtainedBufferSizeInSamples() const { return _obtainedBufferSizeInSamples; }

		void Pause();
		void Resume();
	};
}
