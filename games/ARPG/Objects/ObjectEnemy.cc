#include "m2/Object.hh"
#include "m2/Game.hh"
#include "m2/Box2D.hh"
#include "m2/Def.hh"
#include "m2/Box2DUtils.hh"
#include "../ARPG_Object.hh"
#include <game/ARPG_Cfg.hh>
#include <game/component.hh>

game::ObjectDataEnemy::ObjectDataEnemy(Object& obj) : automata_ai_chase(obj) {}

void ObjectEnemy_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId);
    auto* data = dynamic_cast<game::ObjectDataEnemy*>(obj->data_new.get());

	CharacterState_ProcessTime(&(AS_ENEMYDATA(obj->data)->characterState), GAME.deltaTime_s);
    data->automata_ai_chase.time(GAME.deltaTime_s);
    data->automata_ai_chase.signal(SIG_AI_PREPHYSICS);
	//Automaton_ProcessTime(&(AS_ENEMYDATA(obj->data)->aiAutomaton), GAME.deltaTime_s);
	//Automaton_ProcessSignal(&(AS_ENEMYDATA(obj->data)->aiAutomaton), SIG_AI_PREPHYSICS);
}

void ObjectEnemy_onHit(game::component_defense* def) {
	Object* obj = GAME.objects.get(def->super.objId);
	AS_ENEMYDATA(obj->data)->onHitColorModTtl = 0.10f;
}

void ObjectEnemy_onDeath(game::component_defense* def) {
	Game_DeleteList_Add(def->super.objId);
}

static void ObjectEnemy_postPhysics(ComponentMonitor* monitor) {
	Object* obj = GAME.objects.get(monitor->super.objId);
	ComponentPhysique* phy = GAME.physics.get(obj->physique);
	// We must call time before other signals
	Automaton_ProcessTime(&(AS_ENEMYDATA(obj->data)->charAnimationAutomaton), GAME.deltaTicks_ms / 1000.0f);
	m2::vec2f velocity = m2::vec2f{ phy->body->GetLinearVelocity() };
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
		gfx->texture = GAME.sdlTextureMask;
		ComponentGraphic_DefaultDraw(gfx);
		gfx->texture = defaultTexture;
		AS_ENEMYDATA(obj->data)->onHitColorModTtl -= GAME.deltaTicks_ms / 1000.0f;
	} else {
		ComponentGraphic_DefaultDraw(gfx);
	}
	game::component_defense* defense = Object_GetDefense(obj); M2ASSERT(defense);
	ComponentGraphic_DefaultDrawHealthBar(gfx, (float) defense->hp / defense->maxHp);
}

int ObjectEnemy_InitFromCfg(Object* obj, const CfgCharacter *cfg, m2::vec2f position) {
	M2ERR_REFLECT(Object_Init(obj, position));
    obj->data = new EnemyData;
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
		GAME.physics.get_id(phy),
		position,
		true, // allowSleep
		CATEGORY_ENEMY,
		ARPG_CFG_SPRITES[cfg->mainSpriteIndex].collider.params.circ.radius_m,
		cfg->mass_kg,
		cfg->linearDamping
	);

	game::component_defense* defense = Object_AddDefense(obj);
    defense->hp = 100;
    defense->maxHp = 100;
    defense->onHit = ObjectEnemy_onHit;
    defense->onDeath = ObjectEnemy_onDeath;

	// Initialise character state after components
	// Character states may access components during initialisation

	AutomatonCharAnimation_Init(&(AS_ENEMYDATA(obj->data)->charAnimationAutomaton), cfg, gfx);

	if (cfg->ai) {
		AiState_Init(&(AS_ENEMYDATA(obj->data)->aiState), cfg->ai, position);
		switch (cfg->ai->behavior) {
			case CFG_AI_BEHAVIOR_CHASE:
				// No need, cleanup around here
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

    obj->data_new = std::make_unique<game::ObjectDataEnemy>(*obj);
	return 0;
}
