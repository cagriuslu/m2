#ifndef M2_ARPG_OBJECT_HH
#define M2_ARPG_OBJECT_HH

#include <m2/object.hh>
#include <game/ARPG_Cfg.hh>
#include <game/automaton/chase.hh>

M2Err ObjectExplosive_InitFromCfg(m2::object::Object* obj, const CfgExplosive* cfg, ID originatorId, m2::vec2f position, m2::vec2f direction);

int ObjectMelee_InitFromCfg(m2::object::Object* obj, const CfgMelee *cfg, ID originatorId, m2::vec2f position, m2::vec2f direction);

typedef struct {
	CharacterState characterState;
	Automaton charAnimationAutomaton;
} PlayerData;
#define AS_PLAYERDATA(ptr) ((PlayerData*)(ptr))

int ObjectPlayer_InitFromCfg(m2::object::Object* obj, const CfgCharacter *cfg, m2::vec2f position);

int ObjectProjectile_InitFromCfg(m2::object::Object* obj, const CfgProjectile *cfg, ID originatorId, m2::vec2f position, m2::vec2f direction);

#endif //M2_ARPG_OBJECT_HH
