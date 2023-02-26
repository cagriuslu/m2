#include <m2g/Object.h>
#include <mine/object/Dwarf.h>
#include <mine/object/Blacksmith.h>
#include <ResourceType.pb.h>

using namespace m2g;
using namespace m2g::pb;

void m2g::post_tile_create(m2::Object& obj, pb::SpriteType sprite_type) {
    switch (sprite_type) {
		case pb::SpriteType::DUNGEON_COAL_1:
		case pb::SpriteType::DUNGEON_COAL_2: {
			// Add HP to destroyable tiles
			auto& chr = obj.add_tiny_character();
			chr.add_resource(RESOURCE_HP, 2.0f);
			chr.set_max_resource(RESOURCE_HP, 2.0f);
			break;
		}
        case pb::SpriteType::GRASSLAND_DIRT_1:
        case pb::SpriteType::GRASSLAND_DIRT_2: {
			// Add HP to destroyable tiles
			auto& chr = obj.add_tiny_character();
			chr.add_resource(RESOURCE_HP, 1.0f);
			chr.set_max_resource(RESOURCE_HP, 1.0f);
            break;
        }
        default:
            break;
    }
}

m2::VoidValue m2g::fg_object_loader(m2::Object& obj, pb::ObjectType object_type) {
	switch (object_type) {
		case pb::ObjectType::DWARF:
			return create_dwarf(obj);
		case pb::ObjectType::BLACKSMITH:
			return create_blacksmith(obj);
		default:
			return m2::failure("Invalid object type");
	}
}
