#include <mine/object/Dwarf.h>
#include <m2/Game.hh>
#include <SpriteType.pb.h>
#include <m2/box2d/Utils.h>

m2::VoidValue create_dwarf(m2::Object& obj) {
	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(GAME.sprites[m2g::pb::DWARF_FULL].background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_category(m2::pb::FRIEND_ON_BACKGROUND);
	bp.set_mass(50);
	bp.set_linear_damping(1);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*GAME.world, obj.physique_id(), obj.position, bp);

	obj.add_graphic(GAME.sprites[m2g::pb::SpriteType::DWARF_FULL]);

	GAME.playerId = obj.id();
	return {};
}
