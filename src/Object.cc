#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/component/Monitor.h"
#include "m2/component/Physique.h"
#include "m2/component/Graphic.h"
#include "m2/component/Light.h"

m2::Object::Object(const m2::Vec2f &position) :
	position(position),
	_group_id(),
	_group_index(0),
	_monitor_id(0),
	_physique_id(0),
	_graphic_id(0),
	_terrain_graphic_id(0),
	_light_id(0),
	_defense_id(0),
	_offense_id(0) {}

m2::Object::Object(Object&& other) noexcept :
	position(other.position),
	impl(std::move(other.impl)),
	_group_id(other._group_id),
	_group_index(other._group_index),
	_monitor_id(other._monitor_id),
	_physique_id(other._physique_id),
	_graphic_id(other._graphic_id),
	_terrain_graphic_id(other._terrain_graphic_id),
	_light_id(other._light_id),
	_defense_id(other._defense_id),
	_offense_id(other._offense_id) {
	other._group_id = {};
	other._group_index = 0;
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
	std::swap(impl, other.impl);
	std::swap(_group_id, other._group_id);
	std::swap(_group_index, other._group_index);
	std::swap(_monitor_id, other._monitor_id);
	std::swap(_physique_id, other._physique_id);
	std::swap(_graphic_id, other._graphic_id);
	std::swap(_terrain_graphic_id, other._terrain_graphic_id);
	std::swap(_light_id, other._light_id);
	std::swap(_defense_id, other._defense_id);
	std::swap(_offense_id, other._offense_id);
	return *this;
}

m2::Object::~Object() {
	auto id = GAME.objects.get_id(this);
	remove_from_group();
	if (_monitor_id) {
		GAME.monitors.free(_monitor_id);
		_monitor_id = 0;
	}
	if (_physique_id) {
		GAME.physics.free(_physique_id);
		_physique_id = 0;
	}
	if (_graphic_id) {
		GAME.draw_list.remove(id);
		GAME.graphics.free(_graphic_id);
		_graphic_id = 0;
	}
	if (_terrain_graphic_id) {
		GAME.terrainGraphics.free(_terrain_graphic_id);
		_terrain_graphic_id = 0;
	}
	if (_defense_id) {
		GAME.defenses.free(_defense_id);
		_defense_id = 0;
	}
	if (_offense_id) {
		GAME.offenses.free(_offense_id);
		_offense_id = 0;
	}
}

m2::GroupID m2::Object::group_id() const {
	return _group_id;
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

m2::Group& m2::Object::group() const {
	return *GAME.groups[_group_id];
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

void m2::Object::add_to_group(const pb::GroupBlueprint& group, const std::function<std::unique_ptr<Group>()>& group_initializer) {
	auto group_id = GroupID{group};
	auto it = GAME.groups.find(group_id);
	if (it == GAME.groups.end()) {
		it = GAME.groups.insert({group_id, group_initializer()}).first;
	}
	_group_index = it->second->add_member(GAME.objects.get_id(this));
	_group_id = group_id;
}
m2::comp::Monitor& m2::Object::add_monitor() {
	auto monitor_pair = GAME.monitors.alloc();
	_monitor_id = monitor_pair.second;
	monitor_pair.first = comp::Monitor{GAME.objects.get_id(this)};
	return monitor_pair.first;
}
m2::comp::Monitor& m2::Object::add_monitor(const comp::Monitor::Callback& pre_phy) {
	auto monitor_pair = GAME.monitors.alloc();
	_monitor_id = monitor_pair.second;
	monitor_pair.first = comp::Monitor{GAME.objects.get_id(this), pre_phy, {}, {}, {}};
	return monitor_pair.first;
}
m2::comp::Monitor& m2::Object::add_monitor(const comp::Monitor::Callback& pre_phy, const comp::Monitor::Callback& pre_gfx) {
	auto monitor_pair = GAME.monitors.alloc();
	_monitor_id = monitor_pair.second;
	monitor_pair.first = comp::Monitor{GAME.objects.get_id(this), pre_phy, {}, pre_gfx, {}};
	return monitor_pair.first;
}
m2::comp::Monitor& m2::Object::add_monitor(const comp::Monitor::Callback& pre_phy, const comp::Monitor::Callback& post_phy, const comp::Monitor::Callback& pre_gfx) {
	auto monitor_pair = GAME.monitors.alloc();
	_monitor_id = monitor_pair.second;
	monitor_pair.first = comp::Monitor{GAME.objects.get_id(this), pre_phy, post_phy, pre_gfx, {}};
	return monitor_pair.first;
}
m2::comp::Monitor& m2::Object::add_monitor(const comp::Monitor::Callback& pre_phy, const comp::Monitor::Callback& post_phy, const comp::Monitor::Callback& pre_gfx, const comp::Monitor::Callback& post_gfx) {
	auto monitor_pair = GAME.monitors.alloc();
	_monitor_id = monitor_pair.second;
	monitor_pair.first = comp::Monitor{GAME.objects.get_id(this), pre_phy, post_phy, pre_gfx, post_gfx};
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
m2::comp::Graphic& m2::Object::add_graphic(const Sprite& sprite) {
	auto graphic_pair = GAME.graphics.alloc();
	_graphic_id = graphic_pair.second;
	auto obj_id = GAME.objects.get_id(this);
	graphic_pair.first = comp::Graphic{obj_id, sprite};
	GAME.draw_list.insert(obj_id);
	return graphic_pair.first;
}
m2::comp::Graphic& m2::Object::add_terrain_graphic(const Sprite& sprite) {
	auto terrain_graphic_pair = GAME.terrainGraphics.alloc();
	_terrain_graphic_id = terrain_graphic_pair.second;
	terrain_graphic_pair.first = comp::Graphic{GAME.objects.get_id(this), sprite};
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

void m2::Object::remove_from_group() {
	if (_group_id.type) {
		GAME.groups[_group_id]->remove_member(_group_index);
		if (GAME.groups[_group_id]->members().size() == 0) {
			GAME.groups.erase(_group_id);
		}
		_group_id = {};
		_group_index = 0;
	}
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
