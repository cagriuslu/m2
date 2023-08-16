#pragma once
#include "../Meta.h"
#include "../ui/Blueprint.h"
#include "../ui/State.h"
#include <m2g_SpriteType.pb.h>
#include <Sprite.pb.h>
#include <filesystem>

namespace m2::sedit {
	struct State {
		pb::SpriteSheets sheets_pb;
		std::optional<int> selected_sheet;

		static m2::expected<State> create(const std::filesystem::path& path);

		std::unique_ptr<ui::State> add_sheet_dialog;
		ui::Action execute_add_sheet_dialog(bool edit = false);
		void_expected add_sheet(const std::string& resource, unsigned ppm, bool edit = false);

	private:
		explicit State(pb::SpriteSheets);
	};
}
