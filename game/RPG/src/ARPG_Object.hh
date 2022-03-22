#ifndef M2_ARPG_OBJECT_HH
#define M2_ARPG_OBJECT_HH

#include <m2/object.hh>
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
	namespace object {
		struct ObjectDataEnemy : public m2::object::Data {
			game::AutomatonAiChase automata_ai_chase;

			explicit ObjectDataEnemy(m2::object::Object&);
		};
	}
}

int ObjectEnemy_InitFromCfg(m2::object::Object* obj, const CfgCharacter *cfg, m2::vec2f position);

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
