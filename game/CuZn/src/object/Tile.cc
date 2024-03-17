#include <cuzn/object/Tile.h>
#include <m2/Game.h>

m2::Object* cuzn::find_tile_at_location(m2g::pb::SpriteType location) {
	// Iterate over Character components
	for (const auto& [character, _] : LEVEL.characters) {
		auto& chr = get_character_base(*character);
		// If the character belongs to a tile
		if (chr.parent().object_type() == m2g::pb::TILE) {
			// If the object is located inside the rectangle of the given locatiom
			if (PROXY.industry_positions[location].point_in_rect(chr.parent().position)) {
				return &chr.parent();
			}
		}
	}
	return nullptr;
}
