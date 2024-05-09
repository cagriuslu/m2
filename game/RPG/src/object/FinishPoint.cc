#include <rpg/Objects.h>
#include <m2/Log.h>
#include <m2/box2d/Detail.h>

m2::void_expected rpg::init_finish_point(m2::Object& obj) {
	auto sprite_type = M2_GAME.object_main_sprites[obj.object_type()];
	auto& sprite = M2_GAME.get_sprite(sprite_type);

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::STATIC);
	bp.set_allow_sleep(true);
	bp.set_is_bullet(false);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(sprite.background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_ITEM_ON_FOREGROUND);
	bp.mutable_background_fixture()->set_is_sensor(true);
	phy.body = m2::box2d::create_body(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);

	auto& gfx = obj.add_graphic(sprite);
	gfx.draw_variant = m2::pb::SpriteEffectType::SPRITE_EFFECT_GRAYSCALE;

	phy.on_collision = [](MAYBE m2::Physique& self, MAYBE m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		// Check enemy counter
		if (M2G_PROXY.alive_enemy_count == 0) {
			// Finish game
			M2_DEFER([]() {
				auto level_duration = M2_LEVEL.get_level_duration();
				LOG_INFO("Level duration", level_duration);
				M2G_PROXY.progress.mutable_level_completion_times()->operator[](M2_LEVEL.name()) = level_duration;
				LOG_INFO("Saving progress...");
				M2G_PROXY.save_progress();
				LOG_INFO("Progress saved");

				if (m2::ui::State::create_execute_sync(M2G_PROXY.main_menu()).is_quit()) {
					M2_GAME.quit = true;
				}
			});
		}
	};
	gfx.pre_draw = [](MAYBE m2::Graphic& gfx) {
		// Check enemy counter, adjust sprite effect
		if (M2G_PROXY.alive_enemy_count) {
			gfx.draw_variant = m2::pb::SpriteEffectType::SPRITE_EFFECT_GRAYSCALE;
		} else {
			gfx.draw_variant = m2::IsForegroundCompanion{false};
		}
	};

	return {};
}
