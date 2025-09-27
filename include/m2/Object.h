#pragma once
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
#include "ObjectId.h"
#include <M2.pb.h>
#include <functional>
#include <memory>

namespace m2 {
	/// Basis of all objects in the game.
	/// How to decide if a component should reside in Pool or impl?
	/// If the component is iterated by the Main Game Loop => Pool
	/// If the component is created and destroyed rapidly => Pool
	/// Else => impl
	struct Object final {
		std::unique_ptr<ObjectImpl> impl; /// Custom Data

		Object() = default;
		explicit Object(m2g::pb::ObjectType type, ObjectId parent_id = 0);
		// Copy not allowed
		Object(const Object& other) = delete;
		Object& operator=(const Object& other) = delete;
		// Move constructors
		Object(Object&& other) noexcept;
		Object& operator=(Object&& other) noexcept;
		// Destructor
		~Object();

		// Accessors

		[[nodiscard]] ObjectId GetId() const;
		[[nodiscard]] m2g::pb::ObjectType GetType() const { return _object_type; }
		[[nodiscard]] ObjectId GetParentId() const;
		[[nodiscard]] GroupIdentifier GetGroupIdentifier() const;
		[[nodiscard]] PhysiqueId GetPhysiqueId() const;
		[[nodiscard]] GraphicId GetGraphicId() const;
		[[nodiscard]] LightId GetLightId() const;
		[[nodiscard]] SoundEmitterId GetSoundId() const;
		[[nodiscard]] CharacterId GetCharacterId() const;

		[[nodiscard]] Object* TryGetParent() const;
		[[nodiscard]] Group* TryGetGroup() const;
		[[nodiscard]] Physique* TryGetPhysique() const;
		[[nodiscard]] Graphic* TryGetGraphic() const;
		[[nodiscard]] Light* TryGetLight() const;
		[[nodiscard]] SoundEmitter* TryGetSoundEmitter() const;
		[[nodiscard]] Character* TryGetCharacter() const;

		[[nodiscard]] Physique& GetPhysique() const;
		[[nodiscard]] Graphic& GetGraphic() const;
		[[nodiscard]] Light& GetLight() const;
		[[nodiscard]] SoundEmitter& GetSoundEmitter() const;
		[[nodiscard]] Character& GetCharacter() const;

		[[nodiscard]] VecF InferPositionF() const;

		// Modifiers

		void SetGroup(const GroupIdentifier& group_id, IndexInGroup group_index);
		Physique& AddPhysique(const VecFE& position = {});
		Graphic& AddGraphic(DrawLayer layer, const VecF& position = {});
		Graphic& AddGraphic(DrawLayer layer, m2g::pb::SpriteType, const VecF& position = {});
		Light& AddLight();
		SoundEmitter& AddSoundEmitter();
		Character& AddCompactCharacter();
		Character& AddFastCharacter();

		/// This method may cause some of the components IDs to change. Must be called from a deferred action.
		void MoveLayer(std::optional<pb::PhysicsLayer>, std::optional<DrawLayer>);

		void RemovePhysique();
		void RemoveGraphic();
		void RemoveLight();
		void RemoveSoundEmitter();
		void RemoveCharacter();

	   private:
		mutable std::optional<ObjectId> _id;
		m2g::pb::ObjectType _object_type{};
		ObjectId _parent_id{};
		GroupIdentifier _group_id{};
		IndexInGroup _index_in_group{};

		// Components

		PhysiqueId _physique_id{};
		GraphicId _graphicId{};
		LightId _light_id{}; // TODO make part of another component?
		SoundEmitterId _sound_emitter_id{};
		CharacterId _character_id{};
	};

	Pool<Object>::Iterator CreateObject(m2g::pb::ObjectType type = {}, ObjectId parent_id = 0); // TODO add orientation to the params
	std::function<void()> CreateObjectDeleter(ObjectId id);
	std::function<void()> CreatePhysiqueDeleter(ObjectId id);
	std::function<void()> CreateGraphicDeleter(ObjectId id);
	std::function<void()> CreateLightDeleter(ObjectId id);
	std::function<void()> CreateSoundEmitterDeleter(ObjectId id);
	std::function<void()> CreateCharacterDeleter(ObjectId id);
	std::function<void()> CreateLayerMover(ObjectId id, std::optional<pb::PhysicsLayer>, std::optional<DrawLayer>);

	// Filter Generators

	std::function<bool(Object&)> is_object_in_area(const RectF& rect);

	// Transformers

	Object& to_object_of_id(ObjectId id);
	inline Character& to_character_of_object_unsafe(Object* o) { return o->GetCharacter(); }
	inline Character& to_character_of_object(Object& o) { return o.GetCharacter(); }
}
