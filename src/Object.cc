#include <m2/Object.h>
#include "m2/Game.h"
#include <m2/Log.h>
#include "m2/component/Physique.h"
#include "m2/component/Graphic.h"
#include "m2/component/Light.h"

using namespace m2;

Object::Object(const VecF &position, const m2g::pb::ObjectType type, const ObjectId parent_id) : position(position),
		_object_type(type), _parent_id(parent_id) {}

Object::Object(Object&& other) noexcept :
		position(other.position),
		orientation(other.orientation),
		impl(std::move(other.impl)),
		_object_type(other._object_type),
		_parent_id(other._parent_id),
		_group_id(other._group_id),
		_index_in_group(other._index_in_group),
		_physique_id(other._physique_id),
		_graphicId(other._graphicId),
		_light_id(other._light_id),
		_sound_emitter_id(other._sound_emitter_id),
		_character_id(other._character_id) {
	other._group_id = {};
	other._object_type = {};
	other._parent_id = 0;
	other._index_in_group = 0;
	other._physique_id = 0;
	other._graphicId = 0;
	other._light_id = 0;
	other._sound_emitter_id = 0;
    other._character_id = 0;
}
Object& Object::operator=(Object&& other) noexcept {
	std::swap(position, other.position);
	std::swap(orientation, other.orientation);
	std::swap(impl, other.impl);
	std::swap(_object_type, other._object_type);
	std::swap(_parent_id, other._parent_id);
	std::swap(_group_id, other._group_id);
	std::swap(_index_in_group, other._index_in_group);
	std::swap(_physique_id, other._physique_id);
	std::swap(_graphicId, other._graphicId);
	std::swap(_light_id, other._light_id);
	std::swap(_sound_emitter_id, other._sound_emitter_id);
	std::swap(_character_id, other._character_id);
	return *this;
}

Object::~Object() {
	if (_group_id) {
		M2_LEVEL.groups[_group_id]->RemoveMember(_index_in_group);
	}
	RemovePhysique();
	RemoveGraphic();
	RemoveLight();
	RemoveSoundEmitter();
	RemoveCharacter();
}

ObjectId Object::GetId() const {
	if (!_id) {
		// Looking up the id of the object itself is not very common
		_id = M2_LEVEL.objects.GetId(this);
	}
	return *_id;
}
ObjectId Object::GetParentId() const {
    return _parent_id;
}
GroupIdentifier Object::GetGroupIdentifier() const {
	return _group_id;
}
PhysiqueId Object::GetPhysiqueId() const {
	return _physique_id;
}
GraphicId Object::GetGraphicId() const {
	return _graphicId;
}
LightId Object::GetLightId() const {
	return _light_id;
}
SoundEmitterId Object::GetSoundId() const {
	return _sound_emitter_id;
}
CharacterId Object::GetCharacterId() const {
    return _character_id;
}

Character* Object::TryGetCharacter() const {
	auto* character_variant = M2_LEVEL.characters.Get(_character_id);
	if (not character_variant) {
		return nullptr;
	}
	return &ToCharacterBase(*character_variant);
}
Object* Object::TryGetParent() const {
    return _parent_id ? M2_LEVEL.objects.Get(_parent_id) : nullptr;
}
Group* Object::TryGetGroup() const {
	return _group_id ? M2_LEVEL.groups[_group_id].get() : nullptr;
}
Physique* Object::TryGetPhysique() const {
	return _physique_id ? M2_LEVEL.physics.Get(_physique_id) : nullptr;
}
Graphic* Object::TryGetGraphic() const {
	if (not _graphicId) {
		return nullptr;
	}
	const auto poolAndDrawList = M2_LEVEL.GetGraphicPoolAndDrawList(_graphicId);
	return poolAndDrawList.first.Get(_graphicId);
}

