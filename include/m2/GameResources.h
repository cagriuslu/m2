#pragma once
#include <filesystem>

namespace m2 {
	/// Root resource directory can be queried before the Game is constructed
	std::filesystem::path GetResourceDir();

	class GameResources final {
		// Engine Resources

		std::filesystem::path _resourceDir, _defaultResourceDir;
		std::filesystem::path _radialWhiteToBlackGradient;

		// Game Resources

		std::filesystem::path _gameResourceDir;
		std::filesystem::path _defaultFont, _levelsDir, _spriteSheets, _items, _animations, _songs, _keys;

	public:
		explicit GameResources(const std::string& gameIdentifier, const std::string& defaultFont);

		// Engine Resources

		[[nodiscard]] const std::filesystem::path& GetRadialWhiteToBlackGradientPath() const { return _radialWhiteToBlackGradient; }

		// Game Resources

		[[nodiscard]] const std::filesystem::path& GetGameResourceDir() const { return _gameResourceDir; }
		[[nodiscard]] const std::filesystem::path& GetDefaultFontPath() const { return _defaultFont; }
		[[nodiscard]] const std::filesystem::path& GetLevelsDir() const { return _levelsDir; }
		[[nodiscard]] const std::filesystem::path& GetSpriteSheetsPath() const { return _spriteSheets; }
		[[nodiscard]] const std::filesystem::path& GetItemsPath() const { return _items; }
		[[nodiscard]] const std::filesystem::path& GetAnimationsPath() const { return _animations; }
		[[nodiscard]] const std::filesystem::path& GetSongsPath() const { return _songs; }
		[[nodiscard]] const std::filesystem::path& GetKeysPath() const { return _keys; }
	};
}
