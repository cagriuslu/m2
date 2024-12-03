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

std::string build_notification(Industry industry, City city) {
	return "Built " + industry_name(industry) + " on " + city_name(city) + ".";
}
std::string network_notification(City connection1_city1, City connection1_city2, City connection2_city1, City connection2_city2) {
	if (connection2_city1 && connection2_city2) {
		return "Built one " + canal_or_railroad() + " between " + city_name(connection1_city1) + " and " + city_name(connection1_city2) + ", and another " + canal_or_railroad() + " between " + city_name(connection2_city1) + " and " + city_name(connection2_city2) + ".";
	} else {
		return "Built " + canal_or_railroad() + " between " + city_name(connection1_city1) + " and " + city_name(connection1_city2) + ".";
	}
}
std::string sell_notification(Industry industry, City city) {
	return "Sold " + industry_name(industry) + " on " + city_name(city) + ".";
}
std::string loan_notification() {
	return "Took loan.";
}
std::string develop_notification(Industry industry1, Industry industry2) {
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
std::string scout_notification() {
	return "Scouted for wild cards.";
}
std::string pass_notification() {
	return "Skipped action.";
}
