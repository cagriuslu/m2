#pragma once
#include <functional>
#include <memory>

#include "Group.h"
#include "ObjectImpl.h"
#include <m2/containers/Pool.h>
#include <m2/video/Sprite.h>
#include "component/Character.h"
#include "component/Graphic.h"
#include "component/Light.h"
#include "component/Physique.h"
#include "component/SoundEmitter.h"
#include "m2/Component.h"
#include "m2/game/Fsm.h"
#include "m2/math/VecF.h"

namespace m2 {
	using ObjectId = Id;
	using PhysiqueId = Id;
	using GraphicId = Id;
	using LightId = Id;
	using SoundEmitterId = Id;
	using CharacterId = Id;

	/// Basis of all objects in the game.
	/// How to decide if a component should reside in Pool or impl?
	/// If the component is iterated by the Main Game Loop => Pool
	/// If the component is created and destroyed rapidly => Pool
	/// Else => impl
	struct Object final {
		VecF position;
		float orientation; /// In radians
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

		[[nodiscard]] ObjectId GetId() const;
		[[nodiscard]] m2g::pb::ObjectType GetType() const { return _object_type; }
		[[nodiscard]] ObjectId GetParentId() const;
		[[nodiscard]] GroupId GetGroupId() const;
		[[nodiscard]] PhysiqueId GetPhysiqueId() const;
		[[nodiscard]] GraphicId GetGraphicId() const;
		[[nodiscard]] std::pair<GraphicId, BackgroundLayer> GetTerrainGraphicId() const;
		[[nodiscard]] LightId GetLightId() const;
		[[nodiscard]] SoundEmitterId GetSoundId() const;
		[[nodiscard]] CharacterId GetCharacterId() const;

		[[nodiscard]] Character* TryGetCharacter() const;
		[[nodiscard]] Object* TryGetParent() const;
		[[nodiscard]] Group* TryGetGroup() const;

		[[nodiscard]] Physique& GetPhysique() const;
		[[nodiscard]] Graphic& GetGraphic() const;
		[[nodiscard]] Graphic& GetTerrainGraphic() const;
		[[nodiscard]] Light& GetLight() const;
		[[nodiscard]] SoundEmitter& GetSoundEmitter() const;
		[[nodiscard]] Character& GetCharacter() const;

		void SetGroup(const GroupId& group_id, IndexInGroup group_index);
		Physique& AddPhysique();
		Graphic& AddGraphic();
		Graphic& AddGraphic(m2g::pb::SpriteType);
		Graphic& AddTerrainGraphic(BackgroundLayer layer);
		Graphic& AddTerrainGraphic(BackgroundLayer layer, m2g::pb::SpriteType);
		Light& AddLight();
		SoundEmitter& AddSoundEmitter();
		Character& AddTinyCharacter();
		// TODO mini(2),small(4),medium(8),large(16),huge(32)
		Character& AddFullCharacter();

		/// If the object has background elements, they are moved between different layers of the background. This may
		/// cause some of the components IDs to change. Must be called from a deferred action.
		void MoveToBackgroundLayer(BackgroundLayer);
		/// If the object has foreground elements, they are moved between different layers of the foreground. This may
		/// cause some of the components IDs to change. Must be called from a deferred action.
		void MoveToForegroundLayer(ForegroundLayer);

		void RemovePhysique();
		void RemoveGraphic();
		void RemoveTerrainGraphic();
		void RemoveLight();
		void RemoveSoundEmitter();
		void RemoveCharacter();

	   private:
		mutable std::optional<ObjectId> _id;
		m2g::pb::ObjectType _object_type{};
		ObjectId _parent_id{};
		GroupId _group_id{}; // TODO group isn't a common feature. make it part of some other component
		IndexInGroup _index_in_group{};
		// Components
		PhysiqueId _physique_id{};
		GraphicId _graphic_id{}; // TODO an object shouldn't have both foreground and background texture. should we infer the graphic id based on Pool Id?
		std::pair<GraphicId, BackgroundLayer> _terrain_graphic_id{}; // TODO background layer shouldn't be necessary, we can learn it from the GraphicId
		LightId _light_id{}; // TODO make part of another component?
		SoundEmitterId _sound_emitter_id{};
		CharacterId _character_id{};
	};

	Pool<Object>::Iterator CreateObject(const m2::VecF& position, m2g::pb::ObjectType type = {}, ObjectId parent_id = 0);
	std::function<void()> CreateObjectDeleter(ObjectId id);
	std::function<void()> CreatePhysiqueDeleter(ObjectId id);
	std::function<void()> CreateGraphicDeleter(ObjectId id);
	std::function<void()> CreateTerrainGraphicDeleter(ObjectId id);
	std::function<void()> CreateLightDeleter(ObjectId id);
	std::function<void()> CreateSoundEmitterDeleter(ObjectId id);
	std::function<void()> CreateCharacterDeleter(ObjectId id);
	std::function<void()> CreateForegroundLayerMover(ObjectId id, ForegroundLayer toLayer);

	// Filters

	// Filter Generators
	std::function<bool(Object&)> is_object_in_area(const RectF& rect);

	// Transformers

	Object& to_object_of_id(ObjectId id);
	inline Character& to_character_of_object_unsafe(Object* o) { return o->GetCharacter(); }
	inline Character& to_character_of_object(Object& o) { return o.GetCharacter(); }
}  // namespace m2
