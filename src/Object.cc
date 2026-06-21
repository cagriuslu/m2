#include <m2/Object.h>
#include "m2/Game.h"
#include <m2/Log.h>
#include "m2/component/Physique.h"
#include "m2/component/Graphic.h"

using namespace m2;

Object::Object(const m2g::pb::ObjectType type, const ObjectId parent_id) : _object_type(type), _parent_id(parent_id) {}

Object::Object(Object&& other) noexcept :
		impl(std::move(other.impl)),
		_object_type(other._object_type),
		_parent_id(other._parent_id),
		_group_id(other._group_id),
		_index_in_group(other._index_in_group),
		_physique_id(other._physique_id),
		_graphicId(other._graphicId),
		_sound_emitter_id(other._sound_emitter_id),
		_character_id(other._character_id) {
	other._group_id = {};
	other._object_type = {};
	other._parent_id = 0;
	other._index_in_group = 0;
	other._physique_id = 0;
	other._graphicId = 0;
	other._sound_emitter_id = 0;
    other._character_id = 0;
}
Object& Object::operator=(Object&& other) noexcept {
	std::swap(impl, other.impl);
	std::swap(_object_type, other._object_type);
	std::swap(_parent_id, other._parent_id);
	std::swap(_group_id, other._group_id);
	std::swap(_index_in_group, other._index_in_group);
	std::swap(_physique_id, other._physique_id);
	std::swap(_graphicId, other._graphicId);
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
SoundEmitterId Object::GetSoundId() const {
	return _sound_emitter_id;
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
SoundEmitter* Object::TryGetSoundEmitter() const {
	return _sound_emitter_id ? M2_LEVEL.soundEmitters.Get(_sound_emitter_id) : nullptr;
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
SoundEmitter& Object::GetSoundEmitter() const {
	return M2_LEVEL.soundEmitters[_sound_emitter_id];
}

VecF Object::InferPositionF() const {
	if (_physique_id) {
		return static_cast<VecF>(GetPhysique().GetPosition());
	} else if (_graphicId) {
		return GetGraphic().position;
	} else if (_sound_emitter_id) {
		return GetSoundEmitter().position;
	}
	return {};
}

void Object::SetParent(const ObjectId parentId) {
	_parent_id = parentId;
}
void Object::SetGroup(const GroupIdentifier& group_id, const IndexInGroup group_index) {
	_group_id = group_id;
	_index_in_group = group_index;
}

Physique& Object::AddPhysique(const VecFE& position) {
	const auto phy = M2_LEVEL.physics.Emplace(GetId(), position);
	_physique_id = phy.GetId();
	return *phy;
}
Graphic& Object::AddGraphic(const DrawLayer layer, const VecF& position) {
	const auto poolAndDrawList = M2_LEVEL.GetGraphicPoolAndDrawList(layer);
	const auto it = poolAndDrawList.first.Emplace(GetId(), position);
	_graphicId = it.GetId();
	if (poolAndDrawList.second) {
		poolAndDrawList.second->Insert(GetId(), _graphicId, position);
	}
	return *it.Data();
}
Graphic& Object::AddGraphic(const DrawLayer layer, const m2g::pb::SpriteType spriteType, const VecF& position) {
	const auto poolAndDrawList = M2_LEVEL.GetGraphicPoolAndDrawList(layer);
	const auto it = poolAndDrawList.first.Emplace(GetId(), M2_GAME.GetSpriteOrTextLabel(spriteType), position);
	_graphicId = it.GetId();
	if (poolAndDrawList.second) {
		poolAndDrawList.second->Insert(GetId(), _graphicId, position);
	}
	return *it.Data();
}
SoundEmitter& Object::AddSoundEmitter() {
	auto sound = M2_LEVEL.soundEmitters.Emplace(GetId());
	_sound_emitter_id = sound.GetId();
	return *sound;
}

void Object::MoveLayer(const std::optional<DrawLayer> newDrawLayer) {
	if (newDrawLayer) {
		auto* gfx = TryGetGraphic();
		if (not gfx) {
			throw M2_ERROR("Graphic component not found");
		}

		if (const auto currentLayer = M2_LEVEL.GetDrawLayer(_graphicId); currentLayer != *newDrawLayer) {
			const auto position = gfx->position;
			// Create new component
			const auto newPoolAndDrawList = M2_LEVEL.GetGraphicPoolAndDrawList(*newDrawLayer);
			const auto it = newPoolAndDrawList.first.Emplace(GetId());
			std::swap(*it, *gfx); // Swap contents
			// Release old component
			RemoveGraphic();
			// Store new component
			_graphicId = it.GetId();
			if (newPoolAndDrawList.second) {
				newPoolAndDrawList.second->Insert(GetId(), _graphicId, position);
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
void Object::RemoveSoundEmitter() {
	if (_sound_emitter_id) {
		M2_LEVEL.soundEmitters.Free(_sound_emitter_id);
		_sound_emitter_id = 0;
	}
}
void Object::RemoveCharacter() {
	if (_character_id) {
		M2_LEVEL.GetCharacterStorage().Free(_character_id);
		_character_id = 0;
	}
}

Pool<Object>::Iterator m2::CreateObject(m2g::pb::ObjectType type, ObjectId parent_id) {
    return M2_LEVEL.objects.Emplace(type, parent_id);
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
std::function<void()> m2::CreateLayerMover(ObjectId id, std::optional<DrawLayer> drawLayer) {
	return [id, drawLayer]() {
		if (auto* object = M2_LEVEL.objects.Get(id)) {
			object->MoveLayer(drawLayer);
		}
	};
}

std::function<bool(Object&)> m2::IsObjectInArea(const RectF& rect) {
	return [rect](const Object& o) -> bool {
		return rect.DoesContain(o.InferPositionF());
	};
}

Object& m2::ObjectIdToObject(const ObjectId id) {
	return M2_LEVEL.objects[id];
}
const Object& m2::ObjectIdToConstObject(const ObjectId id) {
	return M2_LEVEL.objects[id];
}
