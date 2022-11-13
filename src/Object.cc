#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/component/Monitor.h"
#include "m2/component/Physique.h"
#include "m2/component/Graphic.h"
#include "m2/component/Light.h"

m2::Object::Object(const m2::Vec2f &position) : position(position) {}

m2::Object::Object(Object&& other) noexcept :
	position(other.position),
	impl(std::move(other.impl)),
	_group_id(other._group_id),
	_index_in_group(other._index_in_group),
	_monitor_id(other._monitor_id),
	_physique_id(other._physique_id),
	_graphic_id(other._graphic_id),
	_terrain_graphic_id(other._terrain_graphic_id),
	_light_id(other._light_id),
	_defense_id(other._defense_id),
	_offense_id(other._offense_id) {
	other._group_id = {};
	other._index_in_group = 0;
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
	std::swap(_index_in_group, other._index_in_group);
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
	if (_group_id) {
		GAME.groups[_group_id]->remove_member(_index_in_group);
	}
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

m2::ObjectId m2::Object::id() const {
	// Looking up the id of the object itself is not very common
	return GAME.objects.get_id(this);
}
m2::GroupId m2::Object::group_id() const {
	return _group_id;
}
m2::MonitorId m2::Object::monitor_id() const {
	return _monitor_id;
}
m2::PhysiqueId m2::Object::physique_id() const {
	return _physique_id;
}
m2::GraphicId m2::Object::graphic_id() const {
	return _graphic_id;
}
m2::GraphicId m2::Object::terrain_graphic_id() const {
	return _terrain_graphic_id;
}
m2::LightId m2::Object::light_id() const {
	return _light_id;
}
m2::DefenseId m2::Object::defense_id() const {
	return _defense_id;
}
m2::OffenseId m2::Object::offense_id() const {
	return _offense_id;
}

m2::Group* m2::Object::group() const {
	return _group_id ? GAME.groups[_group_id].get() : nullptr;
}
m2::Monitor& m2::Object::monitor() const {
	return GAME.monitors[_monitor_id];
}
m2::Physique& m2::Object::physique() const {
	return GAME.physics[_physique_id];
}
m2::Graphic& m2::Object::graphic() const {
	return GAME.graphics[_graphic_id];
}
m2::Graphic& m2::Object::terrain_graphic() const {
	return GAME.terrainGraphics[_terrain_graphic_id];
}
m2::Light& m2::Object::light() const {
	return GAME.lights[_light_id];
}
m2g::Defense& m2::Object::defense() const {
	return GAME.defenses[_defense_id];
}
m2g::Offense& m2::Object::offense() const {
	return GAME.offenses[_offense_id];
}

void m2::Object::set_group(const GroupId& group_id, IndexInGroup group_index) {
	_group_id = group_id;
	_index_in_group = group_index;
}

m2::Monitor& m2::Object::add_monitor() {
	auto monitor_pair = GAME.monitors.alloc();
	_monitor_id = monitor_pair.second;
	monitor_pair.first = Monitor{GAME.objects.get_id(this)};
	return monitor_pair.first;
}
m2::Monitor& m2::Object::add_monitor(const Monitor::Callback& pre_phy) {
	auto monitor_pair = GAME.monitors.alloc();
	_monitor_id = monitor_pair.second;
	monitor_pair.first = Monitor{GAME.objects.get_id(this), pre_phy, {}, {}, {}};
	return monitor_pair.first;
}
m2::Monitor& m2::Object::add_monitor(const Monitor::Callback& pre_phy, const Monitor::Callback& pre_gfx) {
	auto monitor_pair = GAME.monitors.alloc();
	_monitor_id = monitor_pair.second;
	monitor_pair.first = Monitor{GAME.objects.get_id(this), pre_phy, {}, pre_gfx, {}};
	return monitor_pair.first;
}
m2::Monitor& m2::Object::add_monitor(const Monitor::Callback& pre_phy, const Monitor::Callback& post_phy, const Monitor::Callback& pre_gfx) {
	auto monitor_pair = GAME.monitors.alloc();
	_monitor_id = monitor_pair.second;
	monitor_pair.first = Monitor{GAME.objects.get_id(this), pre_phy, post_phy, pre_gfx, {}};
	return monitor_pair.first;
}
m2::Monitor& m2::Object::add_monitor(const Monitor::Callback& pre_phy, const Monitor::Callback& post_phy, const Monitor::Callback& pre_gfx, const Monitor::Callback& post_gfx) {
	auto monitor_pair = GAME.monitors.alloc();
	_monitor_id = monitor_pair.second;
	monitor_pair.first = Monitor{GAME.objects.get_id(this), pre_phy, post_phy, pre_gfx, post_gfx};
	return monitor_pair.first;
}
m2::Physique& m2::Object::add_physique() {
	auto physique_pair = GAME.physics.alloc();
	_physique_id = physique_pair.second;
	physique_pair.first = Physique{GAME.objects.get_id(this)};
	return physique_pair.first;
}
m2::Graphic& m2::Object::add_graphic() {
	auto graphic_pair = GAME.graphics.alloc();
	_graphic_id = graphic_pair.second;
	auto obj_id = GAME.objects.get_id(this);
	graphic_pair.first = Graphic{obj_id};
	GAME.draw_list.insert(obj_id);
	return graphic_pair.first;
}
m2::Graphic& m2::Object::add_graphic(const Sprite& sprite) {
	auto graphic_pair = GAME.graphics.alloc();
	_graphic_id = graphic_pair.second;
	auto obj_id = GAME.objects.get_id(this);
	graphic_pair.first = Graphic{obj_id, sprite};
	GAME.draw_list.insert(obj_id);
	return graphic_pair.first;
}
m2::Graphic& m2::Object::add_terrain_graphic(const Sprite& sprite) {
	auto terrain_graphic_pair = GAME.terrainGraphics.alloc();
	_terrain_graphic_id = terrain_graphic_pair.second;
	terrain_graphic_pair.first = Graphic{GAME.objects.get_id(this), sprite};
	return terrain_graphic_pair.first;
}
m2::Light& m2::Object::add_light() {
	auto light_pair = GAME.lights.alloc();
	_light_id = light_pair.second;
	light_pair.first = Light{GAME.objects.get_id(this)};
	return light_pair.first;
}
m2g::Defense& m2::Object::add_defense() {
	auto defense_pair = GAME.defenses.alloc();
	_defense_id = defense_pair.second;
	defense_pair.first = m2g::Defense{GAME.objects.get_id(this)};
	return defense_pair.first;
}
m2g::Offense& m2::Object::add_offense() {
	auto offense_pair = GAME.offenses.alloc();
	_offense_id = offense_pair.second;
	offense_pair.first = m2g::Offense{GAME.objects.get_id(this)};
	return offense_pair.first;
}

std::pair<m2::Object&, m2::ObjectId> m2::create_object(const m2::Vec2f &position) {
    return GAME.objects.alloc(position);
}

std::function<void(void)> m2::create_object_deleter(ObjectId id) {
	return [id]() {
		GAME.objects.free(id);
	};
}
