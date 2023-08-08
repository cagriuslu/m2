#pragma once
#include <m2g_ObjectType.pb.h>
#include <m2/Object.h>
#include <m2/Meta.h>

namespace rpg {
	m2::void_expected init_finish_point(m2::Object& obj, m2g::pb::ObjectType& type);
}
