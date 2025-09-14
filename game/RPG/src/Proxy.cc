#include <m2/Proxy.h>
#include <rpg/group/ItemGroup.h>
#include <rpg/object/Enemy.h>
#include <rpg/object/Player.h>
#include <rpg/Objects.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/protobuf/Detail.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <m2/ui/widget/ProgressBar.h>
#include <rpg/Defs.h>

void m2g::Proxy::load_resources() {
	// Load enemies
	enemies = m2MoveOrThrowError(m2::pb::json_file_to_message<rpg::pb::Enemies>(M2_GAME.GetResources().GetGameResourceDir() / "Enemies.json"));
	// Load progress
	progress_file_path = M2_GAME.GetResources().GetGameResourceDir() / "Progress.json";
	auto expect_progress = m2::pb::json_file_to_message<rpg::pb::Progress>(progress_file_path);
	if (expect_progress) {
		progress.CopyFrom(*expect_progress);
	} else {
		LOG_INFO("Unable to load Progress file");
	}
}

void m2g::Proxy::post_single_player_level_init(MAYBE const std::string& name, const m2::pb::Level& level) {
	const auto& id = level.identifier();
	if (id == "WalkingTutorialClosed") {
		M2_LEVEL.ShowMessage("Use W,A,S,D to walk.");
	} else if (id == "WalkingTutorialOpen") {
		M2_LEVEL.ShowMessage("Some levels will be outdoors.");
	} else if (id == "FlagTutorialClosed") {
		M2_LEVEL.ShowMessage("Find the blue flag to exit the level.");
	} else if (id == "FlagTutorialOpen") {
		M2_LEVEL.ShowMessage("Find the blue flag to exit the level.");
	} else if (id == "DashTutorialClosed") {
		M2_LEVEL.ShowMessage("Use SPACE button while walking to dash.");
	} else if (id == "RangedWeaponTutorialClosed") {
		M2_LEVEL.ShowMessage("Use left mouse button to shoot bullets.");
	} else if (id == "MeleeTutorialClosed") {
		M2_LEVEL.ShowMessage("Use right mouse button to melee.");
	} else if (id == "AllMustBeKilledTutorialOpen") {
		M2_LEVEL.ShowMessage("All enemies must be killed to complete the level successfully.");
	}
}

m2::third_party::physics::FixtureDefinition m2g::Proxy::TileFixtureDefinition(MAYBE m2g::pb::SpriteType spriteType) {
	return m2::third_party::physics::FixtureDefinition{
		.colliderFilter = m2::third_party::physics::gColliderCategoryToParams[m2::I(m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_BACKGROUND_OBSTACLE)]
	};
}

m2::void_expected m2g::Proxy::LoadForegroundObjectFromLevelBlueprint(m2::Object& obj, const m2::VecF& position, float orientation) {
	using namespace rpg;
	switch (obj.GetType()) {
		case pb::ObjectType::PLAYER:
			return Player::init(obj, position);
		case pb::ObjectType::SKELETON:
			return Enemy::init(obj, position);
		case pb::ObjectType::CASTLE_FINISH_POINT:
			return rpg::init_finish_point(obj, position);
		case pb::ObjectType::CUTEOPUS:
			return Enemy::init(obj, position);
		case pb::ObjectType::MACHINE_GUN_ITEM_DROP:
			return rpg::create_dropped_item(obj, position, m2g::pb::ITEM_REUSABLE_MACHINE_GUN);
		case pb::ObjectType::EXPLOSIVE_ITEM_DROP:
			return rpg::create_dropped_item(obj, position, m2g::pb::ITEM_REUSABLE_EXPLOSIVE);
		case pb::ObjectType::LONG_SWORD_ITEM_DROP:
			return rpg::create_dropped_item(obj, position, m2g::pb::ITEM_REUSABLE_LONG_SWORD);
		case pb::ObjectType::AXE_ITEM_DROP:
			return rpg::create_dropped_item(obj, position, m2g::pb::ITEM_REUSABLE_AXE);
		case pb::BUSH_01:
		case pb::FLOWER_ORANGE_02:
		case pb::FLOWER_PINK_03:
		case pb::FLOWER_WHITE_05:
		case pb::PINE_01:
		case pb::PINE_DEAD_02:
		case pb::ROCK_03:
		case pb::TREE_03:
		case pb::TREE_DEAD_03:
		case pb::FENCE_HORIZONTAL:
		case pb::FENCE_VERTICAL:
			return rpg::create_decoration(obj, position, *M2_GAME.GetMainSpriteOfObject(obj.GetType()));
		case pb::SPIKES:
			return rpg::create_spikes(obj, position);
		default:
			return m2::make_unexpected("Unhandled object type");
	}
}

