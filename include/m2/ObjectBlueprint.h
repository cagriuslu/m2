#pragma once
#include <m2g_ObjectType.pb.h>
#include <m2g_SpriteType.pb.h>
#include <m2g_AnimationType.pb.h>

namespace m2 {
	struct ObjectBlueprint {
		m2g::pb::ObjectType objectType{};
		m2g::pb::SpriteType defaultSpriteType{};
		m2g::pb::AnimationType defaultAnimationType{};
	};

	std::vector<ObjectBlueprint> LoadObjectBlueprints(const std::vector<ObjectBlueprint>&);
}
