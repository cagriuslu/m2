#pragma once
#include <m2g_ItemType.pb.h>
#include <optional>

std::optional<m2g::pb::ItemType> ask_for_industry_selection(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2);
