#include <m2/thirdparty/video/Shapes.h>
#include <m2/Game.h>
#include <m2/Error.h>
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
