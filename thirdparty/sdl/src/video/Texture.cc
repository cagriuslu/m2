#include <m2/thirdparty/video/Texture.h>
#include <m2/thirdparty/video/Renderer.h>
#include <m2/thirdparty/video/Surface.h>
#include "SdlConversions.h"
#include <SDL3/SDL.h>
#include <vector>

using namespace m2;
using namespace m2::thirdparty;
using namespace m2::thirdparty::video;

Texture Texture::Generate(Renderer& renderer, const uint32_t pixelFormat, const int w, const int h, const std::function<RGBA(int x, int y)>& pixelGenerator) {
	if (SDL_BYTESPERPIXEL(pixelFormat) != 4) {
		throw M2_ERROR("Unsupported window pixel format");
	}

	auto* surface = SDL_CreateSurface(w, h, static_cast<SDL_PixelFormat>(pixelFormat));
	const auto* formatDetails = SDL_GetPixelFormatDetails(static_cast<SDL_PixelFormat>(pixelFormat));
	SDL_LockSurface(surface);
	auto* pixels = static_cast<uint32_t*>(surface->pixels);
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			const auto rgba = pixelGenerator(x, y);
			*(pixels + x + y * w) = SDL_MapRGBA(formatDetails, nullptr, rgba.r, rgba.g, rgba.b, rgba.a);
		}
	}
	SDL_UnlockSurface(surface);

	auto* texture = SDL_CreateTextureFromSurface(static_cast<SDL_Renderer*>(renderer.RawHandle()), surface);
	SDL_DestroySurface(surface);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
	return Texture{texture};
}
Texture Texture::CreateTargetableWindowSized(Renderer& renderer, const uint32_t pixelFormat) {
	if (SDL_BYTESPERPIXEL(pixelFormat) != 4) {
		throw M2_ERROR("Unsupported window pixel format");
	}

	int w, h;
	SDL_GetCurrentRenderOutputSize(static_cast<SDL_Renderer*>(renderer.RawHandle()), &w, &h); // Get screen size
	auto* texture = SDL_CreateTexture(static_cast<SDL_Renderer*>(renderer.RawHandle()), static_cast<SDL_PixelFormat>(pixelFormat), SDL_TEXTUREACCESS_TARGET, w, h);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
	return Texture{texture};
}
Texture Texture::CaptureWindow(Renderer& renderer, const uint32_t pixelFormat) {
	if (SDL_BYTESPERPIXEL(pixelFormat) != 4) {
		throw M2_ERROR("Unsupported window pixel format");
	}

	auto* const rawRenderer = static_cast<SDL_Renderer*>(renderer.RawHandle());
	// SDL3 SDL_RenderReadPixels returns a freshly allocated surface (in the renderer's own format).
	auto* surface = SDL_RenderReadPixels(rawRenderer, nullptr);
	if (not surface) {
		throw M2_ERROR("Unable to read pixels from renderer: " + std::string{SDL_GetError()});
	}
	// Convert to the requested format if the read-back format differs.
	if (surface->format != static_cast<SDL_PixelFormat>(pixelFormat)) {
		auto* converted = SDL_ConvertSurface(surface, static_cast<SDL_PixelFormat>(pixelFormat));
		SDL_DestroySurface(surface);
		if (not converted) {
			throw M2_ERROR("Unable to convert captured surface: " + std::string{SDL_GetError()});
		}
		surface = converted;
	}

	auto* texture = SDL_CreateTextureFromSurface(rawRenderer, surface);
	SDL_DestroySurface(surface);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
	return Texture{texture};
}
Texture Texture::AdoptRawTexture(void* rawSdlTexture) { return Texture{rawSdlTexture}; }
Texture Texture::CreateFromSurface(Renderer& renderer, void* sdlSurface, const bool linearFilter) {
	auto* const rawRenderer = static_cast<SDL_Renderer*>(renderer.RawHandle());
	auto* texture = SDL_CreateTextureFromSurface(rawRenderer, static_cast<SDL_Surface*>(sdlSurface));
	if (not texture) {
		throw M2_ERROR("Unable to create texture from surface: " + std::string{SDL_GetError()});
	}
	SDL_SetTextureScaleMode(texture, linearFilter ? SDL_SCALEMODE_LINEAR : SDL_SCALEMODE_NEAREST);
	return Texture{texture};
}
Texture Texture::CreateFromImageFile(Renderer& renderer, const std::filesystem::path& imageFilePath) {
	const auto surface = Surface::CreateFromImageFile(imageFilePath);
	auto* texture = SDL_CreateTextureFromSurface(static_cast<SDL_Renderer*>(renderer.RawHandle()), static_cast<SDL_Surface*>(surface.RawHandle()));
	if (not texture) {
		throw M2_ERROR("Unable to create texture from surface: " + std::string{SDL_GetError()});
	}
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
	return Texture{texture};
}

