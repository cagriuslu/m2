#include <m2g/Proxy.h>
#include <mine/object/Dwarf.h>
#include <mine/object/Blacksmith.h>

void m2g::Proxy::post_tile_create(m2::Object& obj, m2g::pb::SpriteType sprite_type) {
	switch (sprite_type) {
		case pb::SpriteType::DUNGEON_COAL_1:
		case pb::SpriteType::DUNGEON_COAL_2: {
			// Add HP to destroyable tiles
			auto& chr = obj.add_tiny_character();
			chr.add_resource(m2g::pb::RESOURCE_HP, 2.0f);
			chr.set_max_resource(m2g::pb::RESOURCE_HP, 2.0f);
			break;
		}
		case pb::SpriteType::GRASSLAND_DIRT_1:
		case pb::SpriteType::GRASSLAND_DIRT_2: {
			// Add HP to destroyable tiles
			auto& chr = obj.add_tiny_character();
			chr.add_resource(m2g::pb::RESOURCE_HP, 1.0f);
			chr.set_max_resource(m2g::pb::RESOURCE_HP, 1.0f);
			break;
		}
		default:
			break;
	}
}

m2::void_expected m2g::Proxy::LoadForegroundObjectFromLevelBlueprint(m2::Object& obj) {
	switch (obj.object_type()) {
		case pb::ObjectType::DWARF:
			return create_dwarf(obj);
		case pb::ObjectType::BLACKSMITH:
			return create_blacksmith(obj);
		default:
			return m2::make_unexpected("Invalid object type");
	}
}
