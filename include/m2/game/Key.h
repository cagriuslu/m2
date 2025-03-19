#pragma once
#include <m2/Meta.h>
#include <m2g_KeyType.pb.h>
#include <SDL2/SDL_scancode.h>
#include <map>
#include <filesystem>

namespace m2 {
	std::multimap<m2g::pb::KeyType, SDL_Scancode> GenerateKeyToScancodeMap(const std::filesystem::path& keysPath);
	std::map<SDL_Scancode, m2g::pb::KeyType> GenerateScancodeToKeyMap(const std::filesystem::path& keysPath);
}
