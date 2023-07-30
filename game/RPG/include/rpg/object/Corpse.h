#pragma once
#include <m2/Object.h>
#include <m2/Value.h>
#include <m2g_SpriteType.pb.h>

namespace rpg {
	m2::VoidValue create_corpse(m2::Object& obj, m2g::pb::SpriteType);
}