Physique& Object::GetPhysique() const {
	return M2_LEVEL.physics[_physique_id];
}
Graphic& Object::GetGraphic() const {
	auto* ptr = TryGetGraphic();
	if (not ptr) {
		throw M2_ERROR("Graphic component isn't initialized");
	}
	return *ptr;
}
Light& Object::GetLight() const {
	return M2_LEVEL.lights[_light_id];
}
SoundEmitter& Object::GetSoundEmitter() const {
	return M2_LEVEL.soundEmitters[_sound_emitter_id];
}
Character& Object::GetCharacter() const {
    auto& it = M2_LEVEL.characters[_character_id];
    return ToCharacterBase(it);
}

void Object::SetGroup(const GroupIdentifier& group_id, const IndexInGroup group_index) {
	_group_id = group_id;
	_index_in_group = group_index;
}

Physique& Object::AddPhysique() {
	const auto phy = M2_LEVEL.physics.Emplace(GetId());
	_physique_id = phy.GetId();
	return *phy;
}
Graphic& Object::AddGraphic(const DrawLayer layer) {
	const auto poolAndDrawList = M2_LEVEL.GetGraphicPoolAndDrawList(layer);
	const auto it = poolAndDrawList.first.Emplace(GetId());
	_graphicId = it.GetId();
	if (poolAndDrawList.second) {
		poolAndDrawList.second->Insert(GetId());
	}
	return *it.Data();
}
Graphic& Object::AddGraphic(const DrawLayer layer, const m2g::pb::SpriteType spriteType) {
	const auto poolAndDrawList = M2_LEVEL.GetGraphicPoolAndDrawList(layer);
	const auto it = poolAndDrawList.first.Emplace(GetId(), M2_GAME.GetSpriteOrTextLabel(spriteType));
	_graphicId = it.GetId();
	if (poolAndDrawList.second) {
		poolAndDrawList.second->Insert(GetId());
	}
	return *it.Data();
}
Light& Object::AddLight() {
	const auto light = M2_LEVEL.lights.Emplace(GetId());
	_light_id = light.GetId();
	return *light;
}
SoundEmitter& Object::AddSoundEmitter() {
	auto sound = M2_LEVEL.soundEmitters.Emplace(GetId());
	_sound_emitter_id = sound.GetId();
	return *sound;
}
Character& Object::AddTinyCharacter() {
    auto character = M2_LEVEL.characters.Emplace(std::in_place_type<TinyCharacter>, GetId());
    _character_id = character.GetId();
    return std::get<TinyCharacter>(*character);
}
Character& Object::AddFullCharacter() {
    auto character = M2_LEVEL.characters.Emplace(std::in_place_type<FullCharacter>, GetId());
    _character_id = character.GetId();
    return std::get<FullCharacter>(*character);
}

void Object::MoveLayer(const std::optional<PhysicsLayer> newPhysicsLayer, const std::optional<DrawLayer> newDrawLayer) {
	if (newPhysicsLayer) {
		auto* phy = TryGetPhysique();
		if (not phy) {
			throw M2_ERROR("Physique component not found");
		}

		// Check if necessary
		if (const auto currentLayer = phy->GetCurrentPhysicsLayer(); not currentLayer || *currentLayer != *newPhysicsLayer) {
			// TODO Mover is not capable of moving all the fixtures from one world to the other, thus the object loader
			//  must have loaded the same object to both worlds. Bodies with joints cannot be moved as well, for the
			//  same reason.
			auto& newRigidBody = phy->body[I(*newPhysicsLayer)];
			if (not newRigidBody) {
				throw M2_ERROR("New physics layer do not have a rigid body");
			}
			newRigidBody->SetEnabled(true);

			if (currentLayer) {
				auto& currRigidBody = *phy->body[I(*currentLayer)];
				newRigidBody->TeleportToAnother(currRigidBody);
				currRigidBody.SetEnabled(false);
			}
		}
	}

	if (newDrawLayer) {
		auto* gfx = TryGetGraphic();
		if (not gfx) {
			throw M2_ERROR("Graphic component not found");
		}

		if (const auto currentLayer = M2_LEVEL.GetDrawLayer(_graphicId); currentLayer != *newDrawLayer) {
			// Create new component
			const auto newPoolAndDrawList = M2_LEVEL.GetGraphicPoolAndDrawList(*newDrawLayer);
			const auto it = newPoolAndDrawList.first.Emplace(GetId());
			std::swap(*it, *gfx); // Swap contents
			// Release old component
			RemoveGraphic();
			// Store new component
			_graphicId = it.GetId();
			if (newPoolAndDrawList.second) {
				newPoolAndDrawList.second->Insert(GetId());
			}
		}
	}
}

