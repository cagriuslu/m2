#include <cuzn/object/GameStateTracker.h>
#include <m2/Game.h>
#include <cuzn/detail/Market.h>

void InitGameStateTracker(m2::Object& obj) {
	auto& chr = obj.add_full_character();
	// Hold the resources belonging to the market
	chr.SetResource(m2g::pb::COAL_CUBE_COUNT, COAL_MARKET_INITIAL_COUNT);
	chr.SetResource(m2g::pb::IRON_CUBE_COUNT, IRON_MARKET_INITIAL_COUNT);
	// Era
	chr.ClearResource(m2g::pb::IS_RAILROAD_ERA);
	// Turn
	chr.SetResource(m2g::pb::IS_LAST_ACTION_OF_PLAYER, 1.0f);
}
