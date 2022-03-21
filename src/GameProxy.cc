#include "m2/GameProxy.hh"
#include "m2/UI.hh"
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
	if (cfgSprites) {
		for (CfgSpriteIndex i = 0; i < cfgSpriteCount; i++) {
			M2ASSERT(gp->cfgSprites[i].index == i);
		}
	}
	return M2OK;
}

M2Err m2::game_proxy::exec_entry_ui() const {
	CfgUIButtonType button;
	M2Err result = UI_ExecuteBlocking(entryUi, &button);
	if (result) {
		LOG_ERROR_M2(result);
		return result;
	}
	M2Err handlerResult = entryUiButtonHandler(button);
	if (handlerResult == M2ERR_QUIT) {
		LOG_INFO_M2(handlerResult);
	} else if (handlerResult) {
		LOG_ERROR_M2(handlerResult);
	}
	return handlerResult;
}

M2Err m2::game_proxy::exec_pause_ui() const {
	if (pauseUi) {
		CfgUIButtonType button;
		M2Err result = UI_ExecuteBlocking(pauseUi, &button);
		if (result) {
			LOG_ERROR_M2(result);
			return result;
		}
		if (pauseUiButtonHandler) {
			M2Err handlerResult = pauseUiButtonHandler(button);
			if (handlerResult == M2ERR_QUIT) {
				LOG_INFO_M2(handlerResult);
			} else if (handlerResult) {
				LOG_ERROR_M2(handlerResult);
			}
			return handlerResult;
		}
	}
	return M2OK;
}
