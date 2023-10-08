#pragma once
#include <box2d/b2_body.h>
#include "../Pool.h"
#include "../VecF.h"
#include <BodyBlueprint.pb.h>
#include <memory>

namespace m2::box2d {
	enum class ColliderType {
		NONE,
		RECTANGLE,
		CIRCLE
	};

	struct BodyDeleter {
		void operator()(b2Body*);
	};
	using BodyUniquePtr = std::unique_ptr<b2Body, BodyDeleter>;

	BodyUniquePtr create_body(b2World& world, Id physique_id, m2::VecF position, const pb::BodyBlueprint& blueprint);

	bool has_obstacle(const b2Body* body);
}
