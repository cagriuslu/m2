#include <m2/object/Object.hh>
#include "m2/Game.hh"
#include "m2/Box2D.hh"
#include "m2/Def.hh"
#include "m2/Box2DUtils.hh"
#include "../ARPG_Object.hh"
#include <game/ARPG_Cfg.hh>
#include <game/component.hh>

game::object::ObjectDataEnemy::ObjectDataEnemy(m2::object::Object& obj) : automata_ai_chase(obj) {}

void ObjectEnemy_prePhysics(Monitor& mon) {
	auto& obj = GAME.objects[mon.object_id];
    auto* data = dynamic_cast<game::object::ObjectDataEnemy*>(obj.data_new.get());

	CharacterState_ProcessTime(&(AS_ENEMYDATA(obj.data)->characterState), GAME.deltaTime_s);
    data->automata_ai_chase.time(GAME.deltaTime_s);
    data->automata_ai_chase.signal(SIG_AI_PREPHYSICS);
}

void ObjectEnemy_onHit(game::component_defense* def) {
	auto& obj = GAME.objects[def->object_id];
	AS_ENEMYDATA(obj.data)->onHitColorModTtl = 0.10f;
}

void ObjectEnemy_onDeath(game::component_defense* def) {
	Game_DeleteList_Add(def->object_id);
}

static void ObjectEnemy_postPhysics(Monitor& monitor) {
	auto& obj = GAME.objects[monitor.object_id];
	auto& phy = GAME.physics[obj.physique_id];
	// We must call time before other signals
	Automaton_ProcessTime(&(AS_ENEMYDATA(obj.data)->charAnimationAutomaton), GAME.deltaTicks_ms / 1000.0f);
	m2::vec2f velocity = m2::vec2f{ phy.body->GetLinearVelocity() };
	if (fabsf(velocity.x) < 0.5000f && fabsf(velocity.y) < 0.5000f) {
		Automaton_ProcessSignal(&(AS_ENEMYDATA(obj.data)->charAnimationAutomaton), SIG_CHARANIM_STOP);
	} else if (fabsf(velocity.x) < fabsf(velocity.y)) {
		if (0 < velocity.y) {
			Automaton_ProcessSignal(&(AS_ENEMYDATA(obj.data)->charAnimationAutomaton), SIG_CHARANIM_WALKDOWN);
		} else {
			Automaton_ProcessSignal(&(AS_ENEMYDATA(obj.data)->charAnimationAutomaton), SIG_CHARANIM_WALKUP);
		}
	} else {
		if (0 < velocity.x) {
			Automaton_ProcessSignal(&(AS_ENEMYDATA(obj.data)->charAnimationAutomaton), SIG_CHARANIM_WALKRIGHT);
		} else {
			Automaton_ProcessSignal(&(AS_ENEMYDATA(obj.data)->charAnimationAutomaton), SIG_CHARANIM_WALKLEFT);
		}
	}
}

void ObjectEnemy_Draw(Graphic& gfx) {
	auto& obj = GAME.objects[gfx.object_id];
	if (0.0f < AS_ENEMYDATA(obj.data)->onHitColorModTtl) {
		SDL_Texture *defaultTexture = gfx.texture;
		gfx.texture = GAME.sdlTextureMask;
		Graphic::default_draw(gfx);
		gfx.texture = defaultTexture;
		AS_ENEMYDATA(obj.data)->onHitColorModTtl -= GAME.deltaTicks_ms / 1000.0f;
	} else {
		Graphic::default_draw(gfx);
	}
	auto& def = obj.defense();
	Graphic::default_draw_healthbar(gfx, (float) def.hp / def.maxHp);
}

int ObjectEnemy_InitFromCfg(m2::object::Object* obj, const CfgCharacter *cfg, m2::vec2f position) {
	*obj = m2::object::Object{position};
    obj->data = new EnemyData();
	M2ERR_REFLECT(CharacterState_Init(&(AS_ENEMYDATA(obj->data)->characterState), cfg));

	auto& gfx = obj->add_graphic();
	gfx.textureRect = ARPG_CFG_SPRITES[cfg->mainSpriteIndex].textureRect;
	gfx.center_px = ARPG_CFG_SPRITES[cfg->mainSpriteIndex].objCenter_px;
	gfx.draw = ObjectEnemy_Draw;

	auto& mon = obj->add_monitor();
	mon.prePhysics = ObjectEnemy_prePhysics;
	mon.postPhysics = ObjectEnemy_postPhysics;

	auto& phy = obj->add_physique();
	phy.body = Box2DUtils_CreateDynamicDisk(
		obj->physique_id,
		position,
		true, // allowSleep
		CATEGORY_ENEMY,
		ARPG_CFG_SPRITES[cfg->mainSpriteIndex].collider.params.circ.radius_m,
		cfg->mass_kg,
		cfg->linearDamping
	);

	auto& def = obj->add_defense();
	def.hp = 100;
	def.maxHp = 100;
	def.onHit = ObjectEnemy_onHit;
	def.onDeath = ObjectEnemy_onDeath;

	// Initialise character state after components
	// Character states may access components during initialisation

	AutomatonCharAnimation_Init(&(AS_ENEMYDATA(obj->data)->charAnimationAutomaton), cfg, &gfx);

	if (cfg->ai) {
		AiState_Init(&(AS_ENEMYDATA(obj->data)->aiState), cfg->ai, position);
		switch (cfg->ai->behavior) {
			case CFG_AI_BEHAVIOR_CHASE:
				// No need, cleanup around here
				break;
			case CFG_AI_BEHAVIOR_KEEP_DISTANCE:
				M2ERR_REFLECT(AutomatonAiKeepDistance_Init(&(AS_ENEMYDATA(obj->data)->aiAutomaton), obj, &phy));
				break;
			case CFG_AI_BEHAVIOR_HIT_N_RUN:
				M2ERR_REFLECT(AutomatonAiHitNRun_Init(&(AS_ENEMYDATA(obj->data)->aiAutomaton), obj, &phy));
				break;
			default:
				break;
		}
	}

    obj->data_new = std::make_unique<game::object::ObjectDataEnemy>(*obj);
	return 0;
}
