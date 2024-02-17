#include <cuzn/object/Merchant.h>
#include <m2/Game.h>

void cuzn::init_merchant(m2::Object& obj) {
	auto& chr = obj.add_full_character();

	auto& gfx = obj.add_graphic();
	gfx.pre_draw = [&c = chr](m2::Graphic& g) {
		// Check if license is added
		auto it = c.find_items(m2g::pb::ITEM_CATEGORY_MERCHANT_LICENSE);
		// Set the sprite
		g.sprite = (it != c.end_items()) ? &GAME.get_sprite(it->game_sprite()) : nullptr;
	};
	gfx.on_draw = [&c = chr, pos = obj.position](m2::Graphic& g) {
		// First, draw the merchant sprite
		m2::Graphic::default_draw(g);

		// Second, draw the beer barrl if exists
		auto beer_barrel_count = c.get_resource(m2g::pb::BEER_BARREL_COUNT);
		if (beer_barrel_count == 1.0f) {
			draw_real_2d(
			    pos + m2::VecF{1, 1}, GAME.get_sprite(m2g::pb::BEER_BARREL), m2::IsForegroundCompanion{false}, 0.0f);
		} else if (beer_barrel_count != 0.0f) {
			throw M2ERROR(std::string("Invalid beer barrel count: ") + std::to_string(beer_barrel_count));
		}
	};
}
