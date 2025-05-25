#pragma once
#include <m2/Proxy.h>
#include <Enemy.pb.h>
#include <Progress.pb.h>

namespace m2g {
	class Proxy : public m2::Proxy {
	   public:
		const std::string gameIdentifier = "RPG";
		const bool camera_is_listener = true;
		const std::vector<m2::ObjectBlueprint> objectBlueprints = {
			{.objectType = pb::PLAYER, .defaultSpriteType = pb::PLAYER_LOOKDOWN_00, .defaultAnimationType = pb::ANIMATION_TYPE_PLAYER_MOVEMENT},
			{.objectType = pb::SKELETON, .defaultSpriteType = pb::SKELETON_LOOKDOWN_00, .defaultAnimationType = pb::ANIMATION_TYPE_SKELETON_MOVEMENT},
			{.objectType = pb::CASTLE_FINISH_POINT, .defaultSpriteType = pb::CASTLE_FINISH_POINT_SPRITE},
			{.objectType = pb::CUTEOPUS, .defaultSpriteType = pb::CUTEOPUS_LOOKDOWN_00, .defaultAnimationType = pb::ANIMATION_TYPE_CUTEOPUS_MOVEMENT},
			{.objectType = pb::MACHINE_GUN_ITEM_DROP, .defaultSpriteType = pb::MACHINE_GUN_ICON},
			{.objectType = pb::EXPLOSIVE_ITEM_DROP, .defaultSpriteType = pb::EXPLOSIVE_ICON},
			{.objectType = pb::LONG_SWORD_ITEM_DROP, .defaultSpriteType = pb::LONGSWORD_ICON},
			{.objectType = pb::AXE_ITEM_DROP, .defaultSpriteType = pb::AXE_ICON},
			{.objectType = pb::BUSH_01, .defaultSpriteType = pb::SPRITE_BUSH_01},
			{.objectType = pb::FLOWER_ORANGE_02, .defaultSpriteType = pb::SPRITE_FLOWER_ORANGE_02},
			{.objectType = pb::FLOWER_PINK_03, .defaultSpriteType = pb::SPRITE_FLOWER_PINK_03},
			{.objectType = pb::FLOWER_WHITE_05, .defaultSpriteType = pb::SPRITE_FLOWER_WHITE_05},
			{.objectType = pb::PINE_01, .defaultSpriteType = pb::SPRITE_PINE_01},
			{.objectType = pb::PINE_DEAD_02, .defaultSpriteType = pb::SPRITE_PINE_DEAD_02},
			{.objectType = pb::ROCK_03, .defaultSpriteType = pb::SPRITE_ROCK_03},
			{.objectType = pb::TREE_03, .defaultSpriteType = pb::SPRITE_TREE_03},
			{.objectType = pb::TREE_DEAD_03, .defaultSpriteType = pb::SPRITE_TREE_DEAD_03},
			{.objectType = pb::FENCE_HORIZONTAL, .defaultSpriteType = pb::SPRITE_FENCE},
			{.objectType = pb::FENCE_VERTICAL, .defaultSpriteType = pb::SPRITE_FENCE},
			{.objectType = pb::SPIKES, .defaultSpriteType = pb::SPIKES_OUT},
		};

		void load_resources();

		const m2::UiPanelBlueprint* MainMenuBlueprint();
		const m2::UiPanelBlueprint* PauseMenuBlueprint();
		const m2::UiPanelBlueprint* LeftHudBlueprint();
		const m2::UiPanelBlueprint* RightHudBlueprint();

		void post_single_player_level_init(const std::string& name, const m2::pb::Level& level);
		m2::third_party::physics::FixtureDefinition TileFixtureDefinition(MAYBE m2g::pb::SpriteType spriteType);
		m2::void_expected LoadForegroundObjectFromLevelBlueprint(m2::Object& obj);
		m2::Group* create_group(m2g::pb::GroupType group_type);

		// Game specific
	   public:
		rpg::pb::Enemies enemies;
		unsigned alive_enemy_count{};
		const rpg::pb::Enemy* get_enemy(m2g::pb::ObjectType object_type) const;

		std::filesystem::path progress_file_path;
		rpg::pb::Progress progress;
		void save_progress() const;

		m2::UiPanelBlueprint _main_menu;
		const m2::UiPanelBlueprint* generate_main_menu();

		m2::UiPanelBlueprint _right_hud;
		const m2::UiPanelBlueprint* generate_right_hud();
		void set_ammo_display_state(bool enabled);

		m2::UiPanelBlueprint _you_died_menu;
		const m2::UiPanelBlueprint* you_died_menu();
	};
}
