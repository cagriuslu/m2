#include <cuzn/detail/ActionNotification.h>
#include <m2/Game.h>
#include <cuzn/object/HumanPlayer.h>

namespace {
	std::string industry_name(Industry industry) {
		return M2_GAME.GetNamedItem(industry).in_game_name();
	}
	std::string city_name(City city) {
		return M2_GAME.GetNamedItem(city).in_game_name();
	}
	std::string canal_or_railroad() {
		return M2G_PROXY.is_canal_era() ? "canal" : "railroad";
	}
}

std::string GenerateBuildNotification(Industry industry, City city) {
	return "Built " + industry_name(industry) + " on " + city_name(city) + ".";
}
std::string GenerateNetworkNotification(City connection1City1, City connection1City2, City connection2City1, City connection2City2) {
	if (connection2City1 && connection2City2) {
		return "Built one " + canal_or_railroad() + " between " + city_name(connection1City1) + " and " + city_name(connection1City2) + ", and another " + canal_or_railroad() + " between " + city_name(connection2City1) + " and " + city_name(connection2City2) + ".";
	} else {
		return "Built " + canal_or_railroad() + " between " + city_name(connection1City1) + " and " + city_name(connection1City2) + ".";
	}
}
std::string GenerateSellNotification(Industry industry, City city) {
	return "Sold " + industry_name(industry) + " on " + city_name(city) + ".";
}
std::string GenerateLoanNotification() {
	return "Took loan.";
}
std::string GenerateDevelopNotification(Industry industry1, Industry industry2) {
	if (industry2) {
		if (industry1 == industry2) {
			return "Developed two " + industry_name(industry1) + " tiles.";
		} else {
			return "Developed a " + industry_name(industry1) + " and a " + industry_name(industry2) + " tiles.";
		}
	} else {
		return "Developed a " + industry_name(industry1) + " tile.";
	}
}
std::string GenerateScoutNotification() {
	return "Scouted for wild cards.";
}
std::string GeneratePassNotification() {
	return "Skipped action.";
}
