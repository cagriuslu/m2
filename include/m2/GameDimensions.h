#pragma once
#include <m2/math/RectI.h>
#include <m2/math/VecF.h>
#include <m2/math/VecI.h>
#include <m2/Meta.h>

namespace m2 {
    class GameDimensionsManager final {
        void* const _renderer;
        const int _gamePpm, _gameAspectRatioMul, _gameAspectRatioDiv;
        const bool _isPixelated;

        // The value of the scale is interpreted differently based on whether the game is pixelated or not. If the game
        // is pixelated, the scale is a non-zero integer where negative values mean division. Otherwise, the scale is a
        // positive scaling factor.
        std::variant<int, float> _scale;
        RectI _topEnvelope, _bottomEnvelope, _leftEnvelope, _rightEnvelope;
        RectI _gameAndHud, _game;
        RectI _leftHud, _rightHud, _messageBox;
        VecF _gameAndHudM, _gameM;

    public:
        GameDimensionsManager(void* renderer, int gamePpm, int gameAspectRatioMul, int gameAspectRatioDiv, bool areGraphicsPixelated);

        // Accessors

        [[nodiscard]] VecI WindowDimensions() const;
        [[nodiscard]] RectI TopEnvelope() const { return _topEnvelope; }
        [[nodiscard]] RectI BottomEnvelope() const { return _bottomEnvelope; }
        [[nodiscard]] RectI LeftEnvelope() const { return _leftEnvelope; }
        [[nodiscard]] RectI RightEnvelope() const { return _rightEnvelope; }
        [[nodiscard]] RectI GameAndHud() const { return _gameAndHud; }
        [[nodiscard]] RectI Game() const { return _game; }
        [[nodiscard]] RectI LeftHud() const { return _leftHud; }
        [[nodiscard]] RectI RightHud() const { return _rightHud; }
        [[nodiscard]] RectI MessageBox() const { return _messageBox; }
        [[nodiscard]] VecF GameAndHudM() const { return _gameAndHudM; }
        [[nodiscard]] VecF GameM() const { return _gameM; }
        /// Number of pixels in the renderer output that would be occupied by a one-meter object. Only applicable to pixelated games.
        [[nodiscard]] int PixelatedOutputPixelsPerMeter() const;
        /// Number of pixels in the renderer output that would be occupied by a one-meter object. Applicable to pixelated and non-pixelated games.
        [[nodiscard]] float RealOutputPixelsPerMeter() const;
        [[nodiscard]] int PixelatedScale() const;
        [[nodiscard]] float RealScale() const;
        [[nodiscard]] float GameWidthToGameAndHudWidthRatio() const;
        [[nodiscard]] float HudWidthToGameAndHudWidthRatio() const;

        // Modifiers

        /// Re-evaluates the logical area dimensions and scale after a window resize.
        void OnWindowResize();
        /// Adjusts the zoom of the game. This function should be used for non-pixelated games.
        void SetScale(float scale);
        /// Adjusts the zoom of the game. This functions should be used for pixelated games. Positive values multiply
        /// the size of the sprites on the screen, whereas negative values divide it. Zero is not allowed.
        void SetScale(int scale);

        // Helpers

        /// Calculates the minimum window dimensions for given PPM and game height. A lower height may be picked to ensure a 16:9 window.
        static VecI EstimateMinimumWindowDimensions(int gamePpm, float gameHeightM);
        /// Calculates the next pixelated scale based on the given scale. Pixelated games can use this helper to zoom in.
        static int NextPixelatedScale(int currentScale);
        /// Calculates the previous pixelated scale based on the given scale. Pixelated games can use this helper to zoom out.
        static int PrevPixelatedScale(int currentScale);
    private:
        void ReadjustAfterScaleChange();
    };
}
