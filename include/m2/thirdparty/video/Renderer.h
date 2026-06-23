#pragma once
#include "m2/common/Meta.h"

namespace m2::thirdparty::video {
	class Renderer {
		void* _renderer{};

		explicit Renderer(void* renderer) : _renderer(renderer) {}

	public:
		static expected<Renderer> Create(void* sdlWindow, bool graphicsPixelated);

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept;
		Renderer& operator=(Renderer&&) noexcept;
		virtual ~Renderer();

		[[nodiscard]] void* RawHandle() const { return _renderer; } // TODO remove
	};
}
