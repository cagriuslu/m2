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

	obj.add_graphic(GAME.sprites[m2g::pb::SpriteType::DWARF_FULL]);

	obj.add_monitor([&](m2::Monitor& mon) {
        // Character movement
		auto [direction_enum, direction_vector] = m2::calculate_character_movement(m2::Key::LEFT, m2::Key::RIGHT, m2::Key::UNKNOWN, m2::Key::UNKNOWN);
		auto x_velocity = phy.body->GetLinearVelocity().x;
		auto velocity_limit = 5.0f; // Unit is unknown
		auto limiting_func = logf(velocity_limit - fabs(x_velocity));
		auto force = limiting_func * 2500.0f;
		phy.body->ApplyForceToCenter(b2Vec2{direction_vector * force}, true);

        // Mouse button
        if (GAME.events.is_mouse_button_down(m2::MouseButton::PRIMARY)) {
            if (GAME.mousePositionWRTGameWorld_m.is_near(obj.position, 2.0f)) {
                fprintf(stderr, "Mouse is close\n");

                // Look for objects which have Defence component
                m2::box2d::query(*GAME.world, m2::Aabb2f{GAME.mousePositionWRTGameWorld_m, 0.1f}, [](m2::Physique& other_phy) -> bool {
                    fprintf(stderr, "Query found an object at %llu %f,%f\n", other_phy.parent().id(), other_phy.parent().position.x, other_phy.parent().position.y);

                    if (other_phy.parent().defense_id()) {
                        // Found an object with defense, stop the search
                        fprintf(stderr, "Mouse is close to a defense object\n");
                        return true;
                    }
                    return false;
                });
            }
        }
	});

	GAME.playerId = obj.id();
	return {};
}