m2::Group* m2g::Proxy::create_group(pb::GroupType group_type) {
	switch (group_type) {
		case pb::GROUP_LOW_HP_POTION_DROPPER:
			return new rpg::ItemGroup({std::make_pair(pb::ITEM_CONSUMABLE_HP_POTION_20, 4.0f), std::make_pair(pb::ITEM_CONSUMABLE_HP_POTION_80, 1.0f)});
		case pb::GROUP_MACHINE_GUN_DROPPER:
			return new rpg::ItemGroup({{pb::ITEM_REUSABLE_MACHINE_GUN, 1.0f}});
		default:
			return nullptr;
	}
}

const rpg::pb::Enemy* m2g::Proxy::get_enemy(m2g::pb::ObjectType object_type) const {
	for (const auto &enemy: enemies.enemies()) {
		if (enemy.object_type() == object_type) {
			return &enemy;
		}
	}
	return nullptr;
}

void m2g::Proxy::save_progress() const {
	m2::pb::message_to_json_file(progress, progress_file_path);
}

const m2::UiPanelBlueprint* m2g::Proxy::generate_main_menu() {
	_main_menu = m2::UiPanelBlueprint{
		.name = "MainMenu",
	    .w = 160, .h = 90,
	    .border_width = 0,
	    .background_color = SDL_Color{20, 20, 20, 255}
	};

	auto level_jsons = m2::ListFiles(M2_GAME.GetResources().GetGameResourceDir() / "levels", ".json");
	for (int i = 0; i < (int) level_jsons.size(); ++i) {
		const auto &level_json = level_jsons[i];
		auto level_name = level_json.stem().string();
		LOG_INFO("Adding level button", level_name);

		bool level_completed = progress.level_completion_times().contains(level_name);
		auto level_display_name = level_completed ? level_name : '*' + level_name + '*';

		// 8 columns
		auto row = i / 8;
		auto col = i % 8;

		int x_padding = 26, y_padding = 17;
		int x_button_width = 10, y_button_width = 6;
		int button_gap = 4;
		_main_menu.widgets.emplace_back(m2::UiWidgetBlueprint{
		    .x = x_padding + col * (x_button_width + button_gap),
		    .y = y_padding + row * (y_button_width + button_gap),
		    .w = x_button_width, .h = y_button_width,
		    .variant = m2::widget::TextBlueprint{
		        .text = level_display_name,
		        .onAction = [=, this](MAYBE const m2::widget::Text &self) {
			        alive_enemy_count = 0;
			        m2SucceedOrThrowError(M2_GAME.LoadSinglePlayer(level_json, level_name));
			        M2_GAME.audio_manager->Play(&M2_GAME.songs[m2g::pb::SONG_MAIN_THEME],
			                                 m2::AudioManager::PlayPolicy::LOOP, 0.5f);
			        return m2::MakeReturnAction();
		        }
		    }
		});
	}

	LOG_DEBUG("Adding quit button");
	_main_menu.widgets.emplace_back(m2::UiWidgetBlueprint{
	    .x = 75, .y = 78, .w = 10, .h = 6,
	    .variant = m2::widget::TextBlueprint{
	        .text = "Quit",
	        .onAction = [](MAYBE const m2::widget::Text &self) {
		        return m2::MakeQuitAction();
	        }
	    }
	});

	return &_main_menu;
}

