#ifndef COMPONENT_H
#define COMPONENT_H

#include <m2/Cfg.hh>
#include <m2/Vec2f.h>
#include <m2/Box2D.hh>
#include <b2_body.h>
#include <SDL.h>
#include <stdint.h>

struct Component {
	ID object_id;

	Component() = default;
	explicit Component(ID object_id);
};

namespace m2 {
    struct Defense : public Component {
        Defense() = default;
        explicit Defense(ID object_id);
    };

    struct Offense : public Component {
        Offense() = default;
        explicit Offense(ID object_id);
    };
}

#endif
