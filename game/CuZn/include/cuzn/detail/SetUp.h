#pragma once
#include <m2g_ItemType.pb.h>
#include <m2g_SpriteType.pb.h>
#include <vector>

/// Returns the merchant locations that might be active based on the client count. Returns all merchants by default.
std::vector<m2g::pb::SpriteType> PossiblyActiveMerchantLocations(int client_count = 4);

std::vector<m2g::pb::ItemType> PrepareMerchantLicenseList(int client_count);

std::vector<m2g::pb::ItemType> PrepareDrawDeck(int client_count);

void Give8CardsToEachPlayer(std::vector<m2g::pb::ItemType>& deck);
