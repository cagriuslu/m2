#include <mine/object/Dwarf.h>
#include <m2/Game.h>
#include <m2g_SpriteType.pb.h>
#include <m2/box2d/Detail.h>
#include <m2/box2d/Query.h>
#include <m2/game/CharacterMovement.h>

using namespace m2g;
using namespace m2g::pb;

m2::void_expected create_dwarf(m2::Object& obj) {
	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(M2_GAME.GetSprite(m2g::pb::DWARF_FULL).BackgroundColliderCircRadiusM());
	bp.mutable_background_fixture()->set_friction(0.0f);
	bp.mutable_background_fixture()->set_category(m2::pb::FRIEND_ON_BACKGROUND);
	bp.set_mass(100);
	bp.set_gravity_scale(2.0f);
	bp.set_linear_damping(0.0f);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);

	obj.add_graphic(M2_GAME.GetSprite(SpriteType::DWARF_FULL));

	auto& chr = obj.add_full_character();
	chr.add_named_item(M2_GAME.GetNamedItem(ITEM_REUSABLE_JUMP));
	chr.add_named_item(M2_GAME.GetNamedItem(ITEM_AUTOMATIC_JUMP_ENERGY));

	phy.pre_step = [&obj, &chr](m2::Physique& phy) {
		// Character movement
		auto [direction_enum, direction_vector] = m2::calculate_character_movement(m2::Key::LEFT, m2::Key::RIGHT, m2::Key::UNKNOWN, m2::Key::UNKNOWN);
		if (direction_enum == m2::CHARMOVEMENT_NONE) {
			// Slow down character
			auto linear_velocity = phy.body->GetLinearVelocity();
			if (0.0f < abs(linear_velocity.x)) {
				linear_velocity.x /= 1.25f;
				phy.body->SetLinearVelocity(linear_velocity);
			}
		} else {
			// Accelerate character
			auto force_multiplier = m2::calculate_limited_force(phy.body->GetLinearVelocity().x, 5.0f);
			phy.body->ApplyForceToCenter(b2Vec2{direction_vector * force_multiplier * 4000.0f}, true);
		}
		// Jump
		auto is_grounded = chr.get_resource(RESOURCE_IS_GROUNDED_X) != 0.0f && chr.get_resource(RESOURCE_IS_GROUNDED_Y) != 0.0f;
		if (is_grounded && M2_GAME.events.is_key_down(m2::Key::DASH) && chr.use_item(chr.find_items(ITEM_REUSABLE_JUMP))) {
			auto linear_velocity = phy.body->GetLinearVelocity();
			linear_velocity.y -= 7.0f;
			phy.body->SetLinearVelocity(linear_velocity);
		}

		// Mouse button
		if (M2_GAME.events.is_mouse_button_down(m2::MouseButton::PRIMARY)) {
			m2::box2d::find_objects_near_position_under_mouse(obj.position, 2.0f, [](m2::Physique& other_phy) -> bool {
				auto& obj_under_mouse = other_phy.owner();
				// If object under mouse has character
				if (obj_under_mouse.character_id()) {
					auto& chr_under_mouse = obj_under_mouse.character();
					// If character has HP
					if (chr_under_mouse.has_resource(RESOURCE_HP)) {
						// Damage object
						chr_under_mouse.remove_resource(RESOURCE_HP, 2.0f * M2_GAME.DeltaTimeS());
						// Show health bar
						auto hp = chr_under_mouse.get_resource(RESOURCE_HP);
						auto max_hp = chr_under_mouse.get_max_resource(RESOURCE_HP);
						obj_under_mouse.terrain_graphic().draw_addon_health_bar = hp / max_hp;
						// If object under mouse runs out of HP
						if (hp == 0.0f) {
							// Delete object
							M2_DEFER(m2::create_object_deleter(chr_under_mouse.owner_id()));
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
	phy.on_collision = [&chr](MAYBE m2::Physique& phy, m2::Physique& other, const m2::box2d::Contact& contact) {
		// Check if in contact with obstacle
		if (other.body && m2::box2d::has_obstacle(other.body.get())) {
			// Check is contact normal points upwards
			if (abs(contact.normal.x) <= -contact.normal.y) {
				chr.set_resource(RESOURCE_IS_GROUNDED_X, other.owner().position.x);
				chr.set_resource(RESOURCE_IS_GROUNDED_Y, other.owner().position.y);
			}
		}
	};
	phy.off_collision = [&chr](MAYBE m2::Physique& phy, m2::Physique& other) {
		// Check if in contact with obstacle
		if (other.body && m2::box2d::has_obstacle(other.body.get())) {
			// Check if the other object is the grounding object
			if (chr.get_resource(RESOURCE_IS_GROUNDED_X) == other.owner().position.x && chr.get_resource(RESOURCE_IS_GROUNDED_Y) == other.owner().position.y) {
				chr.set_resource(RESOURCE_IS_GROUNDED_X, 0.0f);
				chr.set_resource(RESOURCE_IS_GROUNDED_Y, 0.0f);
			}
		}
	};

	M2_LEVEL.player_id = obj.id();
	return {};
}
