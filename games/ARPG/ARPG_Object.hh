#ifndef M2_ARPG_OBJECT_HH
#define M2_ARPG_OBJECT_HH

#include <m2/Object.hh>
#include <game/ARPG_Cfg.hh>
#include "Automata/AutomatonAiChase.hh"

struct EnemyData {
	CharacterState characterState;
	Automaton charAnimationAutomaton;
	AiState aiState;
	Automaton aiAutomaton;
	float onHitColorModTtl;
};
#define AS_ENEMYDATA(ptr) ((EnemyData*)(ptr))

namespace game {
    struct ObjectDataEnemy : public ObjectData {
        game::AutomatonAiChase automata_ai_chase;

        explicit ObjectDataEnemy(Object&);
    };
}

int ObjectEnemy_InitFromCfg(Object* obj, const CfgCharacter *cfg, m2::vec2f position);

M2Err ObjectExplosive_InitFromCfg(Object* obj, const CfgExplosive* cfg, ID originatorId, m2::vec2f position, m2::vec2f direction);

int ObjectMelee_InitFromCfg(Object* obj, const CfgMelee *cfg, ID originatorId, m2::vec2f position, m2::vec2f direction);

typedef struct {
	CharacterState characterState;
	Automaton charAnimationAutomaton;
} PlayerData;
#define AS_PLAYERDATA(ptr) ((PlayerData*)(ptr))

int ObjectPlayer_InitFromCfg(Object* obj, const CfgCharacter *cfg, m2::vec2f position);

int ObjectProjectile_InitFromCfg(Object* obj, const CfgProjectile *cfg, ID originatorId, m2::vec2f position, m2::vec2f direction);

#endif //M2_ARPG_OBJECT_HH
