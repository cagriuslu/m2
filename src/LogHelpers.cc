#include <m2/LogHelpers.h>
#include <m2/Log.h>
#include <m2/Game.h>

bool m2::detail::IsDebugEnabledForObject(const ObjectId id) {
	return Game::HasInstance() && Game::Instance().HasLevel() && Game::Instance().GetLevel().IsDebugEnabledForObject(id);
}
