#include "../StateMachine.h"
#include "../Object.h"
#include "../Game.h"

#define ALARM_DURATION (0.1f)

void* StateMachineCharacterAnimation_IdleState(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_DownStopState(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Down00State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Down01State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Down02State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Down03State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_LeftStopState(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Left00State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Left01State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Left02State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Left03State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_RightStopState(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Right00State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Right01State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Right02State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Right03State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_UpStopState(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Up00State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Up01State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Up02State(struct _StateMachine* sm, int signal);
void* StateMachineCharacterAnimation_Up03State(struct _StateMachine* sm, int signal);

XErr StateMachineCharacterAnimation_Init(StateMachine *sm, const void* cfgCharacter, void* gfx) {
	XERR_REFLECT(StateMachine_Init(sm));
	sm->currState = StateMachineCharacterAnimation_IdleState;
	sm->userData_cfg = cfgCharacter;
	sm->userData_gfx = gfx;
	StateMachine_ProcessSignal(sm, SIG_ENTER);
	return XOK;
}

void* StateMachineCharacterAnimation_IdleState(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			gfx->textureRect = cfg->mainTexture->textureRect;
			gfx->center_px = cfg->mainTexture->objCenter_px;
			return NULL;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_DownStopState(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_00]->objCenter_px;
			return NULL;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Down00State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_DownStopState;
		case SIG_CHARANIM_WALKDOWN:
			return NULL;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Down01State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_01]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_01]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Down02State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_DownStopState;
		case SIG_CHARANIM_WALKDOWN:
			return NULL;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Down02State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Down03State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_DownStopState;
		case SIG_CHARANIM_WALKDOWN:
			return NULL;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Down03State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_02]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_02]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Down00State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_DownStopState;
		case SIG_CHARANIM_WALKDOWN:
			return NULL;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_LeftStopState(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_00]->objCenter_px;
			return NULL;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Left00State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Left01State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_LeftStopState;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return NULL;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Left01State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_01]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_01]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Left02State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_LeftStopState;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return NULL;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Left02State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Left03State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_LeftStopState;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return NULL;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Left03State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_02]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_02]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Left00State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_LeftStopState;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return NULL;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_RightStopState(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_00]->objCenter_px;
			return NULL;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Right00State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_RightStopState;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return NULL;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Right01State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_01]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_01]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Right02State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_RightStopState;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return NULL;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Right02State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Right03State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_RightStopState;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return NULL;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Right03State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_02]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_02]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Right00State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_RightStopState;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return NULL;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_UpStopState(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_00]->objCenter_px;
			return NULL;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Up00State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Up01State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_UpStopState;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return NULL;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Up01State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_01]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_01]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Up02State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_UpStopState;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return NULL;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Up02State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Up03State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_UpStopState;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return NULL;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}

void* StateMachineCharacterAnimation_Up03State(struct _StateMachine* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			StateMachine_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_02]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_02]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			StateMachine_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return StateMachineCharacterAnimation_Up00State;
		case SIG_CHARANIM_STOP:
			return StateMachineCharacterAnimation_UpStopState;
		case SIG_CHARANIM_WALKDOWN:
			return StateMachineCharacterAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return StateMachineCharacterAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return NULL;
		case SIG_CHARANIM_WALKLEFT:
			return StateMachineCharacterAnimation_Left01State;
		default:
			return NULL;
	}
}
