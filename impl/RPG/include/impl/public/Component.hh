#ifndef IMPL_COMPONENT_HH
#define IMPL_COMPONENT_HH

#include "impl/private/ARPG_Cfg.hh"
#include "m2/Component.h"

namespace impl {
    struct Defense : public m2::Defense {
        float maxHp;
        float hp;
        void (*onHit)(Defense*);
        void (*onDeath)(Defense*);

        Defense() = default;
        explicit Defense(ID object_id);
    };

    struct Offense : public m2::Offense {
        ID originator;
        union {
            ProjectileState projectile;
            MeleeState melee;
            ExplosiveState explosive;
        } state;

        Offense() = default;
        explicit Offense(ID object_id);
    };
}

#endif //IMPL_COMPONENT_HH
