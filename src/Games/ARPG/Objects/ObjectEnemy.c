#include <m2/Object.h>
#include <m2/Game.h>
#include <m2/Box2D.h>
#include <m2/Def.h>
#include <m2/Box2DUtils.h>
#include "../ARPG_Object.h"
#include "../ARPG_Cfg.h"
#include "../ARPG_Component.h"

void ObjectEnemy_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId);

	CharacterState_ProcessTime(&(AS_ENEMYDATA(obj->data)->characterState), GAME->deltaTime_s);
	Automaton_ProcessTime(&(AS_ENEMYDATA(obj->data)->aiAutomaton), GAME->deltaTime_s);
	Automaton_ProcessSignal(&(AS_ENEMYDATA(obj->data)->aiAutomaton), SIG_AI_PREPHYSICS);
}

void ObjectEnemy_onHit(ComponentDefense* def) {
	Object* obj = Pool_GetById(&GAME->objects, def->super.objId);
	AS_ENEMYDATA(obj->data)->onHitColorModTtl = 0.10f;
}

void ObjectEnemy_onDeath(ComponentDefense* def) {
	Game_DeleteList_Add(def->super.objId);
}

static void ObjectEnemy_postPhysics(ComponentMonitor* monitor) {
	Object* obj = Pool_GetById(&GAME->objects, monitor->super.objId); M2ASSERT(obj);
	ComponentPhysique* phy = Pool_GetById(&GAME->physics, obj->physique); M2ASSERT(phy);
	// We must call time before other signals
	Automaton_ProcessTime(&(AS_ENEMYDATA(obj->data)->charAnimationAutomaton), GAME->deltaTicks_ms / 1000.0f);
	Vec2F velocity = Box2DBodyGetLinearVelocity(phy->body);
	if (fabsf(velocity.x) < 0.5000f && fabsf(velocity.y) < 0.5000f) {
		Automaton_ProcessSignal(&(AS_ENEMYDATA(obj->data)->charAnimationAutomaton), SIG_CHARANIM_STOP);
	} else if (fabsf(velocity.x) < fabsf(velocity.y)) {
		if (0 < velocity.y) {
			Automaton_ProcessSignal(&(AS_ENEMYDATA(obj->data)->charAnimationAutomaton), SIG_CHARANIM_WALKDOWN);
		} else {
			Automaton_ProcessSignal(&(AS_ENEMYDATA(obj->data)->charAnimationAutomaton), SIG_CHARANIM_WALKUP);
		}
	} else {
		if (0 < velocity.x) {
			Automaton_ProcessSignal(&(AS_ENEMYDATA(obj->data)->charAnimationAutomaton), SIG_CHARANIM_WALKRIGHT);
		} else {
			Automaton_ProcessSignal(&(AS_ENEMYDATA(obj->data)->charAnimationAutomaton), SIG_CHARANIM_WALKLEFT);
		}
	}
}

void ObjectEnemy_Draw(ComponentGraphic* gfx) {
	Object* obj = Game_FindObjectById(gfx->super.objId); M2ASSERT(obj);
	if (0.0f < AS_ENEMYDATA(obj->data)->onHitColorModTtl) {
		SDL_Texture *defaultTexture = gfx->texture;
		gfx->texture = GAME->sdlTextureMask;
		ComponentGraphic_DefaultDraw(gfx);
		gfx->texture = defaultTexture;
		AS_ENEMYDATA(obj->data)->onHitColorModTtl -= GAME->deltaTicks_ms / 1000.0f;
	} else {
		ComponentGraphic_DefaultDraw(gfx);
	}
	ComponentDefense* defense = Object_GetDefense(obj); M2ASSERT(defense);
	ARPG_ComponentDefense *defenseData = AS_ARPG_COMPONENTDEFENSE(defense->data);
	ComponentGraphic_DefaultDrawHealthBar(gfx, (float) defenseData->hp / defenseData->maxHp);
}

int ObjectEnemy_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position) {
	M2ERR_REFLECT(Object_Init(obj, position));
	obj->data = calloc(1, sizeof(EnemyData)); M2ASSERT(obj->data);
	M2ERR_REFLECT(CharacterState_Init(&(AS_ENEMYDATA(obj->data)->characterState), cfg));

	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = ARPG_CFG_SPRITES[cfg->mainSpriteIndex].textureRect;
	gfx->center_px = ARPG_CFG_SPRITES[cfg->mainSpriteIndex].objCenter_px;
	gfx->draw = ObjectEnemy_Draw;

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->prePhysics = ObjectEnemy_prePhysics;
	el->postPhysics = ObjectEnemy_postPhysics;

	ComponentPhysique* phy = Object_AddPhysique(obj);
	phy->body = Box2DUtils_CreateDynamicDisk(
		Pool_GetId(&GAME->physics, phy),
		position,
		true, // allowSleep
		CATEGORY_ENEMY,
		ARPG_CFG_SPRITES[cfg->mainSpriteIndex].collider.params.circ.radius_m,
		cfg->mass_kg,
		cfg->linearDamping
	);

	ComponentDefense* defense = Object_AddDefense(obj);
	ARPG_ComponentDefense *defenseData = AS_ARPG_COMPONENTDEFENSE(defense->data);
	defenseData->hp = 100;
	defenseData->maxHp = 100;
	defenseData->onHit = ObjectEnemy_onHit;
	defenseData->onDeath = ObjectEnemy_onDeath;

	// Initialise character state after components
	// Character states may access components during initialisation

	AutomatonCharAnimation_Init(&(AS_ENEMYDATA(obj->data)->charAnimationAutomaton), cfg, gfx);

	if (cfg->ai) {
		AiState_Init(&(AS_ENEMYDATA(obj->data)->aiState), cfg->ai, position);
		switch (cfg->ai->behavior) {
			case CFG_AI_BEHAVIOR_CHASE:
				M2ERR_REFLECT(AutomatonAiChase_Init(&(AS_ENEMYDATA(obj->data)->aiAutomaton), obj, phy));
				break;
			case CFG_AI_BEHAVIOR_KEEP_DISTANCE:
				M2ERR_REFLECT(AutomatonAiKeepDistance_Init(&(AS_ENEMYDATA(obj->data)->aiAutomaton), obj, phy));
				break;
			case CFG_AI_BEHAVIOR_HIT_N_RUN:
				M2ERR_REFLECT(AutomatonAiHitNRun_Init(&(AS_ENEMYDATA(obj->data)->aiAutomaton), obj, phy));
				break;
			default:
				break;
		}
	}

	return 0;
}
