#ifndef MINE_OFFENSE_H
#define MINE_OFFENSE_H

#include <m2/component/Offense.h>

namespace m2g::comp {
	struct Offense : public m2::comp::Offense {
		Offense() = default;
		explicit Offense(m2::Id object_id);
	};
}

#endif //MINE_OFFENSE_H
