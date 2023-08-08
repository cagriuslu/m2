#pragma once
#include <m2g_SpriteType.pb.h>
#include <m2/Object.h>

namespace rpg {
	m2::void_expected create_decoration(m2::Object& obj, m2g::pb::SpriteType sprite_type);
}