VecF Texture::Dimensions() const {
	float w = 0.0f, h = 0.0f;
	SDL_GetTextureSize(static_cast<SDL_Texture*>(_texture), &w, &h);
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
	SDL_RenderTexture(static_cast<SDL_Renderer*>(renderer.RawHandle()), static_cast<SDL_Texture*>(_texture), nullptr, nullptr);
}
void Texture::Render(Renderer& renderer, const RectF& destination) const {
	const auto destPx = destination.Scale(renderer.GetPixelsPerWindowUnit());
	const auto sdlRect = ToSdlFRect(destPx);
	SDL_RenderTexture(static_cast<SDL_Renderer*>(renderer.RawHandle()), static_cast<SDL_Texture*>(_texture), nullptr, &sdlRect);
}
void Texture::Render(Renderer& renderer, const RectI& sourceRect, const RectF& destination) const {
	const auto destPx = destination.Scale(renderer.GetPixelsPerWindowUnit());
	const auto sdlSrc = ToSdlFRect(sourceRect);
	const auto sdlDst = ToSdlFRect(destPx);
	SDL_RenderTexture(static_cast<SDL_Renderer*>(renderer.RawHandle()), static_cast<SDL_Texture*>(_texture), &sdlSrc, &sdlDst);
}
void Texture::RenderWithColorMod(Renderer& renderer, const RectF& destination, const RGB& mod) const {
	SDL_SetTextureColorMod(static_cast<SDL_Texture*>(_texture), mod.r, mod.g, mod.b);
	Texture::Render(renderer, destination);
	SDL_SetTextureColorMod(static_cast<SDL_Texture*>(_texture), 255, 255, 255);
}
void Texture::Render(Renderer& renderer, const RectI& sourceRect, const RectF& destination, const double angleDegrees, const VecI& rotationCenter) const {
	const auto sdlSrc = ToSdlFRect(sourceRect);
	const auto destPx = destination.Scale(renderer.GetPixelsPerWindowUnit());
	const auto sdlDst = ToSdlFRect(destPx);
	const auto sdlCenter = SDL_FPoint{static_cast<float>(rotationCenter.x), static_cast<float>(rotationCenter.y)};
	if (not SDL_RenderTextureRotated(static_cast<SDL_Renderer*>(renderer.RawHandle()),
			static_cast<SDL_Texture*>(_texture), &sdlSrc, &sdlDst, angleDegrees, &sdlCenter,
			SDL_FLIP_NONE)) {
		throw M2_ERROR(std::string{"SDL_RenderTextureRotated failed: "} + SDL_GetError());
	}
}
void Texture::RenderGeometry(Renderer& renderer, std::span<const VecF> positions, std::span<const VecF> texCoords, std::span<const int> indices) const {
	const auto pixelsPerUnit = renderer.GetPixelsPerWindowUnit();

	std::vector<SDL_Vertex> vertices(positions.size());
	for (size_t i = 0; i < positions.size(); ++i) {
		vertices[i] = SDL_Vertex{
			.position = ToSdlFPoint(positions[i].Scale(pixelsPerUnit)),
			.color = SDL_FColor{1.0f, 1.0f, 1.0f, 1.0f},
			.tex_coord = ToSdlFPoint(texCoords[i])};
	}
	if (not SDL_RenderGeometry(static_cast<SDL_Renderer*>(renderer.RawHandle()),
			static_cast<SDL_Texture*>(_texture), vertices.data(),
			static_cast<int>(vertices.size()), indices.data(),
			static_cast<int>(indices.size()))) {
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
