#include <m2/LogHelpers.h>
#include <m2/Log.h>
#include <m2/Game.h>

bool m2::detail::IsDebugLoggingEnabledForObject(const ObjectId id) {
	if (Game::HasInstance() && Game::Instance().HasLevel()) {
		if (const auto* debugOptions = Game::Instance().GetLevel().GetObjectDebugOptions(id)) {
			return debugOptions->logging;
		}
	}
	return false;
}
