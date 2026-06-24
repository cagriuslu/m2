#include <m2/thirdparty/video/Texture.h>
#include <m2/thirdparty/video/Renderer.h>
#include <m2/thirdparty/video/Surface.h>
#include "SdlConversions.h"
#include <SDL2/SDL.h>

using namespace m2;
using namespace m2::thirdparty;
using namespace m2::thirdparty::video;

Texture Texture::Generate(Renderer& renderer, const uint32_t pixelFormat, const int w, const int h, const std::function<RGBA(int x, int y)>& pixelGenerator) {
	if (SDL_BITSPERPIXEL(pixelFormat) != 32) {
		throw M2_ERROR("Unsupported window pixel format");
	}

	auto* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, SDL_BITSPERPIXEL(pixelFormat), pixelFormat);
	SDL_LockSurface(surface);
	auto* pixels = static_cast<uint32_t*>(surface->pixels);
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			const auto rgba = pixelGenerator(x, y);
			*(pixels + x + y * w) = SDL_MapRGBA(surface->format, rgba.r, rgba.g, rgba.b, rgba.a);
		}
	}
	SDL_UnlockSurface(surface);

	auto* texture = SDL_CreateTextureFromSurface(static_cast<SDL_Renderer*>(renderer.RawHandle()), surface);
	SDL_FreeSurface(surface);
	return Texture{texture};
}
Texture Texture::CreateTargetableWindowSized(Renderer& renderer, const uint32_t pixelFormat) {
	if (SDL_BITSPERPIXEL(pixelFormat) != 32) {
		throw M2_ERROR("Unsupported window pixel format");
	}

	int w, h;
	SDL_GetRendererOutputSize(static_cast<SDL_Renderer*>(renderer.RawHandle()), &w, &h); // Get screen size
	return Texture{SDL_CreateTexture(static_cast<SDL_Renderer*>(renderer.RawHandle()), pixelFormat, SDL_TEXTUREACCESS_TARGET, w, h)};
}
Texture Texture::CaptureWindow(Renderer& renderer, const uint32_t pixelFormat) {
	if (SDL_BITSPERPIXEL(pixelFormat) != 32) {
		throw M2_ERROR("Unsupported window pixel format");
	}

	int w, h;
	SDL_GetRendererOutputSize(static_cast<SDL_Renderer*>(renderer.RawHandle()), &w, &h); // Get screen size

	auto* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, SDL_BITSPERPIXEL(pixelFormat), pixelFormat);
	SDL_RenderReadPixels(static_cast<SDL_Renderer*>(renderer.RawHandle()), nullptr, pixelFormat, surface->pixels, surface->pitch);

	auto* texture = SDL_CreateTextureFromSurface(static_cast<SDL_Renderer*>(renderer.RawHandle()), surface);
	SDL_FreeSurface(surface);
	return Texture{texture};
}
Texture Texture::AdoptRawTexture(void* rawSdlTexture) { return Texture{rawSdlTexture}; }
Texture Texture::CreateFromSurface(Renderer& renderer, void* sdlSurface, const bool linearFilter) {
	auto* const rawRenderer = static_cast<SDL_Renderer*>(renderer.RawHandle());
	SDL_Texture* texture{};
	if (linearFilter) {
		const char* previousHintPtr = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
		const auto previousHint = previousHintPtr ? std::string{previousHintPtr} : std::string{};
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
		texture = SDL_CreateTextureFromSurface(rawRenderer, static_cast<SDL_Surface*>(sdlSurface));
		const char* errorPtr = texture ? nullptr : SDL_GetError();
		if (not previousHint.empty()) {
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, previousHint.c_str());
		}
		if (errorPtr) {
			SDL_SetError("%s", errorPtr);
		}
	} else {
		texture = SDL_CreateTextureFromSurface(rawRenderer, static_cast<SDL_Surface*>(sdlSurface));
	}
	if (not texture) {
		throw M2_ERROR("Unable to create texture from surface: " + std::string{SDL_GetError()});
	}
	return Texture{texture};
}
Texture Texture::CreateFromImageFile(Renderer& renderer, const std::filesystem::path& imageFilePath) {
	const auto surface = Surface::CreateFromImageFile(imageFilePath);
	auto* texture = SDL_CreateTextureFromSurface(static_cast<SDL_Renderer*>(renderer.RawHandle()), static_cast<SDL_Surface*>(surface.RawHandle()));
	if (not texture) {
		throw M2_ERROR("Unable to create texture from surface: " + std::string{SDL_GetError()});
	}
	return Texture{texture};
}

VecI Texture::Dimensions() const {
	int w = 0, h = 0;
	SDL_QueryTexture(static_cast<SDL_Texture*>(_texture), nullptr, nullptr, &w, &h);
	return {w, h};
}

