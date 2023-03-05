#ifndef M2_SOUND_H
#define M2_SOUND_H

#include "../Component.h"
#include "../AudioManager.h"
#include <functional>
#include <list>

namespace m2 {
	struct Sound : public Component {
		std::function<void(Sound&)> on_update{};

		std::list<PlaybackId> playbacks;

		Sound() = default;
		explicit Sound(Id object_id);
	};
}

#endif //M2_SOUND_H
