#pragma once
#include <m2/common/Meta.h>
#include <m2g_CardType.pb.h>

#include <vector>

#include "m2/Object.h"
#include "m2/component/character/FastCharacter.h"
#include "m2/game/Animation.h"
#include "m2/game/fsm/AnimationFsm.h"

namespace rpg {
	class PlayerCharacter : public m2::FastCharacter {
	public:
		using FastCharacter::FastCharacter;

		void OnUpdate(m2::Stopwatch::Duration);
		void OnMessage(m2::Interaction);
		using FastCharacter::OnMessage;
	};

	struct Player : public m2::HeapObjectImpl {
		m2::AnimationFsm animation_fsm;

		explicit Player(m2::Object& obj);

		static m2::void_expected init(m2::Object& obj, const m2::VecF& position);
	};
}
