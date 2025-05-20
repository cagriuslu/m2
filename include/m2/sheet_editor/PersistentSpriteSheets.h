#pragma once
#include <m2/protobuf/PersistentObject.h>
#include <Sprite.pb.h>

namespace m2::sheet_editor {
	class PersistentSpriteSheets final : pb::PersistentObject<pb::SpriteSheets> {
		explicit PersistentSpriteSheets(pb::PersistentObject<pb::SpriteSheets>&& po) : pb::PersistentObject<pb::SpriteSheets>(std::move(po)) {}

	public:
		static expected<PersistentSpriteSheets> LoadFile(std::filesystem::path path);

		// Accessors

		std::vector<m2g::pb::SpriteType> AllSpriteTypes() const;
		const pb::SpriteSheet* SpriteSheetPbWithSprite(m2g::pb::SpriteType spriteType) const;
		const pb::Sprite& SpritePb(m2g::pb::SpriteType spriteType) const;
		const google::protobuf::RepeatedPtrField<pb::Fixture>& SpriteFixtures(m2g::pb::SpriteType spriteType) const;
		std::vector<pb::Fixture::FixtureTypeCase> SpriteFixtureTypes(m2g::pb::SpriteType spriteType) const;

		// Modifiers

		void ModifySprite(m2g::pb::SpriteType spriteType, const std::function<void(pb::Sprite&)>& modifier);
		/// Returns the index of the newly added fixture. If insertIndex is negative, the new fixture is added at the end.
		int AddFixtureToSprite(m2g::pb::SpriteType spriteType, pb::Fixture::FixtureTypeCase type, int insertIndex = -1);
		void RemoveFixtureFromSprite(m2g::pb::SpriteType spriteType, int index);

		void_expected Save();
	};
}
