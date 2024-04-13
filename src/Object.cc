#include <m2/Object.h>
#include "m2/Game.h"
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
		LEVEL.groups[_group_id]->remove_member(_index_in_group);
	}
	remove_physique();
	remove_graphic();
	remove_terrain_graphic();
	remove_light();
	remove_sound_emitter();
	remove_character();
}

m2::ObjectId m2::Object::id() const {
	if (!_id) {
		// Looking up the id of the object itself is not very common
		_id = LEVEL.objects.get_id(this);
	}
	return *_id;
}
m2::ObjectId m2::Object::parent_id() const {
    return _parent_id;
}
m2::GroupId m2::Object::group_id() const {
	return _group_id;
}
m2::PhysiqueId m2::Object::physique_id() const {
	return _physique_id;
}
m2::GraphicId m2::Object::graphic_id() const {
	return _graphic_id;
}
std::pair<m2::GraphicId, m2::BackgroundLayer> m2::Object::terrain_graphic_id() const {
	return _terrain_graphic_id;
}
m2::LightId m2::Object::light_id() const {
	return _light_id;
}
m2::SoundEmitterId m2::Object::sound_id() const {
	return _sound_emitter_id;
}
m2::CharacterId m2::Object::character_id() const {
    return _character_id;
}

m2::Character* m2::Object::get_character() const {
	return get_character_base(LEVEL.characters.get(_character_id));
}

m2::Object* m2::Object::parent() const {
    return _parent_id ? LEVEL.objects.get(_parent_id) : nullptr;
}
m2::Group* m2::Object::group() const {
	return _group_id ? LEVEL.groups[_group_id].get() : nullptr;
}
m2::Physique& m2::Object::physique() const {
	return LEVEL.physics[_physique_id];
}
m2::Graphic& m2::Object::graphic() const {
	return LEVEL.graphics[_graphic_id];
}
m2::Graphic& m2::Object::terrain_graphic() const {
	return LEVEL.terrain_graphics[I(_terrain_graphic_id.second)][_terrain_graphic_id.first];
}
m2::Light& m2::Object::light() const {
	return LEVEL.lights[_light_id];
}
m2::SoundEmitter& m2::Object::sound_emitter() const {
	return LEVEL.sound_emitters[_sound_emitter_id];
}
m2::Character& m2::Object::character() const {
    auto& it = LEVEL.characters[_character_id];
    return get_character_base(it);
}

void m2::Object::set_group(const GroupId& group_id, IndexInGroup group_index) {
	_group_id = group_id;
	_index_in_group = group_index;
}

m2::Physique& m2::Object::add_physique() {
	auto physique_pair = LEVEL.physics.alloc();
	_physique_id = physique_pair.second;
	physique_pair.first = Physique{id()};
    LOG_TRACE("Added physique component", _physique_id);
	return physique_pair.first;
}
m2::Graphic& m2::Object::add_graphic() {
	auto graphic_pair = LEVEL.graphics.alloc();
	_graphic_id = graphic_pair.second;
	graphic_pair.first = Graphic{id()};
	LEVEL.draw_list.insert(id());
    LOG_TRACE("Added graphic component", _graphic_id);
	return graphic_pair.first;
}
m2::Graphic& m2::Object::add_graphic(const Sprite& sprite) {
	auto graphic_pair = LEVEL.graphics.alloc();
	_graphic_id = graphic_pair.second;
	graphic_pair.first = Graphic{id(), sprite};
	LEVEL.draw_list.insert(id());
    LOG_TRACE("Added graphic component", _graphic_id);
	return graphic_pair.first;
}
m2::Graphic& m2::Object::add_terrain_graphic(BackgroundLayer layer) {
	auto terrain_graphic_pair = LEVEL.terrain_graphics[I(layer)].alloc();
	_terrain_graphic_id = std::make_pair(terrain_graphic_pair.second, layer);
	terrain_graphic_pair.first = Graphic{id()};
	LOG_TRACE("Added terrain graphic component", _terrain_graphic_id);
	return terrain_graphic_pair.first;
}
m2::Graphic& m2::Object::add_terrain_graphic(BackgroundLayer layer, const Sprite& sprite) {
	auto terrain_graphic_pair = LEVEL.terrain_graphics[I(layer)].alloc();
	_terrain_graphic_id = std::make_pair(terrain_graphic_pair.second, layer);
	terrain_graphic_pair.first = Graphic{id(), sprite};
    LOG_TRACE("Added terrain graphic component", _terrain_graphic_id);
	return terrain_graphic_pair.first;
}
m2::Light& m2::Object::add_light() {
	auto light_pair = LEVEL.lights.alloc();
	_light_id = light_pair.second;
	light_pair.first = Light{id()};
    LOG_TRACE("Added light component", _light_id);
	return light_pair.first;
}
m2::SoundEmitter& m2::Object::add_sound_emitter() {
	auto sound_pair = LEVEL.sound_emitters.alloc();
	_sound_emitter_id = sound_pair.second;
	sound_pair.first = SoundEmitter{id()};
	LOG_TRACE("Added sound component", _sound_emitter_id);
	return sound_pair.first;
}
m2::Character& m2::Object::add_tiny_character() {
    auto character_pair = LEVEL.characters.alloc();
    _character_id = character_pair.second;
    character_pair.first = TinyCharacter{id()};
    LOG_TRACE("Added tiny character", _character_id);
    return std::get<TinyCharacter>(character_pair.first);
}
m2::Character& m2::Object::add_full_character() {
    auto character_pair = LEVEL.characters.alloc();
    _character_id = character_pair.second;
    character_pair.first = FullCharacter{id()};
    LOG_TRACE("Added full character", _character_id);
    return std::get<FullCharacter>(character_pair.first);
}

