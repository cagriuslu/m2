#ifndef IMPL_POWERUP_H
#define IMPL_POWERUP_H

namespace itm {
	enum PowerUpIndex {
		POWERUP_NONE = 0,
		// TODO
		POWERUP_N
	};

	struct PowerUp {
		PowerUpIndex index;
		// TODO
	};

	inline const unsigned power_up_count = POWERUP_N;
	inline const PowerUp power_ups[] = {
		{
			.index = POWERUP_NONE
		},
		// TODO
	};
}

#endif //IMPL_POWERUP_H
