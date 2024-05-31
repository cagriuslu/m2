#pragma once
#include <m2/Object.h>
#include <m2g_ItemType.pb.h>

// Both licensed and unlicensed merchant locations contain a merchant object.
// Licensed merchants can deal coal, and buy goods.
// Unlicensed merchants can deal coal.
void init_merchant(m2::Object& obj);
