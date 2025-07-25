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

m2::GameDimensions::GameDimensions(void* renderer, const int gamePpm, const int gameAspectRatioMul, const int gameAspectRatioDiv)
		: _renderer(renderer), _gamePpm(gamePpm), _gameAspectRatioMul(gameAspectRatioMul), _gameAspectRatioDiv(gameAspectRatioDiv) {
	if (renderer == nullptr) {
		throw M2_ERROR("Given renderer is NULL");
	}
	if (gamePpm < 1) {
		throw M2_ERROR("Given PPM is not positive: " + m2::ToString(gamePpm));
	}
	if (gameAspectRatioMul < 1) {
		throw M2_ERROR("Given aspect ration multiplier is not positive: " + m2::ToString(gameAspectRatioMul));
	}
	if (gameAspectRatioDiv < 1) {
		throw M2_ERROR("Given aspect ration divider is not positive: " + m2::ToString(gameAspectRatioDiv));
	}

	// In the beginning, the scale is 1. We can fit as much as we can into the window using this scale.
	_scale = 1.0f;

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
	_rightHud = {windowDimensions.x - hudWidth, 0, hudWidth, hudHeight};

	_gameAndHudM = {F(_gameAndHud.w) / F(_gamePpm), F(_gameAndHud.h) / F(_gamePpm)};
	_gameM = {F(_game.w) / F(_gamePpm), F(_game.h) / F(_gamePpm)};
}

m2::VecI m2::GameDimensions::WindowDimensions() const {
	int w, h;
	SDL_GetRendererOutputSize(static_cast<SDL_Renderer*>(_renderer), &w, &h);
	return {w, h};
}
float m2::GameDimensions::OutputPixelsPerMeter() const {
	return F(_gamePpm) * _scale;
}
float m2::GameDimensions::GameWidthToGameAndHudWidthRatio() const {
	return F(_game.w) / F(_gameAndHud.w);
}
float m2::GameDimensions::HudWidthToGameAndHudWidthRatio() const {
	return F(_leftHud.w) / F(_gameAndHud.w);
}

void m2::GameDimensions::SetGameAspectRatio(const int gameAspectRatioMul, const int gameAspectRatioDiv) {
	_gameAspectRatioMul = gameAspectRatioMul;
	_gameAspectRatioDiv = gameAspectRatioDiv;
	OnWindowResize();
}
void m2::GameDimensions::OnWindowResize() {
	// We must keep the _gameAndHudM (and thus _gameM) exactly the same. The scale and the envelopes may be adjusted.

	const auto windowDimensions = WindowDimensions();
	// The unit is pixels, but in fractional form
	const auto minimumGameAndHudDimensions = VecF{_gameAndHudM.x * F(_gamePpm), _gameAndHudM.y * F(_gamePpm)};

	const auto idealWidthForSelectedWindowHeight = windowDimensions.y * GAME_AND_HUD_ASPECT_RATIO_MUL
		/ GAME_AND_HUD_ASPECT_RATIO_DIV;
	if (windowDimensions.x < idealWidthForSelectedWindowHeight) {
		// Screen is taller than expected, we'll definitely have top and bottom envelopes.
		// Ensure that only the 16:9 part of the window is taken into scale calculation
		const int realWindowWidth = windowDimensions.x;
		const int realWindowWidthRemainder = realWindowWidth % GAME_AND_HUD_ASPECT_RATIO_MUL;
		const int windowWidthForCorrectAspectRation = realWindowWidth - realWindowWidthRemainder;
		// Use the _gameAndHudM width for scale calculation
		_scale = F(windowWidthForCorrectAspectRation) / minimumGameAndHudDimensions.x;
	} else {
		// Screen is exact or wider than expected, we might have left and right envelopes.
		// Ensure that only the 16:9 part of the window is taken into scale calculation
		const int realWindowHeight = windowDimensions.y;
		const int realWindowHeightRemainder = realWindowHeight % GAME_AND_HUD_ASPECT_RATIO_DIV;
		const int windowHeightForCorrectAspectRation = realWindowHeight - realWindowHeightRemainder;
		// Use the _gameAndHudM height for scale calculation
		_scale = F(windowHeightForCorrectAspectRation) / minimumGameAndHudDimensions.y;
	}

	const int gameHeight = RoundI(minimumGameAndHudDimensions.y * _scale);
	const int gameWidth = gameHeight * _gameAspectRatioMul / _gameAspectRatioDiv;
	const int hudHeight = gameHeight;
	const int hudWidth = gameHeight * HudAspectRatioMul(_gameAspectRatioMul, _gameAspectRatioDiv)
			/ HudAspectRatioDiv(_gameAspectRatioDiv);

	const int topEnvelopeSize = (windowDimensions.y - gameHeight) / 2;
	const int bottomEnvelopeSize = windowDimensions.y - gameHeight - topEnvelopeSize;
	const int leftEnvelopeSize = (windowDimensions.x - gameWidth - 2 * hudWidth) / 2;
	const int rightEnvelopeSize = windowDimensions.x - gameWidth - 2 * hudWidth - leftEnvelopeSize;

	_topEnvelope = RectI{0, 0, windowDimensions.x, topEnvelopeSize};
	_bottomEnvelope = RectI{0, windowDimensions.y - bottomEnvelopeSize, windowDimensions.x, bottomEnvelopeSize};
	_leftEnvelope = RectI{0, topEnvelopeSize, leftEnvelopeSize, gameHeight};
	_rightEnvelope = RectI{windowDimensions.x - rightEnvelopeSize, topEnvelopeSize, rightEnvelopeSize, gameHeight};

	_gameAndHud = RectI{leftEnvelopeSize, topEnvelopeSize, windowDimensions.x - leftEnvelopeSize - rightEnvelopeSize, gameHeight};
	_leftHud = RectI{leftEnvelopeSize, topEnvelopeSize, hudWidth, hudHeight};
	_rightHud = RectI{windowDimensions.x - rightEnvelopeSize - hudWidth, topEnvelopeSize, hudWidth, hudHeight};
	_game = RectI{leftEnvelopeSize + hudWidth, topEnvelopeSize, gameWidth, gameHeight};
}
void m2::GameDimensions::SetScale(const float scale) {
	if (scale <= 0.0f) {
		throw M2_ERROR("Given scale is invalid: " + m2::ToString(scale));
	}
	LOG_INFO("Setting scale to", scale);

	_scale = scale;
	ReadjustAfterScaleChange();
}
void m2::GameDimensions::SetGameHeightM(const float heightM) {
	// GameHeightM determines how much of the word is shown in the game window.
	// It's notmally calculated with the formula: GameHeightM = GameHeightPx / PPM
	// We can't change GameHeightPx, because the window isn't resized, thus we must calculate a new PPM instead.
	// We also don't prefer to change the PPM directly, instead we change the scale: PPM = GamePpm * Scale
	// Thus: GameHeightM = GameHeightPx / (GamePpm * Scale)
	// Thus: Scale = GameHeightPx / (GamePpm * GameHeightM)
	SetScale(F(_game.h) / (F(_gamePpm) * heightM));
}

