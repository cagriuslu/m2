#ifndef M2_SOUND_H
#define M2_SOUND_H

#include "../Component.h"
#include "../AudioManager.h"
#include "../Vec2f.h"
#include <functional>
#include <list>

namespace m2 {
	struct Sound : public Component {
		std::function<void(Sound&)> on_update{};

		std::list<PlaybackId> playbacks;

		Sound() = default;
		explicit Sound(Id object_id);
	};

	struct SoundListener {
		Vec2f position;
		float direction{}; // [0,2PI]
		float listen_angle{PI_MUL2}; // [0,2PI] Minimum angle to omnidirectional hearing
	};
}

#endif //M2_SOUND_H
