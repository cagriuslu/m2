#include <rpg/model/SaveSlot.h>
#include <rpg/PowerUp.h>
#include <algorithm>

bool PlayerSaveData::validate() const {
	return std::all_of(power_up_indexes.begin(), power_up_indexes.end(),
		[](long l) { return itm::POWERUP_NONE < l && l < itm::POWERUP_N; });
}

bool ChapterSaveData::validate() const {
	return std::all_of(stage_levels.begin(), stage_levels.end(), [](long l) { return 0 <= l; }) &&
		std::all_of(min_completion_times_s.begin(), min_completion_times_s.end(), [](double d) { return 0.0 <= d; });
}

bool SaveData::validate() const {
	return 0 <= points && player.validate() &&
		std::all_of(chapters.begin(), chapters.end(), [](const auto& c) { return c.validate(); });
}
