#include <m2/third_party/physics/ColliderCategory.h>
#include <m2g/Proxy.h>
#include <mine/object/Dwarf.h>
#include <mine/object/Blacksmith.h>

using namespace m2::third_party::physics;

FixtureDefinition m2g::Proxy::TileFixtureDefinition(MAYBE pb::SpriteType spriteType) {
	return FixtureDefinition{
		.colliderFilter = gColliderCategoryToParams[m2::I(ColliderCategory::COLLIDER_CATEGORY_OBSTACLE)]
	};
}

void m2g::Proxy::post_tile_create(m2::Object& obj, m2g::pb::SpriteType sprite_type) {
	switch (sprite_type) {
		case pb::SpriteType::DUNGEON_COAL_1:
		case pb::SpriteType::DUNGEON_COAL_2: {
			// Add HP to destroyable tiles
			auto& chr = obj.AddCompactCharacter();
			chr.AddResource(m2g::pb::RESOURCE_HP, 2.0f);
			chr.SetMaxResource(m2g::pb::RESOURCE_HP, 2.0f);
			break;
		}
		case pb::SpriteType::GRASSLAND_DIRT_1:
		case pb::SpriteType::GRASSLAND_DIRT_2: {
			// Add HP to destroyable tiles
			auto& chr = obj.AddCompactCharacter();
			chr.AddResource(m2g::pb::RESOURCE_HP, 1.0f);
			chr.SetMaxResource(m2g::pb::RESOURCE_HP, 1.0f);
			break;
		}
		default:
			break;
	}
}

m2::void_expected m2g::Proxy::LoadForegroundObjectFromLevelBlueprint(m2::Object& obj, const m2::VecF& position, float orientation) {
	switch (obj.GetType()) {
		case pb::ObjectType::DWARF:
			return create_dwarf(obj, position);
		case pb::ObjectType::BLACKSMITH:
			return create_blacksmith(obj, position);
		default:
			return m2::make_unexpected("Invalid object type");
	}
}
