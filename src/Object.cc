#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/component/Monitor.h"
#include "m2/component/Physique.h"
#include "m2/component/Graphic.h"
#include "m2/component/Light.h"

m2::Object::Object(const m2::Vec2f &position) :
	position(position),
	_monitor_id(0),
	_physique_id(0),
	_graphic_id(0),
	_terrain_graphic_id(0),
	_light_id(0),
	_defense_id(0),
	_offense_id(0) {}

m2::Object::Object(Object&& other) noexcept :
	position(other.position),
	_monitor_id(other._monitor_id),
	_physique_id(other._physique_id),
	_graphic_id(other._graphic_id),
	_terrain_graphic_id(other._terrain_graphic_id),
	_light_id(other._light_id),
	_defense_id(other._defense_id),
	_offense_id(other._offense_id),
	impl(std::move(other.impl)) {
	other._monitor_id = 0;
	other._physique_id = 0;
	other._graphic_id = 0;
	other._terrain_graphic_id = 0;
	other._light_id = 0;
	other._defense_id = 0;
	other._offense_id = 0;
}
m2::Object& m2::Object::operator=(Object&& other) noexcept {
	std::swap(position, other.position);
	std::swap(_monitor_id, other._monitor_id);
	std::swap(_physique_id, other._physique_id);
	std::swap(_graphic_id, other._graphic_id);
	std::swap(_terrain_graphic_id, other._terrain_graphic_id);
	std::swap(_light_id, other._light_id);
	std::swap(_defense_id, other._defense_id);
	std::swap(_offense_id, other._offense_id);
	std::swap(impl, other.impl);
	return *this;
}

m2::Object::~Object() {
	if (_monitor_id) {
		GAME.monitors.free(_monitor_id);
	}
	if (_physique_id) {
		GAME.physics.free(_physique_id);
	}
	if (_graphic_id) {
		GAME.draw_list.remove(GAME.objects.get_id(this));
		GAME.graphics.free(_graphic_id);
	}
	if (_terrain_graphic_id) {
		GAME.terrainGraphics.free(_terrain_graphic_id);
	}
	if (_defense_id) {
		GAME.defenses.free(_defense_id);
	}
	if (_offense_id) {
		GAME.offenses.free(_offense_id);
	}
}

m2::MonitorID m2::Object::monitor_id() const {
	return _monitor_id;
}
m2::PhysiqueID m2::Object::physique_id() const {
	return _physique_id;
}
m2::GraphicID m2::Object::graphic_id() const {
	return _graphic_id;
}
m2::GraphicID m2::Object::terrain_graphic_id() const {
	return _terrain_graphic_id;
}
m2::LightID m2::Object::light_id() const {
	return _light_id;
}
m2::DefenseID m2::Object::defense_id() const {
	return _defense_id;
}
m2::OffenseID m2::Object::offense_id() const {
	return _offense_id;
}

m2::comp::Monitor& m2::Object::monitor() const {
	return GAME.monitors[_monitor_id];
}
m2::comp::Physique& m2::Object::physique() const {
	return GAME.physics[_physique_id];
}
m2::comp::Graphic& m2::Object::graphic() const {
	return GAME.graphics[_graphic_id];
}
m2::comp::Graphic& m2::Object::terrain_graphic() const {
	return GAME.terrainGraphics[_terrain_graphic_id];
}
m2::comp::Light& m2::Object::light() const {
	return GAME.lights[_light_id];
}
m2g::comp::Defense& m2::Object::defense() const {
	return GAME.defenses[_defense_id];
}
m2g::comp::Offense& m2::Object::offense() const {
	return GAME.offenses[_offense_id];
}

m2::comp::Monitor& m2::Object::add_monitor() {
	auto monitor_pair = GAME.monitors.alloc();
	_monitor_id = monitor_pair.second;
	monitor_pair.first = comp::Monitor{GAME.objects.get_id(this)};
	return monitor_pair.first;
}
m2::comp::Physique& m2::Object::add_physique() {
	auto physique_pair = GAME.physics.alloc();
	_physique_id = physique_pair.second;
	physique_pair.first = comp::Physique{GAME.objects.get_id(this)};
	return physique_pair.first;
}
m2::comp::Graphic& m2::Object::add_graphic() {
	auto graphic_pair = GAME.graphics.alloc();
	_graphic_id = graphic_pair.second;

	auto obj_id = GAME.objects.get_id(this);
	graphic_pair.first = comp::Graphic{obj_id};
	GAME.draw_list.insert(obj_id);
	return graphic_pair.first;
}
m2::comp::Graphic& m2::Object::add_terrain_graphic() {
	auto terrain_graphic_pair = GAME.terrainGraphics.alloc();
	_terrain_graphic_id = terrain_graphic_pair.second;
	terrain_graphic_pair.first = comp::Graphic{GAME.objects.get_id(this)};
	return terrain_graphic_pair.first;
}
m2::comp::Light& m2::Object::add_light() {
	auto light_pair = GAME.lights.alloc();
	_light_id = light_pair.second;
	light_pair.first = comp::Light{GAME.objects.get_id(this)};
	return light_pair.first;
}
m2g::comp::Defense& m2::Object::add_defense() {
	auto defense_pair = GAME.defenses.alloc();
	_defense_id = defense_pair.second;
	defense_pair.first = m2g::comp::Defense{GAME.objects.get_id(this)};
	return defense_pair.first;
}
m2g::comp::Offense& m2::Object::add_offense() {
	auto offense_pair = GAME.offenses.alloc();
	_offense_id = offense_pair.second;
	offense_pair.first = m2g::comp::Offense{GAME.objects.get_id(this)};
	return offense_pair.first;
}

std::pair<m2::Object&, m2::ObjectID> m2::create_object(const m2::Vec2f &position) {
    auto obj_pair = GAME.objects.alloc();
    obj_pair.first = Object{position};
    return obj_pair;
}

std::function<void(void)> m2::create_object_deleter(ObjectID id) {
	return [id]() {
		GAME.objects.free(id);
	};
}
