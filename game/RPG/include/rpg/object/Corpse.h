#pragma once
#include <m2/Object.h>
#include <m2/Meta.h>
#include <m2g_SpriteType.pb.h>

namespace rpg {
	m2::void_expected create_corpse(m2::Object& obj, m2g::pb::SpriteType);
}
