#include <m2g/Proxy.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/object/MarketTracker.h>
#include <cuzn/object/Merchant.h>
#include <cuzn/object/Factory.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextListSelection.h>
#include <cuzn/ui/Client.h>
#include <cuzn/ui/PauseMenu.h>
#include <cuzn/ui/MainMenu.h>
#include <cuzn/ui/LeftHud.h>
#include <cuzn/ui/RightHud.h>
#include <cuzn/detail/SetUp.h>
#include <m2/game/Detail.h>

using namespace cuzn;

const m2::ui::Blueprint* m2g::Proxy::main_menu() { return &main_menu_blueprint; }

const m2::ui::Blueprint* m2g::Proxy::pause_menu() { return &pause_menu_blueprint; }

const m2::ui::Blueprint* m2g::Proxy::left_hud() { return &left_hud_blueprint; }

const m2::ui::Blueprint* m2g::Proxy::right_hud() { return &right_hud_blueprint; }

void m2g::Proxy::post_multi_player_level_client_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {
	DEBUG_FN();

	auto client_count =
	    M2_GAME.is_server() ? M2_GAME.server_thread().client_count() : M2_GAME.client_thread().total_player_count();

	// Add human players
	for (auto i = 0; i < client_count; ++i) {
		auto it = m2::create_object(m2::VecF{i, i}, m2g::pb::ObjectType::HUMAN_PLAYER);
		auto client_init_result = init_human_player(*it);
		m2_succeed_or_throw_error(client_init_result);
		multi_player_object_ids.emplace_back(it.id());
		player_colors.emplace_back(generate_player_color(i));

		if (i == M2_GAME.client_thread().receiver_index()) {
			M2_LEVEL.player_id = it.id();
		}
	}

	// Add all merchants (without license) since all merchants can deal in coal
	for (const auto& merchant_sprite : active_merchant_locations()) {
		// Create merchant object
		auto it = m2::create_object(merchant_positions[merchant_sprite].first, m2g::pb::ObjectType::MERCHANT);
		init_merchant(*it);
		// Store for later
		merchant_object_ids[merchant_sprite] = it.id();
	}

	// Add market object
	auto it = m2::create_object(m2::VecF{}, m2g::pb::ObjectType::MARKET_TRACKER);
	init_market(*it);
	_market_object_id = it.id();
}

