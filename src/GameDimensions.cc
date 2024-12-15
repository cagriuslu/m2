#include <m2/GameDimensions.h>
#include <m2/Meta.h>
#include <m2/Game.h>
#include <m2/Log.h>

constexpr int GAME_AND_HUD_ASPECT_RATIO_MUL = 16;
constexpr int GAME_AND_HUD_ASPECT_RATIO_DIV = 9;

namespace {
	constexpr int HudAspectRatioMul(const int gameAspectRatioMul, const int gameAspectRatioDiv) {
		return GAME_AND_HUD_ASPECT_RATIO_MUL * gameAspectRatioDiv - gameAspectRatioMul * GAME_AND_HUD_ASPECT_RATIO_DIV;
	}
	constexpr int HudAspectRatioDiv(const int gameAspectRatioDiv) {
		return GAME_AND_HUD_ASPECT_RATIO_DIV * gameAspectRatioDiv * 2;
	}
}

m2::GameDimensionsManager::GameDimensionsManager(void* renderer, const int gamePpm, const int gameAspectRatioMul, const int gameAspectRatioDiv, bool areGraphicsPixelated)
		: _renderer(renderer), _gamePpm(gamePpm), _gameAspectRatioMul(gameAspectRatioMul), _gameAspectRatioDiv(gameAspectRatioDiv), _isPixelated(areGraphicsPixelated) {
	if (renderer == nullptr) {
		throw M2_ERROR("Given renderer is NULL");
	}
	if (gamePpm < 1) {
		throw M2_ERROR("Given PPM is not positive: " + std::to_string(gamePpm));
	}
	if (gameAspectRatioMul < 1) {
		throw M2_ERROR("Given aspect ration multiplier is not positive: " + std::to_string(gameAspectRatioMul));
	}
	if (gameAspectRatioDiv < 1) {
		throw M2_ERROR("Given aspect ration divider is not positive: " + std::to_string(gameAspectRatioDiv));
	}

	// In the beginning, the scale is 1. We can fit as much as we can into the window using this scale.
	if (_isPixelated) {
		_scale = 1;
	} else {
		_scale = 1.0f;
	}

	_topEnvelope = _bottomEnvelope = _leftEnvelope = _rightEnvelope = {};

	const auto windowDimensions = WindowDimensions();
	_gameAndHud = RectI{0, 0, windowDimensions.x, windowDimensions.y};

	const int gameHeight = _gameAndHud.h;
	const int gameWidth = gameHeight * _gameAspectRatioMul / _gameAspectRatioDiv;
	const int hudHeight = gameHeight;
	const int hudWidth = hudHeight * HudAspectRatioMul(_gameAspectRatioMul, _gameAspectRatioDiv)
			/ HudAspectRatioDiv(_gameAspectRatioDiv);

	_game = {hudWidth, 0, gameWidth, gameHeight};
	_leftHud = {0, 0, hudWidth, hudHeight};
	_rightHud = {hudWidth + gameWidth, 0, hudWidth, hudHeight};

	const auto messageBoxHeight = _game.h / 25;
	_messageBox = {_game.x, _game.y + _game.h - messageBoxHeight, _game.w, messageBoxHeight};

	_gameAndHudM = {F(_gameAndHud.w) / F(_gamePpm), F(_gameAndHud.h) / F(_gamePpm)};
	_gameM = {F(_game.w) / F(_gamePpm), F(_game.h) / F(_gamePpm)};
}

