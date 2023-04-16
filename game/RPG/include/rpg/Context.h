#ifndef RPG_CONTEXT_H
#define RPG_CONTEXT_H

#include <Progress.pb.h>
#include <filesystem>

namespace rpg {
	struct Context {
		std::filesystem::path progress_file_path;
		pb::Progress progress;

		Context();

		void save_progress() const;
	};
}

#endif //RPG_CONTEXT_H