void m2g::Proxy::multi_player_level_server_populate(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {
	auto client_count = M2_GAME.server_thread().client_count();

	// Assign licenses to active merchants
	{
		auto merchant_licenses = prepare_merchant_license_list(client_count);
		auto active_merchant_locations = cuzn::active_merchant_locations(client_count);
		if (merchant_licenses.size() != active_merchant_locations.size()) {
			throw M2ERROR("Merchant count mismatch");
		}

		for (const auto& merchant_location : active_merchant_locations) {
			// Pop license from the list
			auto license = merchant_licenses.back();
			merchant_licenses.pop_back();
			const auto& license_item = M2_GAME.get_named_item(license);

			// Retrieve merchant object
			auto merchant_object_id = merchant_object_ids[merchant_location];
			auto& merchant_char = M2_LEVEL.objects[merchant_object_id].character();

			LOG_DEBUG("Adding license to merchant", m2g::pb::ItemType_Name(license));
			merchant_char.add_named_item(license_item);
			merchant_char.add_resource(pb::BEER_BARREL_COUNT, license_item.get_attribute(pb::BEER_BONUS_FIRST_ERA));
		}
	}

	// Prepare draw deck
	auto draw_deck = prepare_draw_deck(client_count);
	// In the canal era, we discard client_count number of cards from the deck
	m2_repeat(client_count) { draw_deck.pop_back(); }
	// Give 8 cards to each player
	for (const auto& player_object_id : M2G_PROXY.multi_player_object_ids) {
		m2_repeat(8) {
			// Draw card
			auto card = draw_deck.back();
			draw_deck.pop_back();
			// Add card
			M2_LEVEL.objects[player_object_id].character().add_named_item(M2_GAME.get_named_item(card));
		}
	}

	// TODO
}

std::optional<int> m2g::Proxy::handle_client_command(unsigned turn_holder_index, MAYBE const m2g::pb::ClientCommand& client_command) {
	LOG_INFO("Received command from client", turn_holder_index);

	if (client_command.has_build_action()) {
		// TODO verify whether the player can build it

		auto it = m2::create_object(
			position_of_industry_location(client_command.build_action().industry_location()),
			m2g::pb::FACTORY,
			M2G_PROXY.multi_player_object_ids[turn_holder_index]);
		auto success = init_factory(*it,
			city_of_location(client_command.build_action().industry_location()),
			client_command.build_action().industry_tile());
		// TODO check result
	}

	// Increment turn holder
	return (turn_holder_index + 1) % M2_GAME.server_thread().client_count();
}

void m2g::Proxy::post_tile_create(m2::Object& obj, m2g::pb::SpriteType sprite_type) {
	// Store the positions of the merchants
	if (is_merchant_location(sprite_type)) {
		// Object position has {0.5f, 0.5f} offset
		auto merchant_cell_rect = m2::RectF{obj.position.x - 0.5f, obj.position.y - 0.5f, 2.0f, 2.0f};
		merchant_positions[sprite_type] = std::make_pair(obj.position, merchant_cell_rect);
		LOG_DEBUG("Merchant position", m2g::pb::SpriteType_Name(sprite_type), merchant_cell_rect);
	}

	// Store the positions of the industries build locations
	else if (is_industry_location(sprite_type)) {
		// Verify that ppm of the industry tiles are double of the sprite sheet
		if (M2_GAME.get_sprite(sprite_type).ppm() != M2_GAME.get_sprite(sprite_type).sprite_sheet().sprite_sheet().ppm()) {
			throw M2ERROR("Sprite ppm mismatch");
		}
		// Object position has {0.5f, 0.5f} offset
		auto industry_cell_rect = m2::RectF{obj.position.x - 0.5f, obj.position.y - 0.5f, 2.0f, 2.0f};
		industry_positions[sprite_type] = std::make_pair(obj.position, industry_cell_rect);
		LOG_DEBUG("Industry position", m2g::pb::SpriteType_Name(sprite_type), industry_cell_rect);
	}

	// Store the positions of the connection locations
	else if (is_canal(sprite_type) || is_railroad(sprite_type)) {
		// Object position has {0.5f, 0.5f} offset
		auto connection_cell_rect = m2::RectF{obj.position.x - 0.5f, obj.position.y - 0.5f, 1.0f, 1.0f};
		connection_positions[sprite_type] = std::make_pair(obj.position, connection_cell_rect);
		LOG_DEBUG("Connection position", m2g::pb::SpriteType_Name(sprite_type), connection_cell_rect);
	}
}

m2::void_expected m2g::Proxy::init_level_blueprint_fg_object(MAYBE m2::Object& obj) {
	return m2::make_unexpected("Invalid object type");
}

m2::void_expected m2g::Proxy::init_server_update_fg_object(m2::Object& obj, const std::vector<m2g::pb::ItemType>& items,
	MAYBE const std::vector<m2::pb::Resource>& resources) {
	switch (obj.object_type()) {
		case pb::FACTORY: {
			auto city = std::ranges::find_if(items, cuzn::is_city);
			auto industry_tile = std::ranges::find_if(items, cuzn::is_industry_tile);
			if (city != items.end() && industry_tile != items.end()) {
				return init_factory(obj, *city, *industry_tile);
			} else {
				return m2::make_unexpected("Unable to find city or industry tile of the object received from the server");
			}
		}
		default:
			return m2::make_unexpected("Invalid object type");
	}
}

m2g::Proxy& m2g::Proxy::get_instance() {
	return dynamic_cast<m2g::Proxy&>(M2_GAME.proxy());
}

void m2g::Proxy::user_journey_deleter() {
	get_instance().user_journey.reset();
}

unsigned m2g::Proxy::player_index(m2::Id id) const {
	auto it = std::find(multi_player_object_ids.begin(), multi_player_object_ids.end(), id);
	if (it != multi_player_object_ids.end()) {
		return std::distance(multi_player_object_ids.begin(), it);
	} else {
		throw M2ERROR("Invalid player ID");
	}
}
