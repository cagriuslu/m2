#pragma once

// Returns the merchant locations that might be active based on the client count. Returns all merchants by default.
std::vector<m2g::pb::SpriteType> possibly_active_merchant_locations(int client_count = 4);

std::vector<m2g::pb::ItemType> prepare_merchant_license_list(int client_count);

std::vector<m2g::pb::ItemType> prepare_draw_deck(int client_count);

void give_8_cards_to_each_player(std::vector<m2g::pb::ItemType>& deck);
