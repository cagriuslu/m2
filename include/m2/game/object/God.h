#pragma once
#include "m2/Object.h"

namespace m2::obj {
	struct God : public ObjectImpl {
		int dummy;
	};

	Id create_god();
}
