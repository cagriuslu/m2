#pragma once
#include "../ui/UiAction.h"
#include "m2/containers/Pool.h"
#include "m2/math/RectI.h"
#include "m2/math/VecF.h"
#include "m2/math/VecI.h"
#include <m2g_SpriteType.pb.h>
#include <m2/sheet_editor/PersistentSpriteSheets.h>
#include <Level.pb.h>

namespace m2::level_editor {
	// Forward declarations
	struct ArcDescription;
	struct TangentDescription;

	class State {
		using BackgroundSpritePlaceholderMap = std::map<VecI, std::tuple<Id, m2g::pb::SpriteType>, VecICompareTopLeftToBottomRight>;
		using BackgroundSpriteClipboardMap = std::map<VecI, m2g::pb::SpriteType, VecICompareTopLeftToBottomRight>;
		using ForegroundObjectPlaceholderMap = std::multimap<VecF, std::tuple<Id, pb::LevelObject>, VecFCompareTopLeftToBottomRight>;
		using ForegroundObjectClipboardMap = std::multimap<VecF, pb::LevelObject, VecFCompareTopLeftToBottomRight>;

		std::array<BackgroundSpritePlaceholderMap, gBackgroundLayerCount> _backgroundSpritePlaceholders;
		ForegroundObjectPlaceholderMap _foregroundObjectPlaceholders;
		BackgroundSpriteClipboardMap _backgroundSpriteClipboard;
		ForegroundObjectClipboardMap _foregroundObjectClipboard;

		/// Keep the sprite sheet open as well, either for drawing or displaying the fixtures
		std::optional<sheet_editor::PersistentSpriteSheets> _persistentSpriteSheets;

	public:
		/// ID of the ghost object, the object that follows the mouse around.
		Id ghostId{};

		/// Set of objects drawn independent of the current mode
		std::set<m2g::pb::ObjectType> physicsObjectsToDraw;

		State();

		// Accessors

		[[nodiscard]] BackgroundLayer GetSelectedBackgroundLayer() const;
		[[nodiscard]] bool GetSnapToGridStatus() const;
		[[nodiscard]] std::vector<ForegroundObjectPlaceholderMap::const_iterator> GetForegroundObjectsOfType(m2g::pb::ObjectType) const;

		[[nodiscard]] const pb::Sprite& GetSpritePb(const m2g::pb::SpriteType st) const { return _persistentSpriteSheets->SpritePb(st); }
		[[nodiscard]] std::vector<pb::Fixture::FixtureTypeCase> GetSpriteFixtureTypes(m2g::pb::SpriteType st) const;

		// Modifiers

		void LoadLevelBlueprint(const pb::Level& lb);

		void HandleMousePrimaryButton(const VecF& position);
		void HandleMouseSecondaryButton(const VecF& position);
		void HandleMousePrimarySelectionComplete(const VecF& firstPosition, const VecF& secondPosition);

		void EraseBackground(const RectI& area);
		void CopyBackground(const RectI& area);
		void PasteBackground(const VecI& position);
		void RandomFillBackground(const RectI& area, const std::vector<m2g::pb::SpriteType>& spriteSet);

		void RemoveForegroundObject();
		void CopyForeground();
		void PasteForeground();

		int AddChain(m2g::pb::SpriteType st, int insertIndex);
		void RemoveFixture(m2g::pb::SpriteType st, int selectedIndex);
		void StoreWorldPoint(int selectedIndex, const VecF& pointM);
		void StoreArc(int selectedIndex, const VecF& pointM, const ArcDescription& arc);
		void StoreTangent(int selectedIndex, const TangentDescription& tangent);
		void UndoPoint(int selectedIndex);

		void Draw() const;
		[[nodiscard]] void_expected Save();

	private:
		void PaintBackground(const VecI& position, m2g::pb::SpriteType spriteType);
		void PlaceForeground(const VecF& position, float orientation, m2g::pb::ObjectType objectType, m2g::pb::GroupType groupType, unsigned groupInstance);
		/// Returns the iterator to the sampled placeholder if successful
		ForegroundObjectPlaceholderMap::iterator ApplySampling(const VecF& position);
		[[nodiscard]] RectF ForegroundSelectionArea() const;
		static int SpritePpm(ForegroundObjectPlaceholderMap::const_iterator fgObject);
		static VecF WorldCoordinateToSpriteCoordinate(ForegroundObjectPlaceholderMap::const_iterator fgObject, const VecF& worldCoordinate);
		void StoreFixturePoint(m2g::pb::SpriteType st, int selectedIndex, const VecF& spriteOriginToPointVec);
		void StoreArc(int selectedIndex, const VecF& fromPointOffset, const VecF& toPointOffset, float angleInRads, int pieceCount, bool drawTowardsRight);
		static std::optional<int> FindClosestChainPointInRange(const pb::Fixture_ChainFixture& chain, int spritePpm, const VecF& positionPx);
	};
}
