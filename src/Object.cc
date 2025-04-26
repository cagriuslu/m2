#include <m2/Object.h>
#include "m2/Game.h"
#include <m2/Log.h>
#include "m2/component/Physique.h"
#include "m2/component/Graphic.h"
#include "m2/component/Light.h"

m2::Object::Object(const m2::VecF &position, m2g::pb::ObjectType type, ObjectId parent_id) : position(position),
		_object_type(type), _parent_id(parent_id) {}

m2::Object::Object(Object&& other) noexcept :
		position(other.position),
		impl(std::move(other.impl)),
		_object_type(other._object_type),
		_parent_id(other._parent_id),
		_group_id(other._group_id),
		_index_in_group(other._index_in_group),
		_physique_id(other._physique_id),
		_graphic_id(other._graphic_id),
		_terrain_graphic_id(other._terrain_graphic_id),
		_light_id(other._light_id),
		_sound_emitter_id(other._sound_emitter_id),
		_character_id(other._character_id) {
	other._group_id = {};
	other._object_type = {};
	other._parent_id = 0;
	other._index_in_group = 0;
	other._physique_id = 0;
	other._graphic_id = 0;
	other._terrain_graphic_id = {};
	other._light_id = 0;
	other._sound_emitter_id = 0;
    other._character_id = 0;
}
m2::Object& m2::Object::operator=(Object&& other) noexcept {
	std::swap(position, other.position);
	std::swap(impl, other.impl);
	std::swap(_object_type, other._object_type);
	std::swap(_parent_id, other._parent_id);
	std::swap(_group_id, other._group_id);
	std::swap(_index_in_group, other._index_in_group);
	std::swap(_physique_id, other._physique_id);
	std::swap(_graphic_id, other._graphic_id);
	std::swap(_terrain_graphic_id, other._terrain_graphic_id);
	std::swap(_light_id, other._light_id);
	std::swap(_sound_emitter_id, other._sound_emitter_id);
	std::swap(_character_id, other._character_id);
	return *this;
}

m2::Object::~Object() {
	if (_group_id) {
		M2_LEVEL.groups[_group_id]->RemoveMember(_index_in_group);
	}
	RemovePhysique();
	RemoveGraphic();
	RemoveTerrainGraphic();
	RemoveLight();
	RemoveSoundEmitter();
	RemoveCharacter();
}

m2::ObjectId m2::Object::GetId() const {
	if (!_id) {
		// Looking up the id of the object itself is not very common
		_id = M2_LEVEL.objects.GetId(this);
	}
	return *_id;
}
m2::ObjectId m2::Object::GetParentId() const {
    return _parent_id;
}
m2::GroupId m2::Object::GetGroupId() const {
	return _group_id;
}
m2::PhysiqueId m2::Object::GetPhysiqueId() const {
	return _physique_id;
}
m2::GraphicId m2::Object::GetGraphicId() const {
	return _graphic_id;
}
std::pair<m2::GraphicId, m2::BackgroundLayer> m2::Object::GetTerrainGraphicId() const {
	return _terrain_graphic_id;
}
m2::LightId m2::Object::GetLightId() const {
	return _light_id;
}
m2::SoundEmitterId m2::Object::GetSoundId() const {
	return _sound_emitter_id;
}
m2::CharacterId m2::Object::GetCharacterId() const {
    return _character_id;
}

m2::Character* m2::Object::TryGetCharacter() const {
	auto* character_variant = M2_LEVEL.characters.Get(_character_id);
	if (not character_variant) {
		return nullptr;
	}
	return &ToCharacterBase(*character_variant);
}
m2::Object* m2::Object::TryGetParent() const {
    return _parent_id ? M2_LEVEL.objects.Get(_parent_id) : nullptr;
}
m2::Group* m2::Object::TryGetGroup() const {
	return _group_id ? M2_LEVEL.groups[_group_id].get() : nullptr;
}

