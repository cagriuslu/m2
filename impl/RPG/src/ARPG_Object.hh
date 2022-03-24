#ifndef M2_ARPG_OBJECT_HH
#define M2_ARPG_OBJECT_HH

#include <m2/Object.h>
#include <impl/ARPG_Cfg.hh>
#include <impl/fsm/Chaser.h>

M2Err ObjectExplosive_InitFromCfg(m2::Object* obj, const CfgExplosive* cfg, ID originatorId, m2::Vec2f position, m2::Vec2f direction);

int ObjectMelee_InitFromCfg(m2::Object* obj, const CfgMelee *cfg, ID originatorId, m2::Vec2f position, m2::Vec2f direction);

typedef struct {
	CharacterState characterState;
	Automaton charAnimationAutomaton;
} PlayerData;
#define AS_PLAYERDATA(ptr) ((PlayerData*)(ptr))

int ObjectPlayer_InitFromCfg(m2::Object* obj, const CfgCharacter *cfg, m2::Vec2f position);

int ObjectProjectile_InitFromCfg(m2::Object* obj, const CfgProjectile *cfg, ID originatorId, m2::Vec2f position, m2::Vec2f direction);

#endif //M2_ARPG_OBJECT_HH
