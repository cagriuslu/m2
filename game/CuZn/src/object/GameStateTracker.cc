#include <cuzn/object/GameStateTracker.h>
#include <m2/Game.h>
#include <cuzn/detail/Market.h>
#include <m2/ObjectEx.h>

void InitGameStateTracker(m2::Object& obj) {
	auto& chr = m2::AddCharacterToObject<m2g::ProxyEx::FastCharacterStorageIndex>(obj);
	// Hold the resources belonging to the market
	chr.SetVariable(m2g::pb::COAL_CUBE_COUNT, m2::IVFE{COAL_MARKET_INITIAL_COUNT});
	chr.SetVariable(m2g::pb::IRON_CUBE_COUNT, m2::IVFE{IRON_MARKET_INITIAL_COUNT});
	// Era
	chr.ClearVariable(m2g::pb::IS_RAILROAD_ERA);
	// Turn
	chr.SetVariable(m2g::pb::IS_LAST_ACTION_OF_PLAYER, m2::IVFE{1});
}
