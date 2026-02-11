#pragma once
#include <filesystem>

namespace m2 {
	/// Returns the root resource directory without depending on Game.
	std::filesystem::path GetResourceDir();

	/// Class grouping various resource paths
	class GameResources final {
		// Engine Resources

		std::filesystem::path _resourceDir, _defaultResourceDir;
		std::filesystem::path _radialWhiteToBlackGradient;

		// Game Resources

		std::filesystem::path _gameResourceDir;
		std::filesystem::path _defaultFont, _systemFont, _levelsDir, _spriteSheets, _cards, _animations, _songs, _keys;

	public:
		explicit GameResources(const std::string& gameIdentifier, const std::string& defaultFont);

		// Engine Resources

		[[nodiscard]] const std::filesystem::path& GetRadialWhiteToBlackGradientPath() const { return _radialWhiteToBlackGradient; }

		// Game Resources

		[[nodiscard]] const std::filesystem::path& GetGameResourceDir() const { return _gameResourceDir; }
		[[nodiscard]] const std::filesystem::path& GetDefaultFontPath() const { return _defaultFont; }
		[[nodiscard]] const std::filesystem::path& GetSystemFontPath() const { return _systemFont; }
		[[nodiscard]] const std::filesystem::path& GetLevelsDir() const { return _levelsDir; }
		[[nodiscard]] const std::filesystem::path& GetSpriteSheetsPath() const { return _spriteSheets; }
		[[nodiscard]] const std::filesystem::path& GetCardsPath() const { return _cards; }
		[[nodiscard]] const std::filesystem::path& GetAnimationsPath() const { return _animations; }
		[[nodiscard]] const std::filesystem::path& GetSongsPath() const { return _songs; }
		[[nodiscard]] const std::filesystem::path& GetKeysPath() const { return _keys; }
	};
}