m2::Physique& m2::Object::GetPhysique() const {
	return M2_LEVEL.physics[_physique_id];
}
m2::Graphic& m2::Object::GetGraphic() const {
	return M2_LEVEL.fgGraphics[_graphic_id];
}
m2::Graphic& m2::Object::GetTerrainGraphic() const {
	return M2_LEVEL.bgGraphics[I(_terrain_graphic_id.second)][_terrain_graphic_id.first];
}
m2::Light& m2::Object::GetLight() const {
	return M2_LEVEL.lights[_light_id];
}
m2::SoundEmitter& m2::Object::GetSoundEmitter() const {
	return M2_LEVEL.soundEmitters[_sound_emitter_id];
}
m2::Character& m2::Object::GetCharacter() const {
    auto& it = M2_LEVEL.characters[_character_id];
    return ToCharacterBase(it);
}

void m2::Object::SetGroup(const GroupId& group_id, IndexInGroup group_index) {
	_group_id = group_id;
	_index_in_group = group_index;
}

m2::Physique& m2::Object::AddPhysique() {
	auto phy = M2_LEVEL.physics.Emplace(GetId());
	_physique_id = phy.GetId();
    LOG_TRACE("Added physique component", _physique_id);
	return *phy;
}
m2::Graphic& m2::Object::AddGraphic() {
	auto gfx = M2_LEVEL.fgGraphics.Emplace(GetId());
	_graphic_id = gfx.GetId();
	M2_LEVEL.drawList[I(ForegroundLayer::F0)].Insert(GetId());
    LOG_TRACE("Added graphic component", _graphic_id);
	return *gfx;
}
m2::Graphic& m2::Object::AddGraphic(const m2g::pb::SpriteType spriteType) {
	auto gfx = M2_LEVEL.fgGraphics.Emplace(GetId(), M2_GAME.GetSpriteOrTextLabel(spriteType));
	_graphic_id = gfx.GetId();
	M2_LEVEL.drawList[I(ForegroundLayer::F0)].Insert(GetId());
	LOG_TRACE("Added graphic component", _graphic_id);
	return *gfx;
}
m2::Graphic& m2::Object::AddTerrainGraphic(BackgroundLayer layer) {
	auto terrain_gfx = M2_LEVEL.bgGraphics[I(layer)].Emplace(GetId());
	_terrain_graphic_id = std::make_pair(terrain_gfx.GetId(), layer);
	LOG_TRACE("Added terrain graphic component", _terrain_graphic_id);
	return *terrain_gfx;
}
m2::Graphic& m2::Object::AddTerrainGraphic(BackgroundLayer layer, const m2g::pb::SpriteType spriteType) {
	auto terrain_gfx = M2_LEVEL.bgGraphics[I(layer)].Emplace(GetId(), M2_GAME.GetSpriteOrTextLabel(spriteType));
	_terrain_graphic_id = std::make_pair(terrain_gfx.GetId(), layer);
	LOG_TRACE("Added terrain graphic component", _terrain_graphic_id);
	return *terrain_gfx;
}
m2::Light& m2::Object::AddLight() {
	auto light = M2_LEVEL.lights.Emplace(GetId());
	_light_id = light.GetId();
    LOG_TRACE("Added light component", _light_id);
	return *light;
}
m2::SoundEmitter& m2::Object::AddSoundEmitter() {
	auto sound = M2_LEVEL.soundEmitters.Emplace(GetId());
	_sound_emitter_id = sound.GetId();
	LOG_TRACE("Added sound component", _sound_emitter_id);
	return *sound;
}
m2::Character& m2::Object::AddTinyCharacter() {
    auto character = M2_LEVEL.characters.Emplace(std::in_place_type<TinyCharacter>, GetId());
    _character_id = character.GetId();
    LOG_TRACE("Added tiny character", _character_id);
    return std::get<TinyCharacter>(*character);
}
m2::Character& m2::Object::AddFullCharacter() {
    auto character = M2_LEVEL.characters.Emplace(std::in_place_type<FullCharacter>, GetId());
    _character_id = character.GetId();
    LOG_TRACE("Added full character", _character_id);
    return std::get<FullCharacter>(*character);
}

