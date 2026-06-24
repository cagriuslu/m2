#pragma once
#include "m2/common/Meta.h"   // expected
#include <m2/math/VecI.h>
#include <cstdint>

namespace m2::thirdparty::video {
	class Window {
		void* _window{};
		explicit Window(void* window) : _window(window) {}

	public:
		static expected<Window> Create(VecI minDimensions, const char* title);

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) noexcept;
		Window& operator=(Window&&) noexcept;
		~Window();

		[[nodiscard]] void* RawHandle() const { return _window; } // in-layer only
		[[nodiscard]] uint32_t GetPixelFormat() const; // throws on UNKNOWN
	};
}
