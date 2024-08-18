#pragma once
#include <m2/Object.h>
#include <m2g_ItemType.pb.h>
#include <cuzn/Detail.h>

// Filters
constexpr auto is_merchant_character = [](m2::Character& chr) { return chr.owner().object_type() == m2g::pb::MERCHANT; };
bool can_merchant_buy_sellable_industry(m2::Character& chr, SellableIndustry ind);

// Accessors
m2::Object* find_merchant_at_location(m2g::pb::SpriteType location);

// Both licensed and unlicensed merchant locations contain a merchant object.
// Licensed merchants can deal coal, and buy goods.
// Unlicensed merchants can deal coal.
void init_merchant(m2::Object& obj);
