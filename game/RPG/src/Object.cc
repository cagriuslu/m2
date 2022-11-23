#include <m2g/Object.h>
#include <rpg/object/Enemy.h>
#include <rpg/object/Player.h>

void m2g::post_tile_create(m2::Object& obj, pb::SpriteType sprite_type) {}

m2::VoidValue m2g::fg_object_loader(m2::Object& obj, pb::ObjectType object_type) {
	using namespace obj;
	switch (object_type) {
		case pb::ObjectType::PLAYER:
			return Player::init(obj, &chr::character_player);
		case pb::ObjectType::SKELETON:
			return Enemy::init(obj, &chr::character_skeleton_000_chase);
		default:
			return m2::failure("Invalid sprite index");
	}
}
