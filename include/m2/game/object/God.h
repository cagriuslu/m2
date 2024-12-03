#pragma once
#include "m2/Object.h"

namespace m2::obj {
	struct God : ObjectImpl {
		int dummy;
	};

	Id create_god();
}