void Texture::DrawOnto(Renderer& renderer, const std::function<void()>& draw) {
	auto* const rawRenderer = static_cast<SDL_Renderer*>(renderer.RawHandle());
	auto* const previousTarget = SDL_GetRenderTarget(rawRenderer);
	SDL_SetRenderTarget(rawRenderer, static_cast<SDL_Texture*>(_texture));
	draw();
	SDL_SetRenderTarget(rawRenderer, previousTarget);
}

void Texture::RenderToWindow(Renderer& renderer) const {
	SDL_RenderCopy(static_cast<SDL_Renderer*>(renderer.RawHandle()), static_cast<SDL_Texture*>(_texture), nullptr, nullptr);
}
void Texture::Render(Renderer& renderer, const RectI& destinationPx) const {
	const auto sdlRect = ToSdlRect(destinationPx);
	SDL_RenderCopy(static_cast<SDL_Renderer*>(renderer.RawHandle()), static_cast<SDL_Texture*>(_texture), nullptr, &sdlRect);
}
void Texture::Render(Renderer& renderer, const RectI& sourceRect, const RectI& destinationRect) const {
	const auto sdlSrc = ToSdlRect(sourceRect);
	const auto sdlDst = ToSdlRect(destinationRect);
	SDL_RenderCopy(static_cast<SDL_Renderer*>(renderer.RawHandle()), static_cast<SDL_Texture*>(_texture), &sdlSrc, &sdlDst);
}
void Texture::RenderWithColorMod(Renderer& renderer, const RectI& destinationPx, const RGB& mod) const {
	SDL_SetTextureColorMod(static_cast<SDL_Texture*>(_texture), mod.r, mod.g, mod.b);
	Texture::Render(renderer, destinationPx);
	SDL_SetTextureColorMod(static_cast<SDL_Texture*>(_texture), 255, 255, 255);
}
void Texture::Render(Renderer& renderer, const RectI& sourceRect, const RectI& destinationRect, const double angleDegrees, const VecI& rotationCenter) const {
	const auto sdlSrc = ToSdlRect(sourceRect);
	const auto sdlDst = ToSdlRect(destinationRect);
	const auto sdlCenter = SDL_Point{rotationCenter.x, rotationCenter.y};
	if (SDL_RenderCopyEx(static_cast<SDL_Renderer*>(renderer.RawHandle()),
			static_cast<SDL_Texture*>(_texture), &sdlSrc, &sdlDst, angleDegrees, &sdlCenter,
			SDL_FLIP_NONE) != 0) {
		throw M2_ERROR(std::string{"SDL_RenderCopyEx failed: "} + SDL_GetError());
	}
}
void Texture::RenderGeometry(Renderer& renderer, std::span<const VecF> positionsPx, std::span<const VecF> texCoords, std::span<const int> indices) const {
	std::vector<SDL_Vertex> vertices(positionsPx.size());
	for (size_t i = 0; i < positionsPx.size(); ++i) {
		vertices[i] = SDL_Vertex{
			.position = ToSdlFPoint(positionsPx[i]),
			.color = SDL_Color{255, 255, 255, 255},
			.tex_coord = ToSdlFPoint(texCoords[i])};
	}
	if (SDL_RenderGeometry(static_cast<SDL_Renderer*>(renderer.RawHandle()),
			static_cast<SDL_Texture*>(_texture), vertices.data(),
			static_cast<int>(vertices.size()), indices.data(),
			static_cast<int>(indices.size())) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderGeometry failed: "} + SDL_GetError());
	}
}

Texture::ColorModGuard::ColorModGuard(void* texture, const RGB& mod) : _texture(texture) {
	SDL_SetTextureColorMod(static_cast<SDL_Texture*>(_texture), mod.r, mod.g, mod.b);
}
Texture::ColorModGuard::ColorModGuard(ColorModGuard&& other) noexcept : _texture(other._texture) {
	other._texture = nullptr;
}
Texture::ColorModGuard& Texture::ColorModGuard::operator=(ColorModGuard&& other) noexcept {
	std::swap(_texture, other._texture);
	return *this;
}
Texture::ColorModGuard::~ColorModGuard() {
	if (_texture) {
		SDL_SetTextureColorMod(static_cast<SDL_Texture*>(_texture), 255, 255, 255);
	}
}
Texture::ColorModGuard Texture::ScopedColorMod(const RGB& mod) const {
	return ColorModGuard{_texture, mod};
}

Texture::Texture(Texture&& other) noexcept : _texture(other._texture) {
	other._texture = nullptr;
}
Texture& Texture::operator=(Texture&& other) noexcept {
	std::swap(_texture, other._texture);
	return *this;
}
Texture::~Texture() {
	if (_texture) {
		SDL_DestroyTexture(static_cast<SDL_Texture*>(_texture));
		_texture = nullptr;
	}
}
