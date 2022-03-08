#include "../Automaton.h"
#include "../Object.h"

#define ALARM_DURATION (0.1f)

void* CharAnimation_IdleState(struct _Automaton* sm, int signal);
void* CharAnimation_DownStopState(struct _Automaton* sm, int signal);
void* CharAnimation_Down00State(struct _Automaton* sm, int signal);
void* CharAnimation_Down01State(struct _Automaton* sm, int signal);
void* CharAnimation_Down02State(struct _Automaton* sm, int signal);
void* CharAnimation_Down03State(struct _Automaton* sm, int signal);
void* CharAnimation_LeftStopState(struct _Automaton* sm, int signal);
void* CharAnimation_Left00State(struct _Automaton* sm, int signal);
void* CharAnimation_Left01State(struct _Automaton* sm, int signal);
void* CharAnimation_Left02State(struct _Automaton* sm, int signal);
void* CharAnimation_Left03State(struct _Automaton* sm, int signal);
void* CharAnimation_RightStopState(struct _Automaton* sm, int signal);
void* CharAnimation_Right00State(struct _Automaton* sm, int signal);
void* CharAnimation_Right01State(struct _Automaton* sm, int signal);
void* CharAnimation_Right02State(struct _Automaton* sm, int signal);
void* CharAnimation_Right03State(struct _Automaton* sm, int signal);
void* CharAnimation_UpStopState(struct _Automaton* sm, int signal);
void* CharAnimation_Up00State(struct _Automaton* sm, int signal);
void* CharAnimation_Up01State(struct _Automaton* sm, int signal);
void* CharAnimation_Up02State(struct _Automaton* sm, int signal);
void* CharAnimation_Up03State(struct _Automaton* sm, int signal);

M2Err AutomatonCharAnimation_Init(Automaton *sm, const void* cfgCharacter, void* gfx) {
	M2ERR_REFLECT(Automaton_Init(sm));
	sm->currState = CharAnimation_IdleState;
	sm->userData_cfg = cfgCharacter;
	sm->userData_gfx = gfx;
	Automaton_ProcessSignal(sm, SIG_ENTER);
	return M2OK;
}

void* CharAnimation_IdleState(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			gfx->textureRect = cfg->mainTexture->textureRect;
			gfx->center_px = cfg->mainTexture->objCenter_px;
			return NULL;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_DownStopState(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_00]->objCenter_px;
			return NULL;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Down00State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_DownStopState;
		case SIG_CHARANIM_WALKDOWN:
			return NULL;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Down01State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_01]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_01]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Down02State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_DownStopState;
		case SIG_CHARANIM_WALKDOWN:
			return NULL;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Down02State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Down03State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_DownStopState;
		case SIG_CHARANIM_WALKDOWN:
			return NULL;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Down03State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_02]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKDOWN_02]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Down00State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_DownStopState;
		case SIG_CHARANIM_WALKDOWN:
			return NULL;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_LeftStopState(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_00]->objCenter_px;
			return NULL;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Left00State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Left01State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_LeftStopState;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		//case SIG_CHARANIM_WALKLEFT:
		default:
			return NULL;
	}
}

void* CharAnimation_Left01State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_01]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_01]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Left02State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_LeftStopState;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		//case SIG_CHARANIM_WALKLEFT:
		default:
			return NULL;
	}
}

void* CharAnimation_Left02State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Left03State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_LeftStopState;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		//case SIG_CHARANIM_WALKLEFT:
		default:
			return NULL;
	}
}

void* CharAnimation_Left03State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_02]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKLEFT_02]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Left00State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_LeftStopState;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		//case SIG_CHARANIM_WALKLEFT:
		default:
			return NULL;
	}
}

void* CharAnimation_RightStopState(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_00]->objCenter_px;
			return NULL;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Right00State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_RightStopState;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return NULL;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Right01State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_01]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_01]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Right02State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_RightStopState;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return NULL;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Right02State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Right03State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_RightStopState;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return NULL;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Right03State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_02]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKRIGHT_02]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Right00State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_RightStopState;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return NULL;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_UpStopState(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_00]->objCenter_px;
			return NULL;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Up00State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Up01State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_UpStopState;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return NULL;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Up01State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_01]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_01]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Up02State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_UpStopState;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return NULL;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Up02State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_00]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_00]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Up03State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_UpStopState;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return NULL;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}

void* CharAnimation_Up03State(struct _Automaton* sm, int signal) {
	const CfgCharacter* cfg = sm->userData_cfg;
	ComponentGraphic* gfx = sm->userData_gfx;
	switch (signal) {
		case SIG_ENTER:
			Automaton_ArmAlarm(sm, ALARM_DURATION);
			gfx->textureRect = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_02]->textureRect;
			gfx->center_px = cfg->textures[CFG_CHARTEXTURETYP_LOOKUP_02]->objCenter_px;
			return NULL;
		case SIG_EXIT:
			Automaton_DisarmAlarm(sm);
			return NULL;
		case SIG_ALARM:
			return CharAnimation_Up00State;
		case SIG_CHARANIM_STOP:
			return CharAnimation_UpStopState;
		case SIG_CHARANIM_WALKDOWN:
			return CharAnimation_Down01State;
		case SIG_CHARANIM_WALKRIGHT:
			return CharAnimation_Right01State;
		case SIG_CHARANIM_WALKUP:
			return NULL;
		case SIG_CHARANIM_WALKLEFT:
			return CharAnimation_Left01State;
		default:
			return NULL;
	}
}