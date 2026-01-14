#pragma once
#include <m2/component/Character.h>
#include <m2/Object.h>
#include <m2/Meta.h>
#include <set>
#include <cuzn/Detail.h>

m2::void_expected PlayerInitThisInstance(m2::Object& obj, const m2::VecF& position);
m2::void_expected PlayerInitOtherInstance(m2::Object& obj);

// Card Accessors

size_t PlayerCardCount(const m2::Character& player);
std::list<m2g::pb::CardType> PlayerCards(const m2::Character& player);
inline bool PlayerHasCard(const m2::Character& player, const m2g::pb::CardType card) { return player.FindCards(card) != player.EndCards(); }

// Resource Accessors

int PlayerLinkCount(const m2::Character& player);
int PlayerEstimatedVictoryPoints(const m2::Character& player);
int PlayerVictoryPoints(const m2::Character& player);
int PlayerIncomePoints(const m2::Character& player);
int PlayerMoney(const m2::Character& player);

// Industry and Tile Accessors

size_t PlayerIndustryTileCount(const m2::Character& player);
std::optional<m2g::pb::CardType> PlayerNextIndustryTileOfCategory(const m2::Character& player, m2g::pb::CardCategory tile_category);
std::optional<m2g::pb::CardType> PlayerNextIndustryTileOfIndustry(const m2::Character& player, Industry);
size_t PlayerBuiltFactoryCount(const m2::Character& player);
std::set<IndustryLocation> PlayerBuiltFactoryLocations(const m2::Character& player);
std::set<IndustryLocation> PlayerSellableFactoryLocations(const m2::Character& player);
m2::void_expected PlayerCanOverbuild(const m2::Character& player, IndustryLocation, m2g::pb::CardType);

// Road and Network Accessors

inline size_t PlayerUnbuiltRoadCount(const m2::Character& player) { return player.CountCard(m2g::pb::ROAD_TILE); }
std::set<m2g::pb::CardType> PlayerCitiesInNetwork(const m2::Character& player);
std::set<m2g::pb::SpriteType> PlayerCanalsInNetwork(const m2::Character& player, Connection provisional_extra_connection = {});
std::set<m2g::pb::SpriteType> PlayerRailroadsInNetwork(const m2::Character& player, Connection provisional_extra_connection = {});
std::set<m2g::pb::SpriteType> PlayerConnectionsInNetwork(const m2::Character& player, Connection provisional_extra_connection = {});
