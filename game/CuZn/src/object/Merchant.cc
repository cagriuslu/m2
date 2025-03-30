#include <cuzn/object/Merchant.h>
#include <cuzn/detail/Graphic.h>
#include <m2/Game.h>

bool can_merchant_buy_sellable_industry(m2::Character& chr, SellableIndustry ind) {
	return chr.HasItem(m2g::pb::WILD_MERCHANT_LICENSE) || chr.HasItem(merchant_license_type_of_sellable_industry(ind));
}

m2::Object* find_merchant_at_location(m2g::pb::SpriteType location) {
	auto merchants = M2_LEVEL.characters
		| std::views::transform(m2::ToCharacterBase)
		| std::views::filter(is_merchant_character)
		| std::views::transform(m2::ToOwnerOfCharacter)
		| std::views::filter(m2::is_object_in_area(std::get<m2::RectF>( M2G_PROXY.merchant_positions[location])));
	if (auto merchant_it = merchants.begin(); merchant_it != merchants.end()) {
		return &*merchant_it;
	}
	return nullptr;
}

void init_merchant(m2::Object& obj) {
	auto& chr = obj.AddFullCharacter();
	// Active merchants will be given a merchant license during setup. (ITEM_CATEGORY_MERCHANT_LICENSE)
	// Passive merchants will simply exist without a license.

	auto& gfx = obj.AddGraphic();
	gfx.preDraw = [&chr](m2::Graphic& g) {
		// Set the sprite if license is added. Licenses are assigned after population, thus do it pre_draw.
		auto it = chr.FindItems(m2g::pb::ITEM_CATEGORY_MERCHANT_LICENSE);
		if (it != chr.EndItems()) {
			g.visual = &std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(it->GameSprite()));
		} else {
			g.visual = std::monostate{};
		}
	};
	gfx.onDraw = [&chr](m2::Graphic& g) {
		m2::Graphic::DefaultDrawCallback(g); // Merchant sprite
		DrawResources(chr); // Resources
	};
}
