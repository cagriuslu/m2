#ifndef M2_ARPG_COMPONENT_HH
#define M2_ARPG_COMPONENT_HH

#include <m2/Component.hh>
#include "ARPG_Cfg.hh"

typedef struct {
	float maxHp;
	float hp;
	void (*onHit)(ComponentDefense*);
	void (*onDeath)(ComponentDefense*);
} ARPG_ComponentDefense;
#define AS_ARPG_COMPONENTDEFENSE(ptr) ((ARPG_ComponentDefense*)(ptr))

typedef struct {
	ID originator;
	union {
		ProjectileState projectile;
		MeleeState melee;
		ExplosiveState explosive;
	} state;
} ARPG_ComponentOffense;
#define AS_ARPG_COMPONENTOFFENSE(ptr) ((ARPG_ComponentOffense*)(ptr))

#endif //M2_ARPG_COMPONENT_HH
