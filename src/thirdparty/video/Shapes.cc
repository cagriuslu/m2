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
void video::FillTriangle(const VecF& point0, const VecF& point1, const VecF& point2, const RGBA& color0, const RGBA& color1, const RGBA& color2) {
	const auto sdlColor0 = static_cast<SDL_Color>(color0);
	const auto sdlColor1 = static_cast<SDL_Color>(color1);
	const auto sdlColor2 = static_cast<SDL_Color>(color2);
	const SDL_Vertex vertices[3] = {
		SDL_Vertex{.position = static_cast<SDL_FPoint>(point0), .color = sdlColor0, .tex_coord = {}},
		SDL_Vertex{.position = static_cast<SDL_FPoint>(point1), .color = sdlColor1, .tex_coord = {}},
		SDL_Vertex{.position = static_cast<SDL_FPoint>(point2), .color = sdlColor2, .tex_coord = {}},
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
void video::FillRectangle(const RectF& rectPx, const RGBA& color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	const SDL_FRect rect{rectPx.x, rectPx.y, rectPx.w, rectPx.h};
	if (SDL_RenderFillRectF(M2_GAME.renderer, &rect) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderFillRectF failed: "} + SDL_GetError());
	}
}
void video::FillRectangle(const RectI& rectPx, const RGBA& color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	const SDL_Rect rect{rectPx.x, rectPx.y, rectPx.w, rectPx.h};
	if (SDL_RenderFillRect(M2_GAME.renderer, &rect) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderFillRect failed: "} + SDL_GetError());
	}
}

void video::DrawLine(const VecI& point0, const VecI& point1, const RGBA& color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	if (SDL_RenderDrawLine(M2_GAME.renderer, point0.x, point0.y, point1.x, point1.y) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderDrawLine failed: "} + SDL_GetError());
	}
}
void video::DrawLine(const VecF& point0, const VecF& point1, const RGBA& color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	if (SDL_RenderDrawLineF(M2_GAME.renderer, point0.GetX(), point0.GetY(), point1.GetX(), point1.GetY()) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderDrawLineF failed: "} + SDL_GetError());
	}
}
void video::DrawRectangle(const RectI& rect, const RGBA& color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	const auto sdlRect = static_cast<SDL_Rect>(rect);
	if (SDL_RenderDrawRect(M2_GAME.renderer, &sdlRect) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderDrawRect failed: "} + SDL_GetError());
	}
}
void video::DrawRectangle(const RectF& rect, const RGBA& color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	const auto sdlRect = static_cast<SDL_FRect>(rect);
	if (SDL_RenderDrawRectF(M2_GAME.renderer, &sdlRect) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderDrawRectF failed: "} + SDL_GetError());
	}
}
void video::DrawRectangle(const VecF& center, const float width, const float height, const float orientationRads, const RGBA& color) {
	const auto topLeft = center + VecF{-width / 2.0f, -height / 2.0f}.Rotate(orientationRads);
	const auto topRight = center + VecF{width / 2.0f, -height / 2.0f}.Rotate(orientationRads);
	const auto bottomLeft = center + VecF{-width / 2.0f, height / 2.0f}.Rotate(orientationRads);
	const auto bottomRight = center + VecF{width / 2.0f, height / 2.0f}.Rotate(orientationRads);
	DrawLine(topLeft, topRight, color);
	DrawLine(topRight, bottomRight, color);
	DrawLine(bottomRight, bottomLeft, color);
	DrawLine(bottomLeft, topLeft, color);
}
