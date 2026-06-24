#include <m2/thirdparty/video/Shapes.h>
#include <m2/thirdparty/video/Detail.h>
#include <m2/Game.h>
#include <m2/common/Error.h>
#include <m2/Math.h>
#include <SDL2/SDL.h>

using namespace m2;
using namespace m2::thirdparty;
using namespace m2::thirdparty::video;

void video::FillTriangle(const VecF& point0, const VecF& point1, const VecF& point2, const RGBA& color) {
	const auto sdlColor = ToSdlColor(color);
	const SDL_Vertex vertices[3] = {
		SDL_Vertex{.position = ToSdlFPoint(point0), .color = sdlColor, .tex_coord = {}},
		SDL_Vertex{.position = ToSdlFPoint(point1), .color = sdlColor, .tex_coord = {}},
		SDL_Vertex{.position = ToSdlFPoint(point2), .color = sdlColor, .tex_coord = {}},
	};
	if (SDL_RenderGeometry(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), nullptr, vertices, 3, nullptr, 0) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderGeometry failed: "} + SDL_GetError());
	}
}
void video::FillTriangle(const VecF& point0, const VecF& point1, const VecF& point2, const RGBA& color0, const RGBA& color1, const RGBA& color2) {
	const auto sdlColor0 = ToSdlColor(color0);
	const auto sdlColor1 = ToSdlColor(color1);
	const auto sdlColor2 = ToSdlColor(color2);
	const SDL_Vertex vertices[3] = {
		SDL_Vertex{.position = ToSdlFPoint(point0), .color = sdlColor0, .tex_coord = {}},
		SDL_Vertex{.position = ToSdlFPoint(point1), .color = sdlColor1, .tex_coord = {}},
		SDL_Vertex{.position = ToSdlFPoint(point2), .color = sdlColor2, .tex_coord = {}},
	};
	if (SDL_RenderGeometry(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), nullptr, vertices, 3, nullptr, 0) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderGeometry failed: "} + SDL_GetError());
	}
}
void video::FillCircle(const VecF& centerPx, const RGBA& centerColor, const float radiusPx, const RGBA& edgeColor, const unsigned steps) {
	const auto sdlCenterColor = ToSdlColor(centerColor);
	const auto sdlEdgeColor = ToSdlColor(edgeColor);
	std::vector<SDL_Vertex> vertices(steps * 3);
	VecF spanPx{radiusPx, 0.0f}; // The vector that'll be rotated for the edges
	for (unsigned i = 0; i < steps; ++i) {
		// Center point of the triangle
		vertices.push_back(SDL_Vertex{.position = ToSdlFPoint(centerPx), .color = sdlCenterColor, .tex_coord = {}});
		// Second point of the triangle
		vertices.push_back(SDL_Vertex{.position = ToSdlFPoint(centerPx + spanPx), .color = sdlEdgeColor, .tex_coord = {}});
		// Rotate spanPx for next iteration
		spanPx = spanPx.Rotate(PI_MUL2 / static_cast<float>(steps));
		// Third point of the triangle
		vertices.push_back(SDL_Vertex{.position = ToSdlFPoint(centerPx + spanPx), .color = sdlEdgeColor, .tex_coord = {}});
	}
	if (SDL_RenderGeometry(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), nullptr, vertices.data(), static_cast<int>(vertices.size()), nullptr, 0) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderGeometry failed: "} + SDL_GetError());
	}
}
void video::FillRectangle(const RectF& rectPx, const RGBA& color) {
	SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), color.r, color.g, color.b, color.a);
	const SDL_FRect rect{rectPx.x, rectPx.y, rectPx.w, rectPx.h};
	if (SDL_RenderFillRectF(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), &rect) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderFillRectF failed: "} + SDL_GetError());
	}
}
void video::FillRectangle(const RectI& rectPx, const RGBA& color) {
	SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), color.r, color.g, color.b, color.a);
	const SDL_Rect rect{rectPx.x, rectPx.y, rectPx.w, rectPx.h};
	if (SDL_RenderFillRect(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), &rect) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderFillRect failed: "} + SDL_GetError());
	}
}

void video::DrawPoint(const VecF& pointPx, const RGBA& color) {
	SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()),
		color.r, color.g, color.b, color.a);
	if (SDL_RenderDrawPointF(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()),
			pointPx.GetX(), pointPx.GetY()) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderDrawPointF failed: "} + SDL_GetError());
	}
}
void video::DrawLine(const VecI& point0, const VecI& point1, const RGBA& color) {
	SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), color.r, color.g, color.b, color.a);
	if (SDL_RenderDrawLine(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), point0.x, point0.y, point1.x, point1.y) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderDrawLine failed: "} + SDL_GetError());
	}
}
void video::DrawLine(const VecF& point0, const VecF& point1, const RGBA& color) {
	SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), color.r, color.g, color.b, color.a);
	if (SDL_RenderDrawLineF(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), point0.GetX(), point0.GetY(), point1.GetX(), point1.GetY()) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderDrawLineF failed: "} + SDL_GetError());
	}
}
void video::DrawRectangle(const RectI& rect, const RGBA& color) {
	SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), color.r, color.g, color.b, color.a);
	const auto sdlRect = ToSdlRect(rect);
	if (SDL_RenderDrawRect(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), &sdlRect) < 0) {
		throw M2_ERROR(std::string{"SDL_RenderDrawRect failed: "} + SDL_GetError());
	}
}
void video::DrawRectangle(const RectF& rect, const RGBA& color) {
	SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), color.r, color.g, color.b, color.a);
	const auto sdlRect = ToSdlFRect(rect);
	if (SDL_RenderDrawRectF(static_cast<SDL_Renderer*>(M2_GAME.renderer->RawHandle()), &sdlRect) < 0) {
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
