#include <m2/mt/Actor.h>

m2::ActorLifetimeManager::ActorLifetimeManager() : _isRunning(true) {}

bool m2::ActorLifetimeManager::IsRunning() const {
	bool readResult;
	_isRunning.Read([&readResult](const bool& isRunning) { readResult = isRunning; });
	return readResult;
}

void m2::ActorLifetimeManager::RequestStop() {
	_isRunning.Write([](bool& isRunning) { isRunning = false; });
}