m2::VecI m2::GameDimensionsManager::WindowDimensions() const {
	int w, h;
	SDL_GetRendererOutputSize(static_cast<SDL_Renderer*>(_renderer), &w, &h);
	return {w, h};
}
int m2::GameDimensionsManager::PixelatedOutputPixelsPerMeter() const {
	if (not _isPixelated) {
		throw M2_ERROR("Pixelated output PPM is queried for non-pixelated game");
	}
	if (0 < std::get<int>(_scale)) {
		return _gamePpm * std::get<int>(_scale);
	}
	return iround(F(_gamePpm) / F(-std::get<int>(_scale)));
}
float m2::GameDimensionsManager::RealOutputPixelsPerMeter() const {
	if (_isPixelated) {
		if (0 < std::get<int>(_scale)) {
			return F(_gamePpm * std::get<int>(_scale));
		}
		return F(_gamePpm / -std::get<int>(_scale));
	}
	return F(_gamePpm) * std::get<float>(_scale);
}
int m2::GameDimensionsManager::PixelatedScale() const {
	if (not _isPixelated) {
		throw M2_ERROR("Pixelated scale is queried for non-pixelated game");
	}
	return std::get<int>(_scale);
}
float m2::GameDimensionsManager::RealScale() const {
	if (_isPixelated) {
		throw M2_ERROR("Real scale is queried for pixelated game");
	}
	return std::get<float>(_scale);
}
float m2::GameDimensionsManager::GameWidthToGameAndHudWidthRatio() const {
	return F(_game.w) / F(_gameAndHud.w);
}
float m2::GameDimensionsManager::HudWidthToGameAndHudWidthRatio() const {
	return F(_leftHud.w) / F(_gameAndHud.w);
}

void m2::GameDimensionsManager::OnWindowResize() {
	// We must keep the _gameAndHudM (and thus _gameM) exactly the same. The scale and the envelopes may be adjusted.

	const auto windowDimensions = WindowDimensions();
	const auto minimumGameAndHudDimensions = VecF{_gameAndHudM.x * F(_gamePpm), _gameAndHudM.y * F(_gamePpm)};

	float provisionalScale;
	const auto idealWidthForSelectedWindowHeight = windowDimensions.y * GAME_AND_HUD_ASPECT_RATIO_MUL
		/ GAME_AND_HUD_ASPECT_RATIO_DIV;
	if (windowDimensions.x < idealWidthForSelectedWindowHeight) {
		// Screen is taller than expected, we'll definitely have top and bottom envelopes.
		// Ensure that only the 16:9 part of the window is taken into scale calculation
		const int realWindowWidth = windowDimensions.x;
		const int realWindowWidthRemainder = realWindowWidth % GAME_AND_HUD_ASPECT_RATIO_MUL;
		const int windowWidthForCorrectAspectRation = realWindowWidth - realWindowWidthRemainder;
		// Use the _gameAndHudM width for scale calculation
		provisionalScale = F(windowWidthForCorrectAspectRation) / minimumGameAndHudDimensions.x;
	} else {
		// Screen is exact or wider than expected, we might have left and right envelopes.
		// Ensure that only the 16:9 part of the window is taken into scale calculation
		const int realWindowHeight = windowDimensions.y;
		const int realWindowHeightRemainder = realWindowHeight % GAME_AND_HUD_ASPECT_RATIO_DIV;
		const int windowHeightForCorrectAspectRation = realWindowHeight - realWindowHeightRemainder;
		// Use the _gameAndHudM height for scale calculation
		provisionalScale = F(windowHeightForCorrectAspectRation) / minimumGameAndHudDimensions.y;
	}

	int gameHeight;
	if (_isPixelated) {
		const auto flooredScale = floorf(provisionalScale);
		_scale = static_cast<int>(flooredScale);
		gameHeight = iround(minimumGameAndHudDimensions.y * flooredScale);
	} else {
		_scale = provisionalScale;
		gameHeight = iround(minimumGameAndHudDimensions.y * provisionalScale);
	}
	const int gameWidth = gameHeight * _gameAspectRatioMul / _gameAspectRatioDiv;
	const int hudHeight = gameHeight;
	const int hudWidth = gameHeight * HudAspectRatioMul(_gameAspectRatioMul, _gameAspectRatioDiv)
			/ HudAspectRatioDiv(_gameAspectRatioDiv);

	const int topEnvelopeSize = (windowDimensions.y - gameHeight) / 2;
	const int bottomEnvelopeSize = windowDimensions.y - gameHeight - topEnvelopeSize;
	const int leftEnvelopeSize = (windowDimensions.x - gameWidth - 2 * hudWidth) / 2;
	const int rightEnvelopeSize = windowDimensions.x - gameWidth - 2 * hudWidth - leftEnvelopeSize;

	_topEnvelope = RectI{0, 0, windowDimensions.x, topEnvelopeSize};
	_bottomEnvelope = RectI{0, topEnvelopeSize + gameHeight, windowDimensions.x, bottomEnvelopeSize};
	_leftEnvelope = RectI{0, topEnvelopeSize, leftEnvelopeSize, gameHeight};
	_rightEnvelope = RectI{leftEnvelopeSize + 2 * hudWidth + gameWidth, topEnvelopeSize, rightEnvelopeSize, gameHeight};

	_gameAndHud = RectI{leftEnvelopeSize, topEnvelopeSize, 2 * hudWidth + gameWidth, gameHeight};
	_leftHud = RectI{leftEnvelopeSize, topEnvelopeSize, hudWidth, hudHeight};
	_rightHud = RectI{leftEnvelopeSize + hudWidth + gameWidth, topEnvelopeSize, hudWidth, hudHeight};
	_game = RectI{leftEnvelopeSize + hudWidth, topEnvelopeSize, gameWidth, gameHeight};

	const auto messageBoxHeight = _game.h / 25;
	_messageBox = RectI{_game.x, _game.y + _game.h - messageBoxHeight, _game.w, messageBoxHeight};
}
void m2::GameDimensionsManager::SetScale(float scale) {
	if (_isPixelated) {
		throw M2_ERROR("Rational scaling is used for a pixelated game");
	}
	if (scale <= 0.0f) {
		throw M2_ERROR("Given scale is invalid: " + std::to_string(scale));
	}
	LOG_INFO("Setting non-pixelated scale to", scale);

	_scale = scale;
	ReadjustAfterScaleChange();
}
void m2::GameDimensionsManager::SetScale(int scale) {
	if (not _isPixelated) {
		throw M2_ERROR("Integer scaling is used for a non-pixelated game");
	}
	if (scale == 0) {
		throw M2_ERROR("Given scale is invalid: 0");
	}
	LOG_INFO("Setting pixelated scale to", scale);

	_scale = scale;
	ReadjustAfterScaleChange();
}

