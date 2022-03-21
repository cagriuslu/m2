#ifndef GAME_COMPONENT_HH
#define GAME_COMPONENT_HH

#include "ARPG_Cfg.hh"
#include <m2/Component.hh>

namespace game {
    struct component_defense : public m2::component_defense {
        float maxHp;
        float hp;
        void (*onHit)(component_defense*);
        void (*onDeath)(component_defense*);

        component_defense();
        explicit component_defense(ID object_id);
    };

    struct component_offense : public m2::component_offense {
        ID originator;
        union {
            ProjectileState projectile;
            MeleeState melee;
            ExplosiveState explosive;
        } state;

        component_offense();
        explicit component_offense(ID object_id);
    };
}

#endif //GAME_COMPONENT_HH
