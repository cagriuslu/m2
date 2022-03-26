#include <impl/private/object/Enemy.h>
#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/Box2D.hh"
#include "m2/Def.hh"
#include "impl/private/ARPG_Cfg.hh"
#include "impl/public/Component.hh"
#include <m2/box2d/Utils.h>

impl::object::Enemy::Enemy(m2::Object& obj) : chaser({obj, GAME.objects[GAME.playerId]}) {}

static void ObjectEnemy_prePhysics(m2::component::Monitor& mon) {
	auto& obj = GAME.objects[mon.object_id];
    auto* data = dynamic_cast<impl::object::Enemy*>(obj.impl.get());

	CharacterState_ProcessTime(&(AS_ENEMYDATA(obj.data)->characterState), GAME.deltaTime_s);
    data->chaser.time(GAME.deltaTime_s);
    data->chaser.signal(m2::FSMSIG_PREPHY);
}

static void ObjectEnemy_onHit(impl::Defense* def) {
	auto& obj = GAME.objects[def->object_id];
	AS_ENEMYDATA(obj.data)->onHitColorModTtl = 0.10f;
}

static void ObjectEnemy_onDeath(impl::Defense* def) {
	Game_DeleteList_Add(def->object_id);
}

static void ObjectEnemy_postPhysics(m2::component::Monitor& monitor) {
	auto& obj = GAME.objects[monitor.object_id];
	auto& phy = GAME.physics[obj.physique_id];
	// We must call time before other signals
	Automaton_ProcessTime(&(AS_ENEMYDATA(obj.data)->charAnimationAutomaton), GAME.deltaTicks_ms / 1000.0f);
	m2::Vec2f velocity = m2::Vec2f{phy.body->GetLinearVelocity() };
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

static void ObjectEnemy_Draw(m2::component::Graphic& gfx) {
	auto& obj = GAME.objects[gfx.object_id];
	if (0.0f < AS_ENEMYDATA(obj.data)->onHitColorModTtl) {
		SDL_Texture *defaultTexture = gfx.texture;
		gfx.texture = GAME.sdlTextureMask;
		m2::component::Graphic::default_draw(gfx);
		gfx.texture = defaultTexture;
		AS_ENEMYDATA(obj.data)->onHitColorModTtl -= GAME.deltaTicks_ms / 1000.0f;
	} else {
		m2::component::Graphic::default_draw(gfx);
	}
	auto& def = obj.defense();
	m2::component::Graphic::default_draw_healthbar(gfx, (float) def.hp / def.maxHp);
}

int ObjectEnemy_InitFromCfg(m2::Object* obj, const CfgCharacter *cfg, m2::Vec2f position) {
	*obj = m2::Object{position};
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
	phy.body = m2::box2d::create_dynamic_disk(
            *GAME.world,
            obj->physique_id,
            position,
            false,
            true,
            m2::box2d::CATEGORY_ENEMY,
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
	}

    obj->impl = std::make_unique<impl::object::Enemy>(*obj);
	return 0;
}
