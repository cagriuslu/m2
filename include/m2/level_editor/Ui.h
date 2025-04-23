#pragma once
#include <m2/ui/UiPanelBlueprint.h>
#include <m2/ui/UiPanelState.h>
#include <m2/sheet_editor/PersistentSpriteSheets.h>
#include <m2/Game.h>

namespace m2::level_editor {
	class DrawFgRightHudState final : public UiPanelStateBase {
		sheet_editor::PersistentSpriteSheets _persistentSpriteSheets;
		m2g::pb::ObjectType _selectedObjectType;
	public:
		explicit DrawFgRightHudState(const m2g::pb::ObjectType selectedObjectType_) : UiPanelStateBase(),
				_persistentSpriteSheets(m2MoveOrThrowError(sheet_editor::PersistentSpriteSheets::LoadFile(M2_GAME.spriteSheetsPath))),
				_selectedObjectType(selectedObjectType_) {}

		// Accessors

		[[nodiscard]] m2g::pb::ObjectType SelectedObjectType() const {
			return _selectedObjectType;
		}
		[[nodiscard]] m2g::pb::SpriteType SelectedObjectMainSpriteType() const {
			return M2_GAME.object_main_sprites[_selectedObjectType];
		}
		const pb::Sprite& SelectedObjectMainSpritePb() const {
			return _persistentSpriteSheets.SpritePb(SelectedObjectMainSpriteType());
		}
		[[nodiscard]] int SelectedSpriteFixtureCount() const {
			return _persistentSpriteSheets.SpritePb(SelectedObjectMainSpriteType()).regular().fixtures_size();
		}
		[[nodiscard]] std::vector<pb::Fixture::FixtureTypeCase> SelectedSpriteFixtureTypes() const {
			return _persistentSpriteSheets.SpriteFixtureTypes(SelectedObjectMainSpriteType());
		}

		// Modifiers

		int AddChain(const int insertIndex) {
			return _persistentSpriteSheets.AddFixtureToSprite(SelectedObjectMainSpriteType(),
					pb::Fixture::FixtureTypeCase::kChain, insertIndex);
		}
		void RemoveFixture(const int index) {
			_persistentSpriteSheets.RemoveFixtureFromSprite(SelectedObjectMainSpriteType(), index);
		}
	};

	extern const UiPanelBlueprint gLeftHudBlueprint;
	extern const UiPanelBlueprint gRightHudBlueprint;
}
