#pragma once
#include <cstdint>

namespace m2::thirdparty::audio {
	/// Matches SDL's audio callback contract: fill `stream` with `lengthBytes` bytes of samples.
	using BufferCallback = void(*)(void* userData, uint8_t* stream, int lengthBytes);

	class Device {
		uint32_t _deviceId{};                   // 0 == invalid / moved-from
		uint16_t _obtainedBufferSizeInSamples{};

		Device(uint32_t deviceId, uint16_t obtainedBufferSizeInSamples)
			: _deviceId(deviceId), _obtainedBufferSizeInSamples(obtainedBufferSizeInSamples) {}

	public:
		/// Opens the default audio output device with the engine's fixed format
		/// (float-stereo at the synthesizer's sample rate). Throws on failure or unexpected format.
		static Device Open(BufferCallback callback, void* userData);

		/// Copy not allowed
		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		/// Move allowed
		Device(Device&&) noexcept;
		Device& operator=(Device&&) noexcept;
		/// Destructor: pauses and closes the device
		~Device();

		[[nodiscard]] uint16_t ObtainedBufferSizeInSamples() const { return _obtainedBufferSizeInSamples; }

		void Pause();
		void Resume();
	};
}
