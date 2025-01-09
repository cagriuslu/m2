#include <cuzn/ui/Detail.h>
#include <m2/Level.h>
#include <m2/Game.h>
#include <cuzn/journeys/BuildJourney.h>

using namespace m2;

m2::RGB generate_player_color(unsigned index) {
	switch (index) {
		case 0:
			return m2::RGB{0, 255, 255};
		case 1:
			return m2::RGB{255, 0, 255};
		case 2:
			return m2::RGB{255, 255, 0};
		case 3:
			return m2::RGB{0, 255, 0};
		default:
			throw M2_ERROR("Invalid player index");
	}
}

std::string generate_player_name(unsigned index) {
	switch (index) {
		case 0:
			return "Cyan";
		case 1:
			return "Pink";
		case 2:
			return "Yellow";
		case 3:
			return "Green";
		default:
			throw M2_ERROR("Invalid player index");
	}
}
