#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/component/Monitor.h"
#include "m2/component/Physique.h"
#include "m2/component/Graphic.h"
#include "m2/component/Light.h"

m2::Object::Object(const m2::Vec2f &position) :
		position(position),
		monitor_id(0),
		physique_id(0),
		graphic_id(0),
		terrain_graphic_id(0),
		light_id(0),
		defense_id(0),
		offense_id(0) {}

m2::Object::Object(Object&& other) noexcept :
        position(other.position),
        monitor_id(other.monitor_id),
        physique_id(other.physique_id),
        graphic_id(other.graphic_id),
        terrain_graphic_id(other.terrain_graphic_id),
        light_id(other.light_id),
        defense_id(other.defense_id),
        offense_id(other.offense_id),
        impl(std::move(other.impl)) {
	other.monitor_id = 0;
	other.physique_id = 0;
	other.graphic_id = 0;
	other.terrain_graphic_id = 0;
	other.light_id = 0;
	other.defense_id = 0;
	other.offense_id = 0;
}
m2::Object& m2::Object::operator=(Object&& other) noexcept {
	std::swap(position, other.position);
	std::swap(monitor_id, other.monitor_id);
	std::swap(physique_id, other.physique_id);
	std::swap(graphic_id, other.graphic_id);
	std::swap(terrain_graphic_id, other.terrain_graphic_id);
	std::swap(light_id, other.light_id);
	std::swap(defense_id, other.defense_id);
	std::swap(offense_id, other.offense_id);
	std::swap(impl, other.impl);
	return *this;
}

m2::Object::~Object() {
	if (monitor_id) {
		GAME.monitors.free(monitor_id);
	}
	if (physique_id) {
		GAME.physics.free(physique_id);
	}
	if (graphic_id) {
		GAME.draw_list.remove(GAME.objects.get_id(this));
		GAME.graphics.free(graphic_id);
	}
	if (terrain_graphic_id) {
		GAME.terrainGraphics.free(terrain_graphic_id);
	}
	if (defense_id) {
		GAME.defenses.free(defense_id);
	}
	if (offense_id) {
		GAME.offenses.free(offense_id);
	}
}

m2::comp::Monitor& m2::Object::monitor() const {
	return GAME.monitors[monitor_id];
}
m2::comp::Physique& m2::Object::physique() const {
	return GAME.physics[physique_id];
}
m2::comp::Graphic& m2::Object::graphic() const {
	return GAME.graphics[graphic_id];
}
m2::comp::Graphic& m2::Object::terrain_graphic() const {
	return GAME.terrainGraphics[terrain_graphic_id];
}
m2::comp::Light& m2::Object::light() const {
	return GAME.lights[light_id];
}
m2g::comp::Defense& m2::Object::defense() const {
	return GAME.defenses[defense_id];
}
m2g::comp::Offense& m2::Object::offense() const {
	return GAME.offenses[offense_id];
}

m2::comp::Monitor& m2::Object::add_monitor() {
	auto monitor_pair = GAME.monitors.alloc();
	monitor_id = monitor_pair.second;
	monitor_pair.first = comp::Monitor{GAME.objects.get_id(this)};
	return monitor_pair.first;
}
m2::comp::Physique& m2::Object::add_physique() {
	auto physique_pair = GAME.physics.alloc();
	physique_id = physique_pair.second;
	physique_pair.first = comp::Physique{GAME.objects.get_id(this)};
	return physique_pair.first;
}
m2::comp::Graphic& m2::Object::add_graphic() {
	auto graphic_pair = GAME.graphics.alloc();
	graphic_id = graphic_pair.second;

	auto obj_id = GAME.objects.get_id(this);
	graphic_pair.first = comp::Graphic{obj_id};
	GAME.draw_list.insert(obj_id);
	return graphic_pair.first;
}
m2::comp::Graphic& m2::Object::add_terrain_graphic() {
	auto terrain_graphic_pair = GAME.terrainGraphics.alloc();
	terrain_graphic_id = terrain_graphic_pair.second;
	terrain_graphic_pair.first = comp::Graphic{GAME.objects.get_id(this)};
	return terrain_graphic_pair.first;
}
m2::comp::Light& m2::Object::add_light() {
	auto light_pair = GAME.lights.alloc();
	light_id = light_pair.second;
	light_pair.first = comp::Light{GAME.objects.get_id(this)};
	return light_pair.first;
}
m2g::comp::Defense& m2::Object::add_defense() {
	auto defense_pair = GAME.defenses.alloc();
	defense_id = defense_pair.second;
	defense_pair.first = m2g::comp::Defense{GAME.objects.get_id(this)};
	return defense_pair.first;
}
m2g::comp::Offense& m2::Object::add_offense() {
	auto offense_pair = GAME.offenses.alloc();
	offense_id = offense_pair.second;
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
