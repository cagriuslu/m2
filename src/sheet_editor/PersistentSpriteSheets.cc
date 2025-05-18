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
	std::ranges::for_each(GetCache().sheets(), [&sprite_types](const auto& sheet) {
		std::for_each(sheet.sprites().cbegin(), sheet.sprites().cend(), [&sprite_types](const auto& sprite) {
			sprite_types.emplace_back(sprite.type());
		});
	});
	return sprite_types;
}
const pb::SpriteSheet* PersistentSpriteSheets::SpriteSheetPbWithSprite(m2g::pb::SpriteType spriteType) const {
	for (const auto& sheet : GetCache().sheets()) {
		for (const auto& sprite : sheet.sprites()) {
			if (sprite.type() == spriteType) {
				return &sheet;
			}
		}
	}
	return nullptr;
}
const pb::Sprite& PersistentSpriteSheets::SpritePb(const m2g::pb::SpriteType spriteType) const {
	for (auto& sheet : GetCache().sheets()) {
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
	auto expectSuccess = MutateAndSave([&](pb::SpriteSheets& sheets) {
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
int PersistentSpriteSheets::AddFixtureToSprite(const m2g::pb::SpriteType spriteType, const pb::Fixture::FixtureTypeCase type, const int insertIndex) {
	int newIndex;
	ModifySprite(spriteType, [&](pb::Sprite& sprite) {
		newIndex = insertIndex < 0 ? sprite.regular().fixtures_size() : insertIndex;
		auto* fixture = mutable_insert(sprite.mutable_regular()->mutable_fixtures(), newIndex);
		switch (type) {
			case pb::Fixture::FixtureTypeCase::kRectangle: fixture->mutable_rectangle(); break;
			case pb::Fixture::FixtureTypeCase::kCircle: fixture->mutable_circle(); break;
			case pb::Fixture::FixtureTypeCase::kChain: fixture->mutable_chain(); break;
			default: throw M2_ERROR("Invalid fixture type");
		}
	});
	return newIndex;
}
void PersistentSpriteSheets::RemoveFixtureFromSprite(const m2g::pb::SpriteType spriteType, const int index) {
	ModifySprite(spriteType, [&](pb::Sprite& sprite) {
		sprite.mutable_regular()->mutable_fixtures()->erase(sprite.mutable_regular()->mutable_fixtures()->begin() + index);
	});
}
