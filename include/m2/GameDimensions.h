#pragma once
#include <m2/math/RectI.h>
#include <m2/math/VecF.h>
#include <m2/math/VecI.h>

namespace m2 {
    class GameDimensions final {
        void* const _renderer;
        int _gamePpm, _gameAspectRatioMul, _gameAspectRatioDiv;

        float _scale;
        RectI _topEnvelope, _bottomEnvelope, _leftEnvelope, _rightEnvelope;
        RectI _gameAndHud, _game;
        RectI _leftHud, _rightHud;
        VecF _gameAndHudM, _gameM;

    public:
        /// This constructor assumes the initial aspect ratio of the window is exact, thus requires no envelope.
        GameDimensions(void* renderer, int gamePpm, int gameAspectRatioMul, int gameAspectRatioDiv);

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
        [[nodiscard]] VecF GameAndHudM() const { return _gameAndHudM; }
        [[nodiscard]] VecF GameM() const { return _gameM; }
        /// Number of pixels in the renderer output that would be occupied by a one-meter object. Applicable to pixelated and non-pixelated games.
        [[nodiscard]] float OutputPixelsPerMeter() const;
        [[nodiscard]] float Scale() const { return _scale; }
        [[nodiscard]] float GameWidthToGameAndHudWidthRatio() const;
        [[nodiscard]] float HudWidthToGameAndHudWidthRatio() const;

        // Modifiers

        /// Changes the game aspect ratio. This function will call OnWindowResize internally.
        void SetGameAspectRatio(int gameAspectRatioMul, int gameAspectRatioDiv);
        /// Re-evaluates the logical area dimensions and scale after a window resize.
        void OnWindowResize();
        void SetScale(float scale);
        void SetGameHeightM(float heightM);

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
