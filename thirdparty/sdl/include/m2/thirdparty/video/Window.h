#pragma once
#include "m2/common/Meta.h"
#include <m2/common/math/VecI.h>
#include "Renderer.h"
#include <cstdint>

// # Logical Pixels vs. Window Pixels
//
// Logical Pixels (Window Coordinates): A unitless floating-point coordinate system over the window. User input
//     coordinates, `SDL_GetWindowSize`, and `SDL_CreateWindow` use this coordinate system. The engine's drawing
//     utilities use this coordinate system so that drawn elements scale properly with different window sizes.
// Window Pixels (Render Coordinates): The actual physical pixels on a display. This is the coordinate system that
//     `SDL_Render*` functions use during rendering.
// Window Pixel Density: The ratio defining how many pixels fit into a single unit of the logical pixel coordinate
//     system.
//
// The relationship between logical pixels and physical pixels varies depending on the host operating system:
//     macOS: Logical and physical pixels are separate. The pixel density reflects the scale factor.
//     Windows: Logical pixels = physical pixels. `SDL_WindowDisplayScale` reflects the scaling requested by the user.
//     Linux: Depends on whether the system uses X11 or Wayland.
//
// Guidelines:
// Calculating Density: Don't rely on `SDL_WindowPixelDensity`. Instead, calculate the pixel density manually and more
//     precisely by comparing the output of `SDL_GetRenderOutputSize` (or `SDL_GetWindowSizeInPixels`) against the
//     logical pixel dimensions. This guarantees properly rounded calculations.
// Engine UI Strategy: Display scaling should not change the amount of content visible on the screen or alter the
//     perceived size of the UI. If an element is designed to take up 50% of the window, it must always take up 50%
//     across all platforms.
//

namespace m2::thirdparty::video {
	class Window {
		void* _window{};

		friend Renderer;
		explicit Window(void* window) : _window(window) {}

	public:
		static expected<Window> Create(VecI minDimensions, const char* title);
		static expected<std::pair<Window,Renderer>> Create2(VecI initialSize, VecI minimumSize, const char* title, bool startMaximized);

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) noexcept;
		Window& operator=(Window&&) noexcept;
		~Window();

		[[nodiscard]] void* RawHandle() const { return _window; } // in-layer only
		[[nodiscard]] uint32_t GetPixelFormat() const; // throws on UNKNOWN
		[[nodiscard]] VecI GetSize() const;
	};

	/// Usable bounds size (excludes taskbar/menu bar/dock) of the primary display, in screen
	/// coordinates. Throws on SDL failure.
	VecI GetPrimaryDisplayUsableSize();
}
