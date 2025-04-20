#include <m2/sheet_editor/PersistentSpriteSheets.h>

using namespace m2;
using namespace m2::sheet_editor;

expected<PersistentSpriteSheets> PersistentSpriteSheets::LoadFile(std::filesystem::path path) {
	auto po = PersistentObject::LoadFile(std::move(path));
	if (not po) {
		return make_unexpected(std::move(po.error()));
	}
	return PersistentSpriteSheets(std::move(*po));
}

std::vector<m2g::pb::SpriteType> PersistentSpriteSheets::AllSpriteTypes() const {
	std::vector<m2g::pb::SpriteType> sprite_types;
	std::ranges::for_each(Cache().sheets(), [&sprite_types](const auto& sheet) {
		std::for_each(sheet.sprites().cbegin(), sheet.sprites().cend(), [&sprite_types](const auto& sprite) {
			sprite_types.emplace_back(sprite.type());
		});
	});
	return sprite_types;
}
const pb::SpriteSheet* PersistentSpriteSheets::SpriteSheetPbWithSprite(m2g::pb::SpriteType spriteType) const {
	for (const auto& sheet : Cache().sheets()) {
		for (const auto& sprite : sheet.sprites()) {
			if (sprite.type() == spriteType) {
				return &sheet;
			}
		}
	}
	return nullptr;
}
const pb::Sprite& PersistentSpriteSheets::SpritePb(const m2g::pb::SpriteType spriteType) const {
	for (auto& sheet : Cache().sheets()) {
		for (auto& sprite : sheet.sprites()) {
			if (sprite.type() == spriteType) {
				return sprite;
			}
		}
	}
	throw M2_ERROR("Unable to find sprite: " + ToString(spriteType));
}
std::vector<pb::Fixture::FixtureTypeCase> PersistentSpriteSheets::SpriteFixtureTypes(const m2g::pb::SpriteType spriteType) const {
	std::vector<pb::Fixture::FixtureTypeCase> fixtureTypes;
	for (const auto& fixture : SpritePb(spriteType).regular().fixtures()) {
		fixtureTypes.emplace_back(fixture.fixture_type_case());
	}
	return fixtureTypes;
}

void PersistentSpriteSheets::ModifySprite(const m2g::pb::SpriteType spriteType, const std::function<void(pb::Sprite&)>& modifier) {
	auto expectSuccess = Mutate([&](pb::SpriteSheets& sheets) {
		for (auto& sheet : *sheets.mutable_sheets()) {
			for (auto& sprite : *sheet.mutable_sprites()) {
				if (sprite.type() == spriteType) {
					modifier(sprite);
					return;
				}
			}
		}
	});
	if (not expectSuccess) {
		throw M2_ERROR("Unable to mutate sprite sheets: " + expectSuccess.error());
	}
}
