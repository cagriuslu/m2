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
void SellFactory(m2::FastCharacter& factory_chr);

// Filters
bool IsFactoryCharacter(const m2::FastCharacter& chr);
bool IsFactorySold(m2::FastCharacter& chr);
inline bool IsFactoryNotSold(m2::FastCharacter& chr) { return not IsFactorySold(chr); }
bool IsFactoryLevel1(m2::FastCharacter& chr);

// Transformers
City ToCityOfFactoryCharacter(m2::FastCharacter& chr);
Industry ToIndustryOfFactoryCharacter(const m2::FastCharacter& chr);
IndustryTile ToIndustryTileOfFactoryCharacter(const m2::FastCharacter& chr);
IndustryLocation ToIndustryLocationOfFactoryCharacter(m2::FastCharacter& chr);

m2::void_expected InitFactory(m2::Object&, const m2::VecF& position, City, IndustryTile);
