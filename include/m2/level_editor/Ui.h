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

		std::set<m2g::pb::ObjectType> physicsObjectsToDraw;

		// Accessors

		[[nodiscard]] m2g::pb::ObjectType SelectedObjectType() const {
			return _selectedObjectType;
		}
		[[nodiscard]] m2g::pb::SpriteType SelectedObjectMainSpriteType() const {
			return *M2_GAME.GetMainSpriteOfObject(_selectedObjectType);
		}
		const pb::Sprite& SelectedObjectMainSpritePb() const {
			return _persistentSpriteSheets.SpritePb(SelectedObjectMainSpriteType());
		}
		const pb::Sprite& SpritePb(const m2g::pb::SpriteType st) const {
			return _persistentSpriteSheets.SpritePb(st);
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
		void StorePoint(const int index, const VecF& spriteOriginToPointVec) {
			_persistentSpriteSheets.ModifySprite(SelectedObjectMainSpriteType(), [&](pb::Sprite& sprite) {
				if (auto* fixture = sprite.mutable_regular()->mutable_fixtures(index); fixture->has_chain()) {
					auto* chain = fixture->mutable_chain();
					auto* point = chain->add_points();
					point->set_x(spriteOriginToPointVec.x);
					point->set_y(spriteOriginToPointVec.y);
				}
			});
		}
		void UndoPoint(const int index) {
			_persistentSpriteSheets.ModifySprite(SelectedObjectMainSpriteType(), [&](pb::Sprite& sprite) {
				if (auto* fixture = sprite.mutable_regular()->mutable_fixtures(index); fixture->has_chain()) {
					if (auto* chain = fixture->mutable_chain(); chain->points_size()) {
						auto* points = chain->mutable_points();
						points->erase(points->end() - 1);
					}
				}
			});
		}
	};

	extern const UiPanelBlueprint gLeftHudBlueprint;
	extern const UiPanelBlueprint gRightHudBlueprint;
}
