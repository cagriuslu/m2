#include <impl/private/fsm/CharacterAnimation.h>
#include "m2/FSM.h"
#include <impl/public/SpriteBlueprint.h>
#include "m2/component/Graphic.h"

#define ALARM_DURATION (0.1f)

impl::fsm::CharacterAnimation::CharacterAnimation(m2::component::Graphic& gfx, const character::CharacterBlueprint* blueprint) : gfx(gfx), blueprint(blueprint) {}

void* impl::fsm::CharacterAnimation::idle(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			gfx.textureRect = impl::sprites[blueprint->main_sprite_index].texture_rect;
			gfx.center_px = impl::sprites[blueprint->main_sprite_index].obj_center_px;
			return nullptr;
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::downStop(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKDOWN_00]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKDOWN_00]].obj_center_px;
			return nullptr;
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::down00(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKDOWN_00]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKDOWN_00]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(downStop);
		case CHARANIM_WALKDOWN:
			return nullptr;
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::down01(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKDOWN_01]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKDOWN_01]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(down02);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(downStop);
		case CHARANIM_WALKDOWN:
			return nullptr;
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::down02(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKDOWN_00]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKDOWN_00]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(down03);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(downStop);
		case CHARANIM_WALKDOWN:
			return nullptr;
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::down03(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKDOWN_02]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKDOWN_02]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(down00);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(downStop);
		case CHARANIM_WALKDOWN:
			return nullptr;
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::leftStop(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKLEFT_00]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKLEFT_00]].obj_center_px;
			return nullptr;
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::left00(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKLEFT_00]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKLEFT_00]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(left01);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(leftStop);
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		//case CHARANIM_WALKLEFT:
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::left01(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKLEFT_01]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKLEFT_01]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(left02);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(leftStop);
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		//case CHARANIM_WALKLEFT:
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::left02(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKLEFT_00]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKLEFT_00]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(left03);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(leftStop);
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		//case CHARANIM_WALKLEFT:
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::left03(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKLEFT_02]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKLEFT_02]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(left00);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(leftStop);
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		//case CHARANIM_WALKLEFT:
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::rightStop(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKRIGHT_00]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKRIGHT_00]].obj_center_px;
			return nullptr;
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::right00(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKRIGHT_00]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKRIGHT_00]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(rightStop);
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return nullptr;
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::right01(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKRIGHT_01]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKRIGHT_01]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(right02);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(rightStop);
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return nullptr;
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::right02(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKRIGHT_00]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKRIGHT_00]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(right03);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(rightStop);
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return nullptr;
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::right03(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKRIGHT_02]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKRIGHT_02]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(right00);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(rightStop);
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return nullptr;
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::upStop(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKUP_00]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKUP_00]].obj_center_px;
			return nullptr;
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::up00(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKUP_00]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKUP_00]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(up01);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(upStop);
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return nullptr;
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::up01(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKUP_01]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKUP_01]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(up02);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(upStop);
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return nullptr;
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::up02(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKUP_00]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKUP_00]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(up03);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(upStop);
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return nullptr;
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}

void* impl::fsm::CharacterAnimation::up03(m2::FSM<CharacterAnimation>& automaton, int signal) {
	auto& gfx = automaton.data.gfx;
	const auto* blueprint = automaton.data.blueprint;
	switch (signal) {
		case m2::FSMSIG_ENTER:
			automaton.arm(ALARM_DURATION);
			gfx.textureRect = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKUP_02]].texture_rect;
			gfx.center_px = impl::sprites[blueprint->sprite_indexes[character::CHARACTER_TEXTURE_TYPE_LOOKUP_02]].obj_center_px;
			return nullptr;
		case m2::FSMSIG_EXIT:
			automaton.disarm();
			return nullptr;
		case m2::FSMSIG_ALARM:
			return reinterpret_cast<void*>(up00);
		case CHARANIM_STOP:
			return reinterpret_cast<void*>(upStop);
		case CHARANIM_WALKDOWN:
			return reinterpret_cast<void*>(down01);
		case CHARANIM_WALKRIGHT:
			return reinterpret_cast<void*>(right01);
		case CHARANIM_WALKUP:
			return nullptr;
		case CHARANIM_WALKLEFT:
			return reinterpret_cast<void*>(left01);
		default:
			return nullptr;
	}
}
