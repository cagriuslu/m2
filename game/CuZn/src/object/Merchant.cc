#include <cuzn/object/Merchant.h>
#include <m2/Game.h>

void cuzn::init_merchant(m2::Object &obj) {
	auto& chr = obj.add_full_character();

	auto& gfx = obj.add_graphic();
	gfx.pre_draw = [&c = chr](m2::Graphic& g) {
		// Check if license is added
		auto it = c.find_items(m2g::pb::ITEM_CATEGORY_MERCHANT_LICENSE);
		// Set the sprite
		g.sprite = (it != c.end_items()) ? &GAME.get_sprite(it->game_sprite()) : nullptr;
	};
	gfx.on_draw = m2::Graphic::default_draw;
}