void m2::Object::remove_physique() {
	if (_physique_id) {
		LEVEL.physics.free(_physique_id);
		_physique_id = 0;
	}
}
void m2::Object::remove_graphic() {
	if (_graphic_id) {
		LEVEL.draw_list.remove(id());
		LEVEL.graphics.free(_graphic_id);
		_graphic_id = 0;
	}
}
void m2::Object::remove_terrain_graphic() {
	if (_terrain_graphic_id.first) {
		LEVEL.terrain_graphics[I(_terrain_graphic_id.second)].free(_terrain_graphic_id.first);
		_terrain_graphic_id = {};
	}
}
void m2::Object::remove_light() {
	if (_light_id) {
		LEVEL.lights.free(_light_id);
		_light_id = 0;
	}
}
void m2::Object::remove_sound_emitter() {
	if (_sound_emitter_id) {
		LEVEL.sound_emitters.free(_sound_emitter_id);
		_sound_emitter_id = 0;
	}
}
void m2::Object::remove_character() {
	if (_character_id) {
		LEVEL.characters.free(_character_id);
		_character_id = 0;
	}
}

std::pair<m2::Object&, m2::ObjectId> m2::create_object(const m2::VecF &position, m2g::pb::ObjectType type, ObjectId parent_id) {
    return LEVEL.objects.alloc(position, type, parent_id);
}
std::function<void(void)> m2::create_object_deleter(ObjectId id) {
	return [id]() {
		LEVEL.objects.free(id);
	};
}
std::function<void(void)> m2::create_physique_deleter(ObjectId id) {
	return [id]() {
		if (auto* object = LEVEL.objects.get(id); object) {
			object->remove_physique();
		}
	};
}
std::function<void(void)> m2::create_graphic_deleter(ObjectId id) {
	return [id]() {
		if (auto* object = LEVEL.objects.get(id); object) {
			object->remove_graphic();
		}
	};
}
std::function<void(void)> m2::create_terrain_graphic_deleter(ObjectId id) {
	return [id]() {
		if (auto* object = LEVEL.objects.get(id); object) {
			object->remove_terrain_graphic();
		}
	};
}
std::function<void(void)> m2::create_light_deleter(ObjectId id) {
	return [id]() {
		if (auto* object = LEVEL.objects.get(id); object) {
			object->remove_light();
		}
	};
}
std::function<void(void)> m2::create_sound_emitter_deleter(ObjectId id) {
	return [id]() {
		if (auto* object = LEVEL.objects.get(id); object) {
			object->remove_sound_emitter();
		}
	};
}
std::function<void(void)> m2::create_character_deleter(ObjectId id) {
	return [id]() {
		if (auto* object = LEVEL.objects.get(id); object) {
			object->remove_character();
		}
	};
}

std::function<bool(m2::Object*)> m2::object_in_rect_filter(const RectF& rect) {
	return [rect](m2::Object* o) -> bool {
		return rect.point_in_rect(o->position);
	};
}
