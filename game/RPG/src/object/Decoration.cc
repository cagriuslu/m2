#include <rpg/object/Decoration.h>
#include <m2/Game.h>

m2::void_expected rpg::create_decoration(m2::Object& obj, m2g::pb::SpriteType sprite_type) {
	obj.add_graphic(GAME.get_sprite(sprite_type));
	return {};
}