void Object::RemovePhysique() {
	if (_physique_id) {
		M2_LEVEL.physics.Free(_physique_id);
		_physique_id = 0;
	}
}
void Object::RemoveGraphic() {
	if (_graphicId) {
		const auto poolAndDrawList = M2_LEVEL.GetGraphicPoolAndDrawList(_graphicId);
		if (poolAndDrawList.second) {
			poolAndDrawList.second->Remove(GetId());
		}
		poolAndDrawList.first.Free(_graphicId);
		_graphicId = 0;
	}
}
void Object::RemoveLight() {
	if (_light_id) {
		M2_LEVEL.lights.Free(_light_id);
		_light_id = 0;
	}
}
void Object::RemoveSoundEmitter() {
	if (_sound_emitter_id) {
		M2_LEVEL.soundEmitters.Free(_sound_emitter_id);
		_sound_emitter_id = 0;
	}
}
void Object::RemoveCharacter() {
	if (_character_id) {
		M2_LEVEL.characters.Free(_character_id);
		_character_id = 0;
	}
}

Pool<Object>::Iterator m2::CreateObject(const VecF &position, m2g::pb::ObjectType type, ObjectId parent_id) {
    return M2_LEVEL.objects.Emplace(position, type, parent_id);
}
std::function<void()> m2::CreateObjectDeleter(ObjectId id) {
	return [id]() {
		M2_LEVEL.objects.Free(id);
	};
}
std::function<void()> m2::CreatePhysiqueDeleter(ObjectId id) {
	return [id]() {
		if (auto* object = M2_LEVEL.objects.Get(id); object) {
			object->RemovePhysique();
		}
	};
}
std::function<void()> m2::CreateGraphicDeleter(ObjectId id) {
	return [id]() {
		if (auto* object = M2_LEVEL.objects.Get(id); object) {
			object->RemoveGraphic();
		}
	};
}
std::function<void()> m2::CreateLightDeleter(ObjectId id) {
	return [id]() {
		if (auto* object = M2_LEVEL.objects.Get(id); object) {
			object->RemoveLight();
		}
	};
}
std::function<void()> m2::CreateSoundEmitterDeleter(ObjectId id) {
	return [id]() {
		if (auto* object = M2_LEVEL.objects.Get(id); object) {
			object->RemoveSoundEmitter();
		}
	};
}
std::function<void()> m2::CreateCharacterDeleter(ObjectId id) {
	return [id]() {
		if (auto* object = M2_LEVEL.objects.Get(id); object) {
			object->RemoveCharacter();
		}
	};
}
std::function<void()> m2::CreateLayerMover(ObjectId id, std::optional<PhysicsLayer> phyLayer, std::optional<DrawLayer> drawLayer) {
	return [id, phyLayer, drawLayer]() {
		if (auto* object = M2_LEVEL.objects.Get(id)) {
			object->MoveLayer(phyLayer, drawLayer);
		}
	};
}

std::function<bool(Object&)> m2::is_object_in_area(const RectF& rect) {
	return [rect](const Object& o) -> bool {
		return rect.DoesContain(o.position);
	};
}

Object& m2::to_object_of_id(ObjectId id) {
	return M2_LEVEL.objects[id];
}
