#include <m2/multi_player/lockstep/LevelSaver.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

LevelSaver::LevelSaver(std::unique_ptr<genORM::database> db) : _db(std::move(db)) {}

const char* LevelSaver::ThreadNameForLogging() const { return "LS"; }

bool LevelSaver::operator()(MessageBox<LevelSaverInput>& inbox, MessageBox<LevelSaverOutput>&) {
	// TODO
	return true;
}