const m2::UiPanelBlueprint* m2g::Proxy::generate_right_hud() {
	_right_hud = m2::UiPanelBlueprint{
		.name = "RightHud",
	    .w = 19, .h = 72,
	    .background_color = {0, 0, 0, 255},
		.widgets = {}
	};

	_right_hud.widgets.emplace_back(m2::UiWidgetBlueprint{
	    .initially_enabled = false,
	    .x = 2, .y = 66, .w = 15, .h = 2,
	    .border_width = 0,
	    .variant = m2::widget::TextBlueprint{
	        .text = "AMMO"
	    }
	});
	_right_hud.widgets.emplace_back(m2::UiWidgetBlueprint{
	    .initially_enabled = false,
	    .x = 2, .y = 68, .w = 15, .h = 2,
	    .variant = m2::widget::ProgressBarBlueprint{
	        .bar_color = SDL_Color{0, 127, 255, 255},
	        .onUpdate = [](m2::widget::ProgressBar& self) {
		        if (const auto *player = M2_LEVEL.GetPlayer(); player) {
			        if (const auto ammo = player->GetCharacter().GetResource(pb::RESOURCE_SPECIAL_RANGED_WEAPON_AMMO); ammo != 0.0f) {
				        if (const auto weapon = player->GetCharacter().FindItems(pb::ITEM_CATEGORY_SPECIAL_RANGED_WEAPON); weapon) {
					        self.SetProgress(ammo / weapon->GetAcquireBenefit(pb::RESOURCE_SPECIAL_RANGED_WEAPON_AMMO));
				        }
			        }
		        }
	        	self.SetProgress(0.0f);
	        }
	    }
	});

	return &_right_hud;
}

void m2g::Proxy::set_ammo_display_state(bool enabled) {
	M2_LEVEL.GetRightHud()->widgets[0]->enabled = enabled;
	M2_LEVEL.GetRightHud()->widgets[1]->enabled = enabled;
}

const m2::UiPanelBlueprint* m2g::Proxy::you_died_menu() {
	_you_died_menu = m2::UiPanelBlueprint{
		.name = "YouDiedPanel",
	    .w = 160, .h = 90,
	    .border_width = 0,
	    .background_color = SDL_Color{127, 0, 0, 127}
	};

	auto lb_path = M2_LEVEL.GetLevelFilePath();
	if (lb_path) {
		_you_died_menu.widgets.emplace_back(m2::UiWidgetBlueprint{
		    .x = 70, .y = 70, .w = 20, .h = 6,
		    .variant = m2::widget::TextBlueprint{
		        .text = "Retry",
		        .onAction = [=, this](MAYBE const m2::widget::Text &self) -> m2::UiAction {
			        alive_enemy_count = 0;
			        m2SucceedOrThrowError(M2_GAME.LoadSinglePlayer(*lb_path, M2_LEVEL.GetName()));
			        return m2::MakeReturnAction();
		        }
		    }
		});
	}

	_you_died_menu.widgets.emplace_back(m2::UiWidgetBlueprint{
	    .x = 55, .y = 80, .w = 20, .h = 6,
	    .variant = m2::widget::TextBlueprint{
	        .text = "Main Menu",
	        .onAction = [&](MAYBE const m2::widget::Text &self) {
		        return m2::UiPanel::create_and_run_blocking(&_main_menu);
	        }
	    }
	});

	_you_died_menu.widgets.emplace_back(m2::UiWidgetBlueprint{
	    .x = 85, .y = 80, .w = 20, .h = 6,
	    .variant = m2::widget::TextBlueprint{
	        .text = "Quit",
	        .onAction = [](MAYBE const m2::widget::Text &self) {
		        return m2::MakeQuitAction();
	        }
	    }
	});

	return &_you_died_menu;
}
