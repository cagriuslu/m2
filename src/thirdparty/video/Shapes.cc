#include <m2/thirdparty/video/Shapes.h>
#include <m2/Game.h>
#include <m2/Error.h>
#include <m2/Math.h>
#include <SDL2/SDL.h>

using namespace m2;
using namespace m2::thirdparty;
using namespace m2::thirdparty::video;

void video::FillTriangle(const VecF& point0, const VecF& point1, const VecF& point2, const RGBA& color) {
	const auto sdlColor = static_cast<SDL_Color>(color);
	const SDL_Vertex vertices[3] = {
		SDL_Vertex{.position = static_cast<SDL_FPoint>(point0), .color = sdlColor, .tex_coord = {}},
		SDL_Vertex{.position = static_cast<SDL_FPoint>(point1), .color = sdlColor, .tex_coord = {}},
		SDL_Vertex{.position = static_cast<SDL_FPoint>(point2), .color = sdlColor, .tex_coord = {}},
	};
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	if (SDL_RenderGeometry(M2_GAME.renderer, nullptr, vertices, 3, nullptr, 0) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderGeometry failed: "} + SDL_GetError());
	}
}

void video::FillCircle(const VecF& centerPx, const RGBA& centerColor, const float radiusPx, const RGBA& edgeColor, const unsigned steps) {
	const auto sdlCenterColor = static_cast<SDL_Color>(centerColor);
	const auto sdlEdgeColor = static_cast<SDL_Color>(edgeColor);
	std::vector<SDL_Vertex> vertices(steps * 3);
	VecF spanPx{radiusPx, 0.0f}; // The vector that'll be rotated for the edges
	for (unsigned i = 0; i < steps; ++i) {
		// Center point of the triangle
		vertices.push_back(SDL_Vertex{.position = static_cast<SDL_FPoint>(centerPx), .color = sdlCenterColor, .tex_coord = {}});
		// Second point of the triangle
		vertices.push_back(SDL_Vertex{.position = static_cast<SDL_FPoint>(centerPx + spanPx), .color = sdlEdgeColor, .tex_coord = {}});
		// Rotate spanPx for next iteration
		spanPx = spanPx.Rotate(PI_MUL2 / static_cast<float>(steps));
		// Third point of the triangle
		vertices.push_back(SDL_Vertex{.position = static_cast<SDL_FPoint>(centerPx + spanPx), .color = sdlEdgeColor, .tex_coord = {}});
	}
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	if (SDL_RenderGeometry(M2_GAME.renderer, nullptr, vertices.data(), static_cast<int>(vertices.size()), nullptr, 0) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderGeometry failed: "} + SDL_GetError());
	}
}
