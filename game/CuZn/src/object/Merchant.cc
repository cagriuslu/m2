#include <cuzn/object/Merchant.h>
#include <cuzn/detail/Graphic.h>
#include <m2/Game.h>

bool can_merchant_buy_sellable_industry(m2::Character& chr, SellableIndustry ind) {
	return chr.has_item(m2g::pb::WILD_MERCHANT_LICENSE) || chr.has_item(merchant_license_type_of_sellable_industry(ind));
}

m2::Object* find_merchant_at_location(m2g::pb::SpriteType location) {
	auto merchants = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(is_merchant_character)
		| std::views::transform(m2::to_parent_of_component)
		| std::views::filter(m2::is_object_in_area(std::get<m2::RectF>( M2G_PROXY.merchant_positions[location])));
	if (auto merchant_it = merchants.begin(); merchant_it != merchants.end()) {
		return &*merchant_it;
	}
	return nullptr;
}

void init_merchant(m2::Object& obj) {
	auto& chr = obj.add_full_character();
	// Active merchants will be given a merchant license during setup. (ITEM_CATEGORY_MERCHANT_LICENSE)
	// Passive merchants will simply exist without a license.

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
