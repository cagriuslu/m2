#pragma once
#include <m2/M2.h>
#include <m2/component/Character.h>

namespace cuzn {
	/// Returns the next buildable tile type
	m2::expected<m2g::pb::ItemType> can_player_build_industry(m2::Character& player, m2g::pb::SpriteType location, m2g::pb::ItemType industry);
}
