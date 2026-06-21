#pragma once
#include <m2/video/Color.h>
#include <m2/math/RectI.h>

namespace m2::thirdparty::video {
	class Texture {
		void* _texture{};

		explicit Texture(void* texture) : _texture(texture) {}

	public:
		static Texture Generate(int w, int h, const std::function<RGBA(int x, int y)>&);
		static Texture CreateTargetableWindowSized();
		static Texture CaptureWindow();

		// Copy not allowed
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;
		// Move allowed
		Texture(Texture&&) noexcept;
		Texture& operator=(Texture&&) noexcept;
		// Destructor
		virtual ~Texture();
	};
}
