#pragma once
#include <m2/ui/UiPanelBlueprint.h>
#include <m2/ui/UiPanelState.h>
#include <m2/sheet_editor/PersistentSpriteSheets.h>
#include <m2/Game.h>

namespace m2::level_editor {
	class DrawFgRightHudState final : public UiPanelStateBase {
		m2g::pb::ObjectType _selectedObjectType;

	public:
		explicit DrawFgRightHudState(const m2g::pb::ObjectType selectedObjectType_) : UiPanelStateBase(),
				_selectedObjectType(selectedObjectType_) {}

		// Accessors

		[[nodiscard]] m2g::pb::ObjectType SelectedObjectType() const {
			return _selectedObjectType;
		}
		[[nodiscard]] m2g::pb::SpriteType SelectedObjectMainSpriteType() const {
			return *M2_GAME.GetMainSpriteOfObject(_selectedObjectType);
		}
	};

	struct ArcDescription {
		int angleInDegrees;
		int pieceCount;
		bool drawTowardsRight;
	};

	struct TangentDescription {
		int first, second;
		float radius;
		int pieceCount;
	};

	extern const UiPanelBlueprint gLeftHudBlueprint;
	extern const UiPanelBlueprint gRightHudBlueprint;
}