m2::VecI m2::GameDimensionsManager::EstimateMinimumWindowDimensions(const int gamePpm, const float gameHeightM) {
	// We expect minimum window height to be integer multiple of GAME_AND_HUD_ASPECT_RATIO_DIV
	const int requestedWindowHeight = iround(gameHeightM * F(gamePpm));
	const int requestedWindowHeightRemainder = requestedWindowHeight % GAME_AND_HUD_ASPECT_RATIO_DIV;
	const int calculatedWindowHeight = requestedWindowHeight - requestedWindowHeightRemainder;

	// Calculate width based on calculated height
	const int calculatedWindowWidth = calculatedWindowHeight * GAME_AND_HUD_ASPECT_RATIO_MUL / GAME_AND_HUD_ASPECT_RATIO_DIV;

	return {calculatedWindowWidth, calculatedWindowHeight};
}
int m2::GameDimensionsManager::NextPixelatedScale(int currentScale) {
	if (currentScale == 0) {
		throw M2_ERROR("Unexpected scale: " + std::to_string(currentScale));
	}
	if (currentScale == -1) {
		return 2;
	}
	return currentScale + 1;
}
int m2::GameDimensionsManager::PrevPixelatedScale(int currentScale) {
	if (currentScale == 0) {
		throw M2_ERROR("Unexpected scale: " + std::to_string(currentScale));
	}
	if (currentScale == 1) {
		return -2;
	}
	return currentScale - 1;
}

void m2::GameDimensionsManager::ReadjustAfterScaleChange() {
	// Scale adjustment is used to adjust the zoom of the game. This means, we must keep envelopes, gameAndHud, game,
	// leftHud, rightHud, messageBox exactly the same. The portion of the game shown inside the game area, thus
	// gameAndHudM and gameM, can be adjusted instead.
	_gameAndHudM = {F(_gameAndHud.w) / RealOutputPixelsPerMeter(), F(_gameAndHud.h) / RealOutputPixelsPerMeter()};
	_gameM = {F(_game.w) / RealOutputPixelsPerMeter(), F(_game.h) / RealOutputPixelsPerMeter()};
}
