#include <m2/thirdparty/video/Shapes.h>
#include <m2/thirdparty/video/Renderer.h>
#include "SdlConversions.h"
#include <m2/common/Error.h>
#include <m2/common/Math.h>
#include <SDL3/SDL.h>
#include <vector>

using namespace m2;
using namespace m2::thirdparty;
using namespace m2::thirdparty::video;

void video::FillTriangle(Renderer& renderer, const VecF& point0Lpx, const VecF& point1Lpx, const VecF& point2Lpx, const RGBA& color) {
	const auto pixelsPerUnit = renderer.GetPixelsPerWindowUnit();
	const auto sdlColor = ToSdlFColor(color);
	const SDL_Vertex vertices[3] = {
		SDL_Vertex{.position = ToSdlFPoint(point0Lpx.Scale(pixelsPerUnit)), .color = sdlColor, .tex_coord = {}},
		SDL_Vertex{.position = ToSdlFPoint(point1Lpx.Scale(pixelsPerUnit)), .color = sdlColor, .tex_coord = {}},
		SDL_Vertex{.position = ToSdlFPoint(point2Lpx.Scale(pixelsPerUnit)), .color = sdlColor, .tex_coord = {}},
	};
	if (not SDL_RenderGeometry(static_cast<SDL_Renderer*>(renderer.RawHandle()), nullptr, vertices, 3, nullptr, 0)) {
		throw M2_ERROR(std::string{"SDL_RenderGeometry failed: "} + SDL_GetError());
	}
}
void video::FillTriangle(Renderer& renderer, const VecF& point0Lpx, const VecF& point1Lpx, const VecF& point2Lpx, const RGBA& color0, const RGBA& color1, const RGBA& color2) {
	const auto pixelsPerUnit = renderer.GetPixelsPerWindowUnit();
	const auto sdlColor0 = ToSdlFColor(color0);
	const auto sdlColor1 = ToSdlFColor(color1);
	const auto sdlColor2 = ToSdlFColor(color2);
	const SDL_Vertex vertices[3] = {
		SDL_Vertex{.position = ToSdlFPoint(point0Lpx.Scale(pixelsPerUnit)), .color = sdlColor0, .tex_coord = {}},
		SDL_Vertex{.position = ToSdlFPoint(point1Lpx.Scale(pixelsPerUnit)), .color = sdlColor1, .tex_coord = {}},
		SDL_Vertex{.position = ToSdlFPoint(point2Lpx.Scale(pixelsPerUnit)), .color = sdlColor2, .tex_coord = {}},
	};
	if (not SDL_RenderGeometry(static_cast<SDL_Renderer*>(renderer.RawHandle()), nullptr, vertices, 3, nullptr, 0)) {
		throw M2_ERROR(std::string{"SDL_RenderGeometry failed: "} + SDL_GetError());
	}
}
void video::FillCircle(Renderer& renderer, const VecF& centerLpx, const RGBA& centerColor, const float radius, const RGBA& edgeColor, const unsigned steps) {
	const auto pixelsPerUnit = renderer.GetPixelsPerWindowUnit();
	const auto centerPx = centerLpx.Scale(pixelsPerUnit);
	const auto radiusPx = radius * pixelsPerUnit.GetY();

	const auto sdlCenterColor = ToSdlFColor(centerColor);
	const auto sdlEdgeColor = ToSdlFColor(edgeColor);
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
	if (not SDL_RenderGeometry(static_cast<SDL_Renderer*>(renderer.RawHandle()), nullptr, vertices.data(), static_cast<int>(vertices.size()), nullptr, 0)) {
		throw M2_ERROR(std::string{"SDL_RenderGeometry failed: "} + SDL_GetError());
	}
}
void video::FillRectangle(Renderer& renderer, const RectF& rectLpx, const RGBA& color) {
	const auto pixelsPerUnit = renderer.GetPixelsPerWindowUnit();
	const auto rectPx = rectLpx.Scale(pixelsPerUnit);

	SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(renderer.RawHandle()), color.r, color.g, color.b, color.a);
	const SDL_FRect sdlRect{rectPx.x, rectPx.y, rectPx.w, rectPx.h};
	if (not SDL_RenderFillRect(static_cast<SDL_Renderer*>(renderer.RawHandle()), &sdlRect)) {
		throw M2_ERROR(std::string{"SDL_RenderFillRect failed: "} + SDL_GetError());
	}
}

void video::DrawPoint(Renderer& renderer, const VecF& pointLpx, const RGBA& color) {
	const auto pixelsPerUnit = renderer.GetPixelsPerWindowUnit();
	const auto pointPx = pointLpx.Scale(pixelsPerUnit);

	SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(renderer.RawHandle()),
		color.r, color.g, color.b, color.a);
	if (not SDL_RenderPoint(static_cast<SDL_Renderer*>(renderer.RawHandle()),
			pointPx.GetX(), pointPx.GetY())) {
		throw M2_ERROR(std::string{"SDL_RenderPoint failed: "} + SDL_GetError());
	}
}
void video::DrawLine(Renderer& renderer, const VecF& point0Lpx, const VecF& point1Lpx, const RGBA& color) {
	const auto pixelsPerUnit = renderer.GetPixelsPerWindowUnit();
	const auto point0Px = point0Lpx.Scale(pixelsPerUnit);
	const auto point1Px = point1Lpx.Scale(pixelsPerUnit);

	SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(renderer.RawHandle()), color.r, color.g, color.b, color.a);
	if (not SDL_RenderLine(static_cast<SDL_Renderer*>(renderer.RawHandle()), point0Px.GetX(), point0Px.GetY(), point1Px.GetX(), point1Px.GetY())) {
		throw M2_ERROR(std::string{"SDL_RenderLine failed: "} + SDL_GetError());
	}
}
void video::DrawRectangle(Renderer& renderer, const RectF& rectLpx, const RGBA& color) {
	const auto pixelsPerUnit = renderer.GetPixelsPerWindowUnit();

	SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(renderer.RawHandle()), color.r, color.g, color.b, color.a);
	const auto sdlRect = ToSdlFRect(rectLpx.Scale(pixelsPerUnit));
	if (not SDL_RenderRect(static_cast<SDL_Renderer*>(renderer.RawHandle()), &sdlRect)) {
		throw M2_ERROR(std::string{"SDL_RenderRect failed: "} + SDL_GetError());
	}
}
void video::DrawRectangle(Renderer& renderer, const VecF& centerLpx, const float width, const float height, const float orientationRads, const RGBA& color) {
	const auto topLeft = centerLpx + VecF{-width / 2.0f, -height / 2.0f}.Rotate(orientationRads);
	const auto topRight = centerLpx + VecF{width / 2.0f, -height / 2.0f}.Rotate(orientationRads);
	const auto bottomLeft = centerLpx + VecF{-width / 2.0f, height / 2.0f}.Rotate(orientationRads);
	const auto bottomRight = centerLpx + VecF{width / 2.0f, height / 2.0f}.Rotate(orientationRads);
	DrawLine(renderer, topLeft, topRight, color);
	DrawLine(renderer, topRight, bottomRight, color);
	DrawLine(renderer, bottomRight, bottomLeft, color);
	DrawLine(renderer, bottomLeft, topLeft, color);
}
