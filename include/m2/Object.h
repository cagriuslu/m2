#pragma once
#include <functional>
#include <memory>

#include "Group.h"
#include "ObjectImpl.h"
#include "Pool.h"
#include "Sprite.h"
#include "component/Character.h"
#include "component/Graphic.h"
#include "component/Light.h"
#include "component/Physique.h"
#include "component/SoundEmitter.h"
#include "m2/Component.h"
#include "m2/Fsm.h"
#include "m2/VecF.h"

namespace m2 {
	using ObjectId = Id;
	using PhysiqueId = Id;
	using GraphicId = Id;
	using LightId = Id;
	using SoundEmitterId = Id;
	using CharacterId = Id;

	/// Basis of all objects in the game.
	/// How to decide if a component should reside in Pool or data?
	/// If the component is iterated by the Main Game Loop => Pool
	/// If the component is created and destroyed rapidly => Pool
	/// Else => Data
	struct Object final {
		m2::VecF position;
		// Custom Data
		std::unique_ptr<ObjectImpl> impl;

		Object() = default;
		explicit Object(const VecF& position, m2g::pb::ObjectType type = {}, ObjectId parent_id = 0);
		// Copy not allowed
		Object(const Object& other) = delete;
		Object& operator=(const Object& other) = delete;
		// Move constructors
		Object(Object&& other) noexcept;
		Object& operator=(Object&& other) noexcept;
		// Destructor
		~Object();

		[[nodiscard]] ObjectId id() const;
		[[nodiscard]] m2g::pb::ObjectType object_type() const { return _object_type; }
		[[nodiscard]] ObjectId parent_id() const;
		[[nodiscard]] GroupId group_id() const;
		[[nodiscard]] PhysiqueId physique_id() const;
		[[nodiscard]] GraphicId graphic_id() const;
		[[nodiscard]] std::pair<GraphicId, BackgroundLayer> terrain_graphic_id() const;
		[[nodiscard]] LightId light_id() const;
		[[nodiscard]] SoundEmitterId sound_id() const;
		[[nodiscard]] CharacterId character_id() const;

		[[nodiscard]] Character* get_character() const;

		[[nodiscard]] Object* parent() const;
		[[nodiscard]] Group* group() const;
		[[nodiscard]] Physique& physique() const;
		[[nodiscard]] Graphic& graphic() const;
		[[nodiscard]] Graphic& terrain_graphic() const;
		[[nodiscard]] Light& light() const;
		[[nodiscard]] SoundEmitter& sound_emitter() const;
		[[nodiscard]] Character& character() const;

		void set_group(const GroupId& group_id, IndexInGroup group_index);
		Physique& add_physique();
		Graphic& add_graphic();
		Graphic& add_graphic(const Sprite& sprite);
		Graphic& add_graphic(m2g::pb::SpriteType);
		Graphic& add_terrain_graphic(BackgroundLayer layer);
		Graphic& add_terrain_graphic(BackgroundLayer layer, const Sprite& sprite);
		Light& add_light();
		SoundEmitter& add_sound_emitter();
		Character& add_tiny_character();
		// TODO mini(2),small(4),medium(8),large(16),huge(32)
		Character& add_full_character();

		void remove_physique();
		void remove_graphic();
		void remove_terrain_graphic();
		void remove_light();
		void remove_sound_emitter();
		void remove_character();

	   private:
		mutable std::optional<ObjectId> _id;
		m2g::pb::ObjectType _object_type{};
		ObjectId _parent_id{};
		GroupId _group_id{};
		IndexInGroup _index_in_group{};
		// Components
		PhysiqueId _physique_id{};
		GraphicId _graphic_id{};
		std::pair<GraphicId, BackgroundLayer> _terrain_graphic_id{};
		LightId _light_id{};
		SoundEmitterId _sound_emitter_id{};
		CharacterId _character_id{};
	};

	Pool<Object>::Iterator create_object(const m2::VecF& position, m2g::pb::ObjectType type = {}, ObjectId parent_id = 0);
	std::function<void(void)> create_object_deleter(ObjectId id);
	std::function<void(void)> create_physique_deleter(ObjectId id);
	std::function<void(void)> create_graphic_deleter(ObjectId id);
	std::function<void(void)> create_terrain_graphic_deleter(ObjectId id);
	std::function<void(void)> create_light_deleter(ObjectId id);
	std::function<void(void)> create_sound_emitter_deleter(ObjectId id);
	std::function<void(void)> create_character_deleter(ObjectId id);

	// Filters
	// Filter Generators
	std::function<bool(Object&)> is_object_in_area(const RectF& rect);
	// Transformers
	Object& to_object_of_id(ObjectId id);
	inline Character& to_character_of_object_unsafe(Object* o) { return o->character(); }
	inline Character& to_character_of_object(Object& o) { return o.character(); }
	inline Object& to_parent_of_component(Character& v) { return v.parent(); }
}  // namespace m2
