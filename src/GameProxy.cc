#include <impl/public/ui/UI.h>
#include <impl/private/LevelBlueprint.h>
#include <m2/ui/UI.h>
#include "m2/GameProxy.hh"
#include "m2/Game.hh"
#include "SDL_image.h"

M2Err m2::game_proxy::activate() const {
	Game_UpdateWindowDimensions(GAME.windowRect.w, GAME.windowRect.h); // Uses tileSize for internal calculations
	if (textureMapFile) {
		SDL_Surface* textureMapSurface = IMG_Load(textureMapFile);
		if (textureMapSurface == NULL) {
			return LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, IMG_GetError());
		}
		if ((GAME.sdlTexture = SDL_CreateTextureFromSurface(GAME.sdlRenderer, textureMapSurface)) == NULL) {
			return LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, SDL_GetError());
		}
		SDL_SetTextureColorMod(GAME.sdlTexture, 127, 127, 127);
		SDL_FreeSurface(textureMapSurface);
	}
	if (textureMaskFile) {
		SDL_Surface* textureMaskSurface = IMG_Load(textureMaskFile);
		if (textureMaskSurface == NULL) {
			return LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, IMG_GetError());
		}
		if ((GAME.sdlTextureMask = SDL_CreateTextureFromSurface(GAME.sdlRenderer, textureMaskSurface)) == NULL) {
			return LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, SDL_GetError());
		}
		SDL_FreeSurface(textureMaskSurface);
	}
	return M2OK;
}

M2Err m2::game_proxy::exec_entry_ui() const {
    auto button = m2::ui::execute_blocking(&impl::ui::entry);
    if (button.index() == 0) {
        int return_value = std::get<0>(button);
        if (return_value == impl::ui::ENTRY_NEW_GAME) {
            return Game_Level_Load(&impl::level::sp_000);
        } else {
            return M2ERR_QUIT;
        }
    } else {
        return std::get<1>(button);
    }
}

M2Err m2::game_proxy::exec_pause_ui() const {
    auto button = m2::ui::execute_blocking(&impl::ui::pause);
    if (button.index() == 0) {
        int return_value = std::get<0>(button);
        if (return_value == impl::ui::PAUSE_RESUME_GAME) {
            return Game_Level_Load(&impl::level::sp_000);
        } else {
            return M2ERR_QUIT;
        }
    } else {
        return std::get<1>(button);
    }
}
