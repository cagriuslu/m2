#pragma once
#include <m2/component/character/CompactCharacter.h>

namespace rpg {
	class ProjectileCharacter : public m2::CompactCharacter {
		bool _isExplosive;
		float _damageRadius;

	public:
		ProjectileCharacter(m2::ObjectId ownerId, bool isExplosive, float damageRadius);

		void OnUpdate(m2::Stopwatch::Duration);
	};
}
