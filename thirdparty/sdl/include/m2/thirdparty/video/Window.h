#pragma once
#include "m2/common/Meta.h"
#include <m2/common/math/VecI.h>
#include "Renderer.h"
#include <cstdint>

// # Logical Pixels vs. Window Pixel Coordinate Systems Summary
//
// Logical Pixels (formerly window coordinates): An arbitrary, unitless floating-point coordinate system. It is used
//     to define window sizes (`SDL_GetWindowSize`), create windows (`SDL_CreateWindow`), and report input event
//     coordinates. Drawing logic operates in this system so that elements scale relative to the window borders
//     rather than physical pixels.
// Window Pixels (Render Coordinates): The actual physical pixels on a display. This is the coordinate system that
//     `SDL_Render` functions and `SDL_GetRenderOutputSize` deal with during the final rendering pass.
// Window Pixel Density: The ratio defining how many pixels fit into a single unit of the logical pixel coordinate
//     system.
//
// The relationship between logical pixels and physical pixels varies depending on the host operating system:
// macOS: Logical pixels and physical pixels are separate. The pixel density reflects the scale factor.
// Windows: Logical pixels always equal physical pixels (`window units == pixel units`). Sizing adjustments rely
//     on `SDL_WindowDisplayScale` when users request scaled rendering.
// Linux: Utilizes a hybrid mixture of both approaches depending on whether the system is running under X11 or Wayland.
//
// Guidelines:
// Calculating Density: Don't rely on `SDL_WindowPixelDensity`. Instead, calculate the pixel density manually and more
//     precisely by comparing the output of `SDL_GetRenderOutputSize` (or `SDL_GetWindowSizeInPixels`) against the
//     logical pixel dimensions. This guarantees properly rounded calculations.
// Engine UI Strategy: Display scaling should not change the absolute amount of layout content visible on the screen or
//     alter the perceived size of the UI. If an element is designed to take up 50% of the window, it must always take
//     up 50% across all platforms.
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
