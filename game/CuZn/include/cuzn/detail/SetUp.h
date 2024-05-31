#pragma once

// Returns all merchants by default
std::vector<m2g::pb::SpriteType> active_merchant_locations(int client_count = 4);

std::vector<m2g::pb::ItemType> prepare_merchant_license_list(int client_count);

std::vector<m2g::pb::ItemType> prepare_draw_deck(int client_count);