m2::VecI m2::GameDimensions::EstimateMinimumWindowDimensions(const int gamePpm, const float gameHeightM) {
	// We expect minimum window height to be integer multiple of GAME_AND_HUD_ASPECT_RATIO_DIV
	const int requestedWindowHeight = RoundI(gameHeightM * F(gamePpm));
	const int requestedWindowHeightRemainder = requestedWindowHeight % GAME_AND_HUD_ASPECT_RATIO_DIV;
	const int calculatedWindowHeight = requestedWindowHeight - requestedWindowHeightRemainder;

	// Calculate width based on calculated height
	const int calculatedWindowWidth = calculatedWindowHeight * GAME_AND_HUD_ASPECT_RATIO_MUL / GAME_AND_HUD_ASPECT_RATIO_DIV;

	return {calculatedWindowWidth, calculatedWindowHeight};
}
int m2::GameDimensions::NextPixelatedScale(int currentScale) {
	if (currentScale == 0) {
		throw M2_ERROR("Unexpected scale: " + m2::ToString(currentScale));
	}
	if (currentScale == -1) {
		return 2;
	}
	return currentScale + 1;
}
int m2::GameDimensions::PrevPixelatedScale(int currentScale) {
	if (currentScale == 0) {
		throw M2_ERROR("Unexpected scale: " + m2::ToString(currentScale));
	}
	if (currentScale == 1) {
		return -2;
	}
	return currentScale - 1;
}

void m2::GameDimensions::ReadjustAfterScaleChange() {
	// Scale adjustment is used to adjust the zoom of the game. This means, we must keep envelopes, gameAndHud, game,
	// leftHud, rightHud, messageBox exactly the same. The portion of the game shown inside the game area, thus
	// gameAndHudM and gameM, can be adjusted instead.
	_gameAndHudM = {F(_gameAndHud.w) / OutputPixelsPerMeter(), F(_gameAndHud.h) / OutputPixelsPerMeter()};
	_gameM = {F(_game.w) / OutputPixelsPerMeter(), F(_game.h) / OutputPixelsPerMeter()};
}
