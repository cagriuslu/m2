#pragma once
#include "../Component.h"
#include "../audio/AudioManager.h"
#include "../math/VecF.h"
#include <m2/Math.h>
#include <functional>
#include <list>

namespace m2 {
	struct SoundEmitter : Component {
		std::function<void(SoundEmitter&, const Stopwatch::Duration& delta)> update{};

		std::list<PlaybackId> playbacks; // Playback should be queued to AudioManager, then the ID should be added here

		SoundEmitter() = default;
		explicit SoundEmitter(Id object_id);
	};

	struct SoundListener {
		VecF position;
		float direction{}; // [0,2PI]
		float listenAngle{PI_MUL2}; // [0,2PI] Minimum angle to omnidirectional hearing

		[[nodiscard]] float VolumeOf(const VecF& emitter_position) const;
	};
}
