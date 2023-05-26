#include <m2/component/SoundEmitter.h>
#include <m2/Game.h>

m2::SoundEmitter::SoundEmitter(Id object_id) : Component(object_id) {}

float m2::SoundListener::volume_of(const VecF& emitter_position) const {
	auto distance = emitter_position.distance(position);
	if (GAME.max_hearing_distance_m < distance) {
		return 0.0f;
	}

	// Calculate volume drop-off based on distance
	auto hear_ratio = (GAME.max_hearing_distance_m - distance) / GAME.max_hearing_distance_m;

	// Check if angle difference to hearing edge is less than zero
	auto listener_to_sound_vector = emitter_position - position;
	auto angle_to_sound = listener_to_sound_vector.angle_rads();
	auto angle_diff_to_sound_backwards = fabs(angle_to_sound - direction);
	auto angle_diff_to_sound_forwards = fabs(angle_to_sound + PI_MUL2 - direction);
	auto angle_diff_to_sound = std::min(angle_diff_to_sound_backwards, angle_diff_to_sound_forwards);
	auto angle_diff_to_hearing_edge = angle_diff_to_sound - listen_angle / 2.0f;
	if (angle_diff_to_hearing_edge <= 0.0f) {
		// Full hearing, apply distance
		return hear_ratio;
	} else {
		// Apply both distance and angle
		auto volume_due_to_angle = std::lerp(GAME.min_hearing_facing_away, 1.0f, (PI - angle_diff_to_hearing_edge) / PI);
		return hear_ratio * volume_due_to_angle;
	}
}
