#include "m2/GameProxy.hh"
#include "m2/UI.hh"
#include "m2/Game.hh"
#include <SDL_image.h>

M2Err GameProxy_Init(GameProxy *gp) {
	memset(gp, 0, sizeof(GameProxy));
	return M2OK;
}

M2Err GameProxy_Activate(GameProxy *gp) {
	Game_UpdateWindowDimensions(GAME->windowRect.w, GAME->windowRect.h); // Uses tileSize for internal calculations
	if (gp->textureMapFile) {
		SDL_Surface* textureMapSurface = IMG_Load(gp->textureMapFile);
		if (textureMapSurface == NULL) {
			return LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, IMG_GetError());
		}
		if ((GAME->sdlTexture = SDL_CreateTextureFromSurface(GAME->sdlRenderer, textureMapSurface)) == NULL) {
			return LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, SDL_GetError());
		}
		SDL_SetTextureColorMod(GAME->sdlTexture, 127, 127, 127);
		SDL_FreeSurface(textureMapSurface);
	}
	if (gp->textureMaskFile) {
		SDL_Surface* textureMaskSurface = IMG_Load(gp->textureMaskFile);
		if (textureMaskSurface == NULL) {
			return LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, IMG_GetError());
		}
		if ((GAME->sdlTextureMask = SDL_CreateTextureFromSurface(GAME->sdlRenderer, textureMaskSurface)) == NULL) {
			return LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, SDL_GetError());
		}
		SDL_FreeSurface(textureMaskSurface);
	}
	if (gp->cfgSprites) {
		for (CfgSpriteIndex i = 0; i < gp->cfgSpriteCount; i++) {
			M2ASSERT(gp->cfgSprites[i].index == i);
		}
	}
	return M2OK;
}

M2Err GameProxy_ExecuteEntryUI(GameProxy *gp) {
	CfgUIButtonType button;
	M2Err result = UI_ExecuteBlocking(gp->entryUi, &button);
	if (result) {
		LOG_ERROR_M2(result);
		return result;
	}
	M2Err handlerResult = gp->entryUiButtonHandler(button);
	if (handlerResult == M2ERR_QUIT || handlerResult == M2ERR_PROXY_CHANGED) {
		LOG_INFO_M2(handlerResult);
	} else if (handlerResult) {
		LOG_ERROR_M2(handlerResult);
	}
	return handlerResult;
}

M2Err GameProxy_ExecutePauseUI(GameProxy *gp) {
	if (gp->pauseUi) {
		CfgUIButtonType button;
		M2Err result = UI_ExecuteBlocking(gp->pauseUi, &button);
		if (result) {
			LOG_ERROR_M2(result);
			return result;
		}
		if (gp->pauseUiButtonHandler) {
			M2Err handlerResult = gp->pauseUiButtonHandler(button);
			if (handlerResult == M2ERR_QUIT || handlerResult == M2ERR_PROXY_CHANGED) {
				LOG_INFO_M2(handlerResult);
			} else if (handlerResult) {
				LOG_ERROR_M2(handlerResult);
			}
			return handlerResult;
		}
	}
	return M2OK;
}

void GameProxy_Term(GameProxy *gp) {
	if (gp->destructor) {
		gp->destructor(gp);
	}
	memset(gp, 0, sizeof(GameProxy));
}
