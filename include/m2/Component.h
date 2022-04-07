#ifndef COMPONENT_H
#define COMPONENT_H

#include "Def.hh"
#include <m2/Vec2f.h>
#include <b2_body.h>
#include <SDL.h>

namespace m2 {
	struct Component {
		ID object_id;

		Component() = default;

		explicit Component(ID object_id);
	};
}

#endif
