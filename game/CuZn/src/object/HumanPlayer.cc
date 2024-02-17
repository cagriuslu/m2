#include <cuzn/object/HumanPlayer.h>
#include <m2/Game.h>
#include <m2/Controls.h>
#include <m2/protobuf/Detail.h>

m2::void_expected cuzn::init_human_player(m2::Object& obj) {
	auto& chr = obj.add_full_character();
	chr.add_resource(m2g::pb::VICTORY_POINTS, 0.0f);
	chr.add_resource(m2g::pb::INCOME_POINTS, 0.0f);
	chr.add_resource(m2g::pb::DEBT_POINTS, 0.0f);
	chr.add_resource(m2g::pb::MONEY, 17.0f);

	// Add industry tiles
	for (auto industry_tile = m2g::pb::COTTON_MILL_TILEI; industry_tile <= m2g::pb::MANUFACTURED_GOODS_TILE_VIII; industry_tile = static_cast<m2g::pb::ItemType>(m2::I(industry_tile) + 1)) {
		// Lookup possession count
		auto& item = GAME.get_named_item(industry_tile);
		auto possession_limit = m2::I(item.get_attribute(m2g::pb::POSSESSION_LIMIT));
		m2_repeat(possession_limit) {
			chr.add_named_item(item);
		}
	}

	auto& gfx = obj.add_graphic();
	gfx.pre_draw = [&o = obj](MAYBE m2::Graphic& _) {
		m2::VecF move_direction;
		if (GAME.events.is_key_down(m2::Key::UP)) {
			move_direction.y -= 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::DOWN)) {
			move_direction.y += 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::LEFT)) {
			move_direction.x -= 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::RIGHT)) {
			move_direction.x += 1.0f;
		}
		o.position += move_direction.normalize() * ((float)GAME.delta_time_s() * GAME.dimensions().height_m);
	};

	return {};
}
