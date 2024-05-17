#include <cuzn/object/Merchant.h>
#include <cuzn/detail/Graphic.h>
#include <m2/Game.h>

void cuzn::init_merchant(m2::Object& obj) {
	auto& chr = obj.add_full_character();

	auto& gfx = obj.add_graphic();
	gfx.pre_draw = [&c = chr](m2::Graphic& g) {
		// Check if license is added
		auto it = c.find_items(m2g::pb::ITEM_CATEGORY_MERCHANT_LICENSE);
		// Set the sprite
		g.sprite = (it != c.end_items()) ? &M2_GAME.get_sprite(it->game_sprite()) : nullptr;
	};
	gfx.on_draw = [&c = chr, pos = obj.position](m2::Graphic& g) {
		// First, draw the merchant sprite
		m2::Graphic::default_draw(g);
		// Then, draw the resources
		cuzn::draw_resources(c);
	};
}
