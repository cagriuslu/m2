#include <m2/game/DynamicGridLinesLoader.h>
#include <m2/game/object/Line.h>
#include "m2/Game.h"

m2::ObjectId m2::DynamicGridLinesLoader::load_vertical(int x) {
	if ((x % _step) == 0) {
		return obj::create_vertical_line(static_cast<float>(x) - 0.5f, _color);
	}
	return 0;
}
m2::ObjectId m2::DynamicGridLinesLoader::load_horizontal(int y) {
	if ((y % _step) == 0) {
		return obj::create_horizontal_line(static_cast<float>(y) - 0.5f, _color);
	}
	return 0;
}

void m2::DynamicGridLinesLoader::unload(ObjectId id) {
	GAME.add_deferred_action(create_object_deleter(id));
}
