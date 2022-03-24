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

#endif
