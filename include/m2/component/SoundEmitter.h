#pragma once
#include "../Component.h"
#include "../AudioManager.h"
#include "../VecF.h"
#include <functional>
#include <list>

namespace m2 {
	struct SoundEmitter : public Component {
		std::function<void(SoundEmitter&)> update{};

		std::list<PlaybackId> playbacks; // Playback should be queued to AudioManager, then the ID should be added here

		SoundEmitter() = default;
		explicit SoundEmitter(Id object_id);
	};

	struct SoundListener {
		VecF position;
		float direction{}; // [0,2PI]
		float listen_angle{PI_MUL2}; // [0,2PI] Minimum angle to omnidirectional hearing

		[[nodiscard]] float volume_of(const VecF& emitter_position) const;
	};
}
