#pragma once
#include <Sprite.pb.h>
#include <m2g_SpriteType.pb.h>
#include <m2/protobuf/PersistentObject.h>
#include <filesystem>

#include "../math/CircF.h"
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

		// Modifiers

		void Select(m2g::pb::SpriteType);
		void SetSpriteRect(const RectI& rect);
		void SetSpriteOrigin(const VecF& origin);
		void ResetSpriteRectAndOrigin();
		void AddForegroundCompanionRect(const RectI& rect);
		void SetForegroundCompanionOrigin(const VecF& origin);
		void ResetForegroundCompanion();
		void AddRectangleFixture(bool foreground, const RectF& rect);
		void AddCircleFixture(bool foreground, const VecF& center, float radius);
		void AddChainFixturePoint(bool foreground, const VecF& point);
		void ResetRectangleFixtures(bool foreground);
		void ResetCircleFixtures(bool foreground);
		void ResetChainFixturePoints(bool foreground);

		void Draw() const;

	   private:
		explicit State(pb::PersistentObject<pb::SpriteSheets>&& persistentSpriteSheets)
				: _persistentSpriteSheets(std::move(persistentSpriteSheets)) {}

		const pb::Sprite& SelectedSprite() const;
		void ModifySelectedSprite(const std::function<void(pb::Sprite&)>& modifier);
		VecF SelectedSpriteCenter() const;
		VecF SelectedSpriteOrigin() const;
	};

	void modify_sprite_in_sheet(
	    const std::filesystem::path& path, m2g::pb::SpriteType type, const std::function<void(pb::Sprite&)>& modifier); // TODO remove and use ModifySpriteInSheets
	void ModifySpriteInSheets(pb::PersistentObject<pb::SpriteSheets>& persistentObject, m2g::pb::SpriteType spriteType,
			const std::function<void(pb::Sprite&)>& modifier);
}  // namespace m2::sheet_editor