void m2::Object::MoveToBackgroundLayer(BackgroundLayer bl) {
	throw M2_ERROR("Not yet implemented");
}
void m2::Object::MoveToForegroundLayer(ForegroundLayer fl) {
	throw M2_ERROR("Not yet implemented");
}

void m2::Object::RemovePhysique() {
	if (_physique_id) {
		M2_LEVEL.physics.Free(_physique_id);
		_physique_id = 0;
	}
}
void m2::Object::RemoveGraphic() {
	if (_graphic_id) {
		M2_LEVEL.drawList[I(ForegroundLayer::F0)].Remove(GetId());
		M2_LEVEL.fgGraphics.Free(_graphic_id);
		_graphic_id = 0;
	}
}
void m2::Object::RemoveTerrainGraphic() {
	if (_terrain_graphic_id.first) {
		M2_LEVEL.bgGraphics[I(_terrain_graphic_id.second)].Free(_terrain_graphic_id.first);
		_terrain_graphic_id = {};
	}
}
void m2::Object::RemoveLight() {
	if (_light_id) {
		M2_LEVEL.lights.Free(_light_id);
		_light_id = 0;
	}
}
void m2::Object::RemoveSoundEmitter() {
	if (_sound_emitter_id) {
		M2_LEVEL.soundEmitters.Free(_sound_emitter_id);
		_sound_emitter_id = 0;
	}
}
void m2::Object::RemoveCharacter() {
	if (_character_id) {
		M2_LEVEL.characters.Free(_character_id);
		_character_id = 0;
	}
}

m2::Pool<m2::Object>::Iterator m2::CreateObject(const m2::VecF &position, m2g::pb::ObjectType type, ObjectId parent_id) {
    return M2_LEVEL.objects.Emplace(position, type, parent_id);
}
std::function<void(void)> m2::CreateObjectDeleter(ObjectId id) {
	return [id]() {
		M2_LEVEL.objects.Free(id);
	};
}
std::function<void(void)> m2::CreatePhysiqueDeleter(ObjectId id) {
	return [id]() {
		if (auto* object = M2_LEVEL.objects.Get(id); object) {
			object->RemovePhysique();
		}
	};
}
std::function<void(void)> m2::CreateGraphicDeleter(ObjectId id) {
	return [id]() {
		if (auto* object = M2_LEVEL.objects.Get(id); object) {
			object->RemoveGraphic();
		}
	};
}
std::function<void(void)> m2::CreateTerrainGraphicDeleter(ObjectId id) {
	return [id]() {
		if (auto* object = M2_LEVEL.objects.Get(id); object) {
			object->RemoveTerrainGraphic();
		}
	};
}
std::function<void(void)> m2::CreateLightDeleter(ObjectId id) {
	return [id]() {
		if (auto* object = M2_LEVEL.objects.Get(id); object) {
			object->RemoveLight();
		}
	};
}
std::function<void(void)> m2::CreateSoundEmitterDeleter(ObjectId id) {
	return [id]() {
		if (auto* object = M2_LEVEL.objects.Get(id); object) {
			object->RemoveSoundEmitter();
		}
	};
}
std::function<void(void)> m2::CreateCharacterDeleter(ObjectId id) {
	return [id]() {
		if (auto* object = M2_LEVEL.objects.Get(id); object) {
			object->RemoveCharacter();
		}
	};
}

std::function<bool(m2::Object&)> m2::is_object_in_area(const RectF& rect) {
	return [rect](const Object& o) -> bool {
		return rect.contains(o.position);
	};
}

m2::Object& m2::to_object_of_id(ObjectId id) {
	return M2_LEVEL.objects[id];
}
