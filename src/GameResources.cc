#define _CRT_SECURE_NO_WARNINGS
#include <m2/GameResources.h>

#include <sstream>
#include <m2/Options.h>

#if defined(__APPLE__)
#include <CoreFoundation/CFBundle.h>
#endif

std::filesystem::path m2::GetResourceDir() {
	if (not m2::gOverrideResourceDir.empty()) {
		return m2::gOverrideResourceDir;
	}
#if defined(__APPLE__)
	const auto main_bundle_handle = CFBundleGetMainBundle();
	const auto resources_directory_url_ref = CFBundleCopyResourcesDirectoryURL(main_bundle_handle);
	const auto resources_directory_absolute_url_ref = CFURLCopyAbsoluteURL(resources_directory_url_ref);
	const auto resources_directory_string_ref = CFURLCopyPath(resources_directory_absolute_url_ref);

	std::string resources_directory{CFStringGetCStringPtr(resources_directory_string_ref, kCFStringEncodingUTF8)};

	CFRelease(resources_directory_string_ref);
	CFRelease(resources_directory_absolute_url_ref);
	CFRelease(resources_directory_url_ref);

	return resources_directory;
#else
	// Assume that the resources are in the current directory
	return "resource";
#endif
}

m2::GameResources::GameResources(const std::string& gameIdentifier, const std::string& defaultFont) {
	// Engine Resources

	_resourceDir = GetResourceDir();
	_defaultResourceDir = _resourceDir / "default";
	_radialWhiteToBlackGradient = _resourceDir / "RadialGradient-WhiteBlack.png";

	// Game Resources

	_gameResourceDir = _resourceDir / "game" / gameIdentifier;
	_defaultFont = _resourceDir / defaultFont;
	_levelsDir = _gameResourceDir / "levels";

	_spriteSheets = _gameResourceDir / "SpriteSheets.json";
	if (not std::filesystem::exists(_spriteSheets)) {
		_spriteSheets = _defaultResourceDir / "SpriteSheets.json";
	}

	_cards = _gameResourceDir / "Cards.json";

	_animations = _gameResourceDir / "Animations.json";
	if (not std::filesystem::exists(_animations)) {
		_animations = _defaultResourceDir / "Animations.json";
	}

	_songs = _gameResourceDir / "Songs.json";
	if (not std::filesystem::exists(_songs)) {
		_songs = _defaultResourceDir / "Songs.json";
	}

	_keys = _gameResourceDir / "Keys.json";
	if (not std::filesystem::exists(_keys)) {
		_keys = _defaultResourceDir / "Keys.json";
	}
}
