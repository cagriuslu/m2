#ifndef M2_ARPG_OBJECT_HH
#define M2_ARPG_OBJECT_HH

#include <m2/Object.hh>
#include "ARPG_Cfg.hh"

typedef struct {
	CharacterState characterState;
	Automaton charAnimationAutomaton;
	AiState aiState;
	Automaton aiAutomaton;
	float onHitColorModTtl;
} EnemyData;
#define AS_ENEMYDATA(ptr) ((EnemyData*)(ptr))

int ObjectEnemy_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position);

M2Err ObjectExplosive_InitFromCfg(Object* obj, const CfgExplosive* cfg, ID originatorId, Vec2F position, Vec2F direction);

int ObjectMelee_InitFromCfg(Object* obj, const CfgMelee *cfg, ID originatorId, Vec2F position, Vec2F direction);

typedef struct {
	CharacterState characterState;
	Automaton charAnimationAutomaton;
} PlayerData;
#define AS_PLAYERDATA(ptr) ((PlayerData*)(ptr))

int ObjectPlayer_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position);

int ObjectProjectile_InitFromCfg(Object* obj, const CfgProjectile *cfg, ID originatorId, Vec2F position, Vec2F direction);

#endif //M2_ARPG_OBJECT_HH
