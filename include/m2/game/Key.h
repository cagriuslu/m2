#pragma once
#include <m2/common/Meta.h>
#include <m2g_KeyType.pb.h>
#include <m2/thirdparty/event/Event.h>
#include <map>
#include <filesystem>

namespace m2 {
	std::multimap<m2g::pb::KeyType, thirdparty::event::Scancode> GenerateKeyToScancodeMap(const std::filesystem::path& keysPath);
	std::map<thirdparty::event::Scancode, m2g::pb::KeyType> GenerateScancodeToKeyMap(const std::filesystem::path& keysPath);
}
