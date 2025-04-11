#pragma once
#include <Sprite.pb.h>
#include <m2g_SpriteType.pb.h>
#include <m2/protobuf/PersistentObject.h>
#include <filesystem>

#include "Ui.h"
#include "../Meta.h"
#include "../ui/UiPanel.h"

namespace m2::sheet_editor {
	class State {
		pb::PersistentObject<pb::SpriteSheets> _persistentSpriteSheets;

		m2g::pb::SpriteType _selected_sprite_type{};
		sdl::TextureUniquePtr _texture;
		VecI _textureDimensions;
		int _ppm{};

	public:
		static expected<State> create(const std::filesystem::path& path);

		// Accessors

		const pb::SpriteSheets& SpriteSheets() const { return _persistentSpriteSheets.Cache(); }
		int SelectedSpriteFixtureCount() const;
		std::vector<FixtureType> SelectedSpriteFixtureTypes() const;

		// Modifiers

		void Select(m2g::pb::SpriteType);
		void SetSpriteRect(const RectI& rect);
		void SetSpriteOrigin(const VecF& origin);
		void ResetSpriteRectAndOrigin();
		void AddForegroundCompanionRect(const RectI& rect);
		void SetForegroundCompanionOrigin(const VecF& origin);
		void ResetForegroundCompanion();
		/// Returns the index of the newly added fixture. If insertIndex is negative, the new fixture is added at the end.
		int AddFixture(FixtureType type, int insertIndex = -1);
		void RemoveFixture(int index);
		void StoreFixture(int index, const RectF& rect, const VecF& point1, const VecF& point2);
		void UndoChainFixturePoint(int index);

		void Draw() const;

	   private:
		explicit State(pb::PersistentObject<pb::SpriteSheets>&& persistentSpriteSheets)
				: _persistentSpriteSheets(std::move(persistentSpriteSheets)) {}

		const pb::Sprite& SelectedSprite() const;
		void ModifySelectedSprite(const std::function<void(pb::Sprite&)>& modifier);
		VecF SelectedSpriteCenter() const;
		VecF SelectedSpriteOrigin() const;
	};

	// TODO remove and use ModifySpriteInSheets
	void modify_sprite_in_sheet(const std::filesystem::path& path, m2g::pb::SpriteType type, const std::function<void(pb::Sprite&)>& modifier);
	void ModifySpriteInSheets(pb::PersistentObject<pb::SpriteSheets>& persistentObject, m2g::pb::SpriteType spriteType,
			const std::function<void(pb::Sprite&)>& modifier);
}  // namespace m2::sheet_editor
