#pragma once

#include <m2/Object.h>
#include <cuzn/Detail.h>

// Accessors
m2::Object* FindFactoryAtLocation(Location location);
int RequiredBeerCountToSell(IndustryLocation location);

// Global Modifiers
void RemoveObsoleteFactories();
void FlipExhaustedFactories();

// Modifiers
void SellFactory(m2::Character& factory_chr);

// Filters
inline bool IsFactoryCharacter(const m2::Character& chr) { return chr.Owner().GetType() == m2g::pb::FACTORY; }
bool IsFactorySold(m2::Character& chr);
inline bool IsFactoryNotSold(m2::Character& chr) { return not IsFactorySold(chr); }
bool IsFactoryLevel1(m2::Character& chr);

// Transformers
City ToCityOfFactoryCharacter(m2::Character& chr);
Industry ToIndustryOfFactoryCharacter(const m2::Character& chr);
IndustryTile ToIndustryTileOfFactoryCharacter(const m2::Character& chr);
IndustryLocation ToIndustryLocationOfFactoryCharacter(m2::Character& chr);

m2::void_expected InitFactory(m2::Object&, const m2::VecF& position, City, IndustryTile);
