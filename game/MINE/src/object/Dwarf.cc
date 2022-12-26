#include <mine/object/Dwarf.h>
#include <m2/Game.hh>
#include <SpriteType.pb.h>
#include <m2/box2d/Utils.h>
#include <m2/box2d/Query.h>
#include <m2/game/CharacterMovement.h>

using namespace m2g;
using namespace m2g::pb;

m2::VoidValue create_dwarf(m2::Object& obj) {
	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(GAME.sprites[m2g::pb::DWARF_FULL].background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_friction(2.0f);
	bp.mutable_background_fixture()->set_category(m2::pb::FRIEND_ON_BACKGROUND);
	bp.set_mass(100);
	bp.set_linear_damping(1.0f);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*GAME.world, obj.physique_id(), obj.position, bp);

	obj.add_graphic(GAME.sprites[m2g::pb::SpriteType::DWARF_FULL]);

	auto& chr = obj.add_full_character();
	chr.add_item(GAME.get_item(ITEM_REUSABLE_JUMP));
	chr.add_item(GAME.get_item(ITEM_AUTOMATIC_JUMP_ENERGY));

	phy.pre_step = [&obj, &chr](m2::Physique& phy) {
		// Character movement
		auto [direction_enum, direction_vector] = m2::calculate_character_movement(m2::Key::LEFT, m2::Key::RIGHT, m2::Key::UNKNOWN, m2::Key::UNKNOWN);
		if (direction_enum == m2::CHARMOVEMENT_NONE) {
			// Slow down character
			auto horizontal_velocity = phy.body->GetLinearVelocity().x;
			if (0.001f < abs(horizontal_velocity)) {
				// TODO
			}
		} else {
			// Accelerate character
			auto force_multiplier = m2::calculate_limited_force(phy.body->GetLinearVelocity().x, 5.0f);
			phy.body->ApplyForceToCenter(b2Vec2{direction_vector * force_multiplier * 2500.0f}, true);
		}
		// Jump
		if (GAME.events.is_key_down(m2::Key::DASH) && chr.use_item(chr.find_items(ITEM_REUSABLE_JUMP))) {
			phy.body->ApplyForceToCenter(b2Vec2{0.0f, -50000}, true);
		}

		// Mouse button
		if (GAME.events.is_mouse_button_down(m2::MouseButton::PRIMARY)) {
			m2::box2d::find_objects_near_position_under_mouse(obj.position, 2.0f, [](m2::Physique& other_phy) -> bool {
				auto& obj_under_mouse = other_phy.parent();
				// If object under mouse has character
				if (obj_under_mouse.character_id()) {
					auto& chr_under_mouse = obj_under_mouse.character();
					// If character has HP
					if (chr_under_mouse.has_resource(RESOURCE_HP)) {
						// Damage object
						chr_under_mouse.remove_resource(RESOURCE_HP, 2.0f * GAME.deltaTime_s);
						// Show health bar
						auto hp = chr_under_mouse.get_resource(RESOURCE_HP);
						auto max_hp = chr_under_mouse.get_max_resource(RESOURCE_HP);
						obj_under_mouse.terrain_graphic().draw_effect_health_bar = hp / max_hp;
						// If object under mouse runs out of HP
						if (hp == 0.0f) {
							// Delete object
							GAME.add_deferred_action(m2::create_object_deleter(chr_under_mouse.object_id));
						}
					}
					// Stop searching
					return false;
				}
				// Continue searching
				return true;
			});
		}
	};
	phy.on_collision = [](m2::Physique& phy, m2::Physique& other) {
	};

	GAME.playerId = obj.id();
	return {};
}
