#pragma once
#include <Sprite.pb.h>
#include <variant>

namespace m2 {
	struct DefaultVariant {};
	struct ForegroundCompanion {};
	using SpriteVariant = std::variant<DefaultVariant, ForegroundCompanion, pb::SpriteEffectType>;
}
