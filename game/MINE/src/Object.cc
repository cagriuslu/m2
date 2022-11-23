#include <m2g/Object.h>
#include <mine/object/Dwarf.h>

void m2g::post_tile_create(m2::Object& obj, pb::SpriteType sprite_type) {
    switch (sprite_type) {
        case pb::SpriteType::GRASSLAND_DIRT_1:
        case pb::SpriteType::GRASSLAND_DIRT_2: {
            auto& def = obj.add_defense();
            fprintf(stderr, "Added defense component to %llu %f, %f\n", def.parent().id(), obj.position.x, obj.position.y);
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
		default:
			return m2::failure("Invalid sprite index");
	}
}
