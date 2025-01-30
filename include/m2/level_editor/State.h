#pragma once
#include "../ui/UiAction.h"
#include "m2/containers/Pool.h"
#include "m2/math/RectI.h"
#include "m2/math/VecF.h"
#include "m2/math/VecI.h"

namespace m2::level_editor {
	class State {
		using BackgroundSpritePlaceholderMap = std::map<VecI, std::tuple<Id, m2g::pb::SpriteType>, VecICompareTopLeftToBottomRight>;
		using BackgroundSpriteClipboardMap = std::map<VecI, m2g::pb::SpriteType, VecICompareTopLeftToBottomRight>;
		using ForegroundObjectPlaceholderMap = std::multimap<VecF, std::tuple<Id, pb::LevelObject>, VecFCompareTopLeftToBottomRight>;
		using ForegroundObjectClipboardMap = std::multimap<VecF, pb::LevelObject, VecFCompareTopLeftToBottomRight>;

		std::array<BackgroundSpritePlaceholderMap, gBackgroundLayerCount> _backgroundSpritePlaceholders;
		ForegroundObjectPlaceholderMap _foregroundObjectPlaceholders;
		BackgroundSpriteClipboardMap _backgroundSpriteClipboard;
		ForegroundObjectClipboardMap _foregroundObjectClipboard;

	public:
		Id ghostId{};

		// Accessors

		BackgroundLayer GetSelectedBackgroundLayer() const;
		bool GetSnapToGridStatus() const;

		// Modifiers

		void LoadLevelBlueprint(const pb::Level& lb);

		void HandleMousePrimaryButton(const VecF& position);

		void EraseBackground(const RectI& area);
		void CopyBackground(const RectI& area);
		void PasteBackground(const VecI& position);
		void RandomFillBackground(const RectI& area, const std::vector<m2g::pb::SpriteType>& spriteSet);

		void RemoveForegroundObject();
		void CopyForeground();
		void PasteForeground();

		void Draw() const;
		void_expected Save() const;

	private:
		void PaintBackground(const VecI& position, m2g::pb::SpriteType spriteType);
		void PlaceForeground(const VecF& position, float orientation, m2g::pb::ObjectType objectType, m2g::pb::GroupType groupType, unsigned groupInstance);
		RectF ForegroundSelectionArea() const;
	};
}
