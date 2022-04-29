#include <rpg/model/SaveSlot.h>
#include <rpg/PowerUp.h>
#include <algorithm>

bool PlayerSaveData::validate() const {
	return std::ranges::all_of(power_up_indexes, [](long l) { return itm::POWERUP_NONE < l && l < itm::POWERUP_N; });
}

bool ChapterSaveData::validate() const {
	return std::ranges::all_of(stage_levels, [](long l) { return 0 <= l; }) &&
		std::ranges::all_of(min_completion_times_s, [](double d) { return 0.0 <= d; });
}

bool SaveData::validate() const {
	return 0 <= points && player.validate() &&
		std::ranges::all_of(chapters, [](const auto& c) { return c.validate(); });
}
