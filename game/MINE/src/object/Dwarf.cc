#include <mine/object/Dwarf.h>
#include <m2/Game.hh>
#include <SpriteType.pb.h>
#include <m2/box2d/Utils.h>
#include <m2/box2d/Query.h>
#include <m2/game/CharacterMovement.h>

m2::VoidValue create_dwarf(m2::Object& obj) {
	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(GAME.sprites[m2g::pb::DWARF_FULL].background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_friction(20.0f);
	bp.mutable_background_fixture()->set_category(m2::pb::FRIEND_ON_BACKGROUND);
	bp.set_mass(100);
	bp.set_linear_damping(1.0f);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*GAME.world, obj.physique_id(), obj.position, bp);
	phy.pre_step = [&obj](m2::Physique& phy) {
		// Character movement
		auto [direction_enum, direction_vector] = m2::calculate_character_movement(m2::Key::LEFT, m2::Key::RIGHT, m2::Key::UNKNOWN, m2::Key::UNKNOWN);
		auto force_multiplier = m2::calculate_limited_force(phy.body->GetLinearVelocity().x, 5.0f);
		phy.body->ApplyForceToCenter(b2Vec2{direction_vector * force_multiplier * 2500.0f}, true);

		// Mouse button
		if (GAME.events.is_mouse_button_down(m2::MouseButton::PRIMARY)) {
			m2::box2d::find_objects_near_position_under_mouse(obj.position, 2.0f, [](m2::Physique& other_phy) -> bool {
				if (other_phy.parent().defense_id()) {
					// Found an object with defense, stop the search
					fprintf(stderr, "Mouse is close to a defense object\n");
					return true;
				}
				return false;
			});
		}
	};

	obj.add_graphic(GAME.sprites[m2g::pb::SpriteType::DWARF_FULL]);

	GAME.playerId = obj.id();
	return {};
}
