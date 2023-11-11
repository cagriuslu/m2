#pragma once
#include <Item.pb.h>
#include <m2g_ItemType.pb.h>
#include <m2/Meta.h>
#include <m2/Object.h>
#include <m2g_SpriteType.pb.h>
#include <m2/sdl/Detail.hh>

namespace rpg {
	/// Represents the blade object that's created by referring to the melee_weapon.
	m2::void_expected create_blade(m2::Object& obj, const m2::VecF& direction, const m2::Item& melee_weapon, bool is_friend);

	m2::void_expected create_corpse(m2::Object& obj, m2g::pb::SpriteType);

	m2::void_expected create_decoration(m2::Object& obj, m2g::pb::SpriteType sprite_type);

	m2::void_expected create_dropped_item(m2::Object& obj, m2g::pb::ItemType item_type);

	m2::void_expected init_finish_point(m2::Object& obj, m2g::pb::ObjectType& type);

	m2::void_expected create_projectile(m2::Object& obj, const m2::VecF& intended_direction, const m2::Item& ranged_weapon, bool is_friend);

	m2::void_expected create_spikes(m2::Object& obj);
}
