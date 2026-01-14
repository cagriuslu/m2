#pragma once
#include <m2g_CardType.pb.h>
#include <optional>

std::optional<m2g::pb::CardType> ask_for_industry_selection(m2g::pb::CardType industry_1, m2g::pb::CardType industry_2);
