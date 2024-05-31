#include <cuzn/object/Merchant.h>
#include <cuzn/detail/Graphic.h>
#include <m2/Game.h>

void init_merchant(m2::Object& obj) {
	auto& chr = obj.add_full_character();

	auto& gfx = obj.add_graphic();
	gfx.pre_draw = [&chr](m2::Graphic& g) {
		// Set the sprite if license is added. Licenses are assigned after population, thus do it pre_draw.
		auto it = chr.find_items(m2g::pb::ITEM_CATEGORY_MERCHANT_LICENSE);
		g.sprite = (it != chr.end_items()) ? &M2_GAME.get_sprite(it->game_sprite()) : nullptr;
	};
	gfx.on_draw = [&chr](m2::Graphic& g) {
		m2::Graphic::default_draw(g); // Merchant sprite
		draw_resources(chr); // Resources
	};
}
