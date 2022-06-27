#ifndef M2_GOD_H
#define M2_GOD_H

#include <m2/Object.h>

namespace m2::obj {
	struct God : public ObjectImpl {
		int dummy;
	};

	std::pair<Object&, ID> create_god();
}

#endif //M2_GOD_H
