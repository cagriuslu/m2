#include <m2/game/Key.h>
#include <m2/protobuf/LUT.h>

std::multimap<m2g::pb::KeyType, SDL_Scancode> m2::GenerateKeyToScancodeMap(const std::filesystem::path& keysPath) {
	const auto keys = pb::LUT<pb::Key>::LoadProtoItems(keysPath, &pb::Keys::keys);
	if (not keys) {
		throw M2_ERROR(keys.error());
	}

	std::multimap<m2g::pb::KeyType, SDL_Scancode> map;
	for (const auto& key : *keys) {
		for (const auto& scancode : key.sdl_scancodes()) {
			map.emplace(key.type(), static_cast<SDL_Scancode>(scancode));
		}
	}
	return map;
}

std::map<SDL_Scancode, m2g::pb::KeyType> m2::GenerateScancodeToKeyMap(const std::filesystem::path& keysPath) {
	const auto keys = pb::LUT<pb::Key>::LoadProtoItems(keysPath, &pb::Keys::keys);
	if (not keys) {
		throw M2_ERROR(keys.error());
	}

	std::map<SDL_Scancode, m2g::pb::KeyType> map;
	for (const auto& key : *keys) {
		for (const auto& scancode : key.sdl_scancodes()) {
			if (auto [_, inserted] = map.emplace(static_cast<SDL_Scancode>(scancode), key.type()); not inserted) {
				throw M2_ERROR("Multiple keys are mapped to the same scancode");
			}
		}
	}
	return map;
}
