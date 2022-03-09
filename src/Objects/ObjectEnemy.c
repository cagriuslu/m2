#include "../Object.h"
#include "../Game.h"
#include "../Box2D.h"
#include "../Def.h"
#include "../Pathfinder.h"
#include "../Box2DUtils.h"

void ObjectEnemy_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId); M2ASSERT(obj);
	Object* player = Game_FindObjectById(GAME->playerId); M2ASSERT(player);

	CharacterState_ProcessTime(&obj->ex->enemy.characterState, GAME->deltaTime);
	Automaton_ProcessTime(&obj->ex->enemy.aiAutomaton, GAME->deltaTime);
	Automaton_ProcessSignal(&obj->ex->enemy.aiAutomaton, SIG_AI_PREPHYSICS);
}

void ObjectEnemy_onHit(ComponentDefense* def) {
	Object* obj = Pool_GetById(&GAME->objects, def->super.objId); M2ASSERT(obj);
	obj->ex->enemy.onHitColorModTtl = 0.10f;
}

void ObjectEnemy_onDeath(ComponentDefense* def) {
	Game_DeleteList_Add(def->super.objId);
}

static void ObjectEnemy_postPhysics(ComponentMonitor* monitor) {
	Object* obj = Pool_GetById(&GAME->objects, monitor->super.objId); M2ASSERT(obj);
	ComponentPhysique* phy = Pool_GetById(&GAME->physics, obj->physique); M2ASSERT(phy);
	// We must call time before other signals
	Automaton_ProcessTime(&obj->ex->enemy.charAnimationAutomaton, GAME->deltaTicks / 1000.0f);
	Vec2F velocity = Box2DBodyGetLinearVelocity(phy->body);
	if (fabsf(velocity.x) < 0.5000f && fabsf(velocity.y) < 0.5000f) {
		Automaton_ProcessSignal(&obj->ex->enemy.charAnimationAutomaton, SIG_CHARANIM_STOP);
	} else if (fabsf(velocity.x) < fabsf(velocity.y)) {
		if (0 < velocity.y) {
			Automaton_ProcessSignal(&obj->ex->enemy.charAnimationAutomaton, SIG_CHARANIM_WALKDOWN);
		} else {
			Automaton_ProcessSignal(&obj->ex->enemy.charAnimationAutomaton, SIG_CHARANIM_WALKUP);
		}
	} else {
		if (0 < velocity.x) {
			Automaton_ProcessSignal(&obj->ex->enemy.charAnimationAutomaton, SIG_CHARANIM_WALKRIGHT);
		} else {
			Automaton_ProcessSignal(&obj->ex->enemy.charAnimationAutomaton, SIG_CHARANIM_WALKLEFT);
		}
	}
}

void ObjectEnemy_Draw(ComponentGraphic* gfx) {
	Object* obj = Game_FindObjectById(gfx->super.objId); M2ASSERT(obj);
	if (0.0f < obj->ex->enemy.onHitColorModTtl) {
		SDL_Texture *defaultTexture = gfx->texture;
		gfx->texture = GAME->sdlTextureMask;
		ComponentGraphic_DefaultDraw(gfx);
		gfx->texture = defaultTexture;
		obj->ex->enemy.onHitColorModTtl -= GAME->deltaTicks / 1000.0f;
	} else {
		ComponentGraphic_DefaultDraw(gfx);
	}
	ComponentDefense* defense = Object_GetDefense(obj); M2ASSERT(defense);
	ComponentGraphic_DefaultDrawHealthBar(gfx, (float) defense->hp / defense->maxHp);
}

int ObjectEnemy_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position) {
	M2ERR_REFLECT(Object_Init(obj, position, true));
	M2ERR_REFLECT(CharacterState_Init(&obj->ex->enemy.characterState, cfg));

	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = cfg->mainTexture->textureRect;
	gfx->center_px = cfg->mainTexture->objCenter_px;
	gfx->draw = ObjectEnemy_Draw;
	gfx->motionBlurEnabled = true;
	gfx->prevObjGfxOriginWRTScreenCenter_px = ComponentGraphic_GraphicsOriginWRTScreenCenter_px(position, cfg->mainTexture->objCenter_px);

	AI* ai = malloc(sizeof(AI)); // TODO error check, who uses malloc anyways
	AI_Init(ai);
	ai->recalculationPeriod = 500;
	ai->recalculationStopwatch = rand() % ai->recalculationPeriod;
	ai->attackPeriod = 500;
	ai->attackStopwatch = rand() % ai->attackPeriod;
	ai->homePosition = position;
	ai->triggerDistance = 6.0f;
	obj->ex->enemy.ai = ai;

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->prePhysics = ObjectEnemy_prePhysics;
	el->postPhysics = ObjectEnemy_postPhysics;

	ComponentPhysique* phy = Object_AddPhysique(obj);
	phy->body = Box2DUtils_CreateDynamicDisk(
		Pool_GetId(&GAME->physics, phy),
		position,
		true, // allowSleep
		CATEGORY_ENEMY,
		cfg->mainTexture->collider.colliderUnion.circ.radius_m,
		cfg->mass_kg,
		cfg->linearDamping
	);

	ComponentDefense* defense = Object_AddDefense(obj);
	defense->hp = 100;
	defense->maxHp = 100;
	defense->onHit = ObjectEnemy_onHit;
	defense->onDeath = ObjectEnemy_onDeath;

	// Initialise character state after components
	// Character states may access components during initialisation

	AutomatonCharAnimation_Init(&obj->ex->enemy.charAnimationAutomaton, cfg, gfx);

	if (cfg->ai) {
		AiState_Init(&obj->ex->enemy.aiState, cfg->ai, position);
		switch (cfg->ai->behavior) {
			case CFG_AI_BEHAVIOR_CHASE:
				M2ERR_REFLECT(AutomatonAiChase_Init(&obj->ex->enemy.aiAutomaton, obj, phy));
				break;
			case CFG_AI_BEHAVIOR_KEEP_DISTANCE:
				M2ERR_REFLECT(AutomatonAiKeepDistance_Init(&obj->ex->enemy.aiAutomaton, obj, phy));
				break;
			case CFG_AI_BEHAVIOR_HIT_N_RUN:
				M2ERR_REFLECT(AutomatonAiHitNRun_Init(&obj->ex->enemy.aiAutomaton, obj, phy));
				break;
			default:
				break;
		}
	}

	return 0;
}
