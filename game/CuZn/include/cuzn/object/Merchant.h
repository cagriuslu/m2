#pragma once
#include <m2/Object.h>
#include <m2g_CardType.pb.h>
#include <cuzn/Detail.h>

// Filters
bool is_merchant_character(m2::FastCharacter& chr);
bool can_merchant_buy_sellable_industry(m2::FastCharacter& chr, SellableIndustry ind);

// Accessors
m2::Object* find_merchant_at_location(m2g::pb::SpriteType location);

// Both licensed and unlicensed merchant locations contain a merchant object.
// Licensed merchants can deal coal, and buy goods.
// Unlicensed merchants can deal coal.
void init_merchant(m2::Object& obj, const m2::VecF& position);
