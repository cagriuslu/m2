#pragma once
#include <m2/component/Character.h>
#include <m2g_ItemType.pb.h>
#include <m2g_SpriteType.pb.h>
#include <m2/M2.h>
#include <m2/game/Graph.h>
#include <cuzn/Detail.h>

/// Returns industry or merchant locations starting from the given city.
/// Useful for iterating over accessible locations from a city.
std::set<Location> ReachableLocationsFromIndustryCity(IndustryCity city);

/// Check if a location can be reached from an industry city
bool IsIndustryCityConnectedToLocation(IndustryCity city, Location location);

/// Create a graph of currently built connections where nodes are of type `City`.
/// Useful for finding the closest cities/industries.
m2::Graph CreateActiveConnectionsGraph();
