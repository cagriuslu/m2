#include <m2/SpriteBlueprint.h>
#include <m2/Exception.h>

std::vector<m2::SpriteBlueprint> m2::make_sprite_blueprints(std::vector<m2::SpriteBlueprint>&& val) {
	// Assert that sprite blueprints are zero indexed
	for (unsigned i = 0; i < val.size(); ++i) {
		if (val[i].index != i) {
			throw M2FATAL("Sprite blueprint indexes are not zero indexed");
		}
	}
	return std::move(val);
}
