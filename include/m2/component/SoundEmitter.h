#ifndef M2_SOUNDEMITTER_H
#define M2_SOUNDEMITTER_H

#include "../Component.h"
#include "../AudioManager.h"
#include "../Vec2f.h"
#include <functional>
#include <list>

namespace m2 {
	struct SoundEmitter : public Component {
		std::function<void(SoundEmitter&)> on_update{};

		std::list<PlaybackId> playbacks;

		SoundEmitter() = default;
		explicit SoundEmitter(Id object_id);
	};

	struct SoundListener {
		Vec2f position;
		float direction{}; // [0,2PI]
		float listen_angle{PI_MUL2}; // [0,2PI] Minimum angle to omnidirectional hearing
	};
}

#endif //M2_SOUNDEMITTER_H
