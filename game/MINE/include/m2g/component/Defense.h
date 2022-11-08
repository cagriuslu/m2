#ifndef MINE_DEFENSE_H
#define MINE_DEFENSE_H

#include <m2/component/Defense.h>

namespace m2g::comp {
	struct Defense : public m2::comp::Defense {
		Defense() = default;
		explicit Defense(m2::Id object_id);
	};
}

#endif //MINE_DEFENSE_H