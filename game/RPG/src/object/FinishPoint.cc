#include <rpg/object/FinishPoint.h>
#include <rpg/Context.h>
#include <m2/box2d/Utils.h>

m2::VoidValue rpg::init_finish_point(m2::Object& obj, m2g::pb::ObjectType& type) {
	auto sprite_type = GAME.level_editor_object_sprites[type];
	auto& sprite = GAME.get_sprite(sprite_type);

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::STATIC);
	bp.set_allow_sleep(true);
	bp.set_is_bullet(false);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(sprite.background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_ITEM_ON_FOREGROUND);
	bp.mutable_background_fixture()->set_is_sensor(true);
	phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);

	auto& gfx = obj.add_graphic(sprite);
	gfx.draw_sprite_effect = m2::pb::SpriteEffectType::SPRITE_EFFECT_GRAYSCALE;

	phy.on_collision = [](MAYBE m2::Physique& self, MAYBE m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		// Check enemy counter
		auto& context = rpg::Context::get_instance();
		if (context.alive_enemy_count == 0) {
			// Finish game
			GAME.add_deferred_action([]() {
				auto level_duration = LEVEL.get_level_duration();
				LOG_INFO("Level duration", level_duration);
				rpg::Context::get_instance().progress.mutable_level_completion_times()->operator[](LEVEL.name()) = level_duration;
				LOG_INFO("Saving progress...");
				rpg::Context::get_instance().save_progress();
				LOG_INFO("Progress saved");

				if (m2::ui::execute_blocking(m2g::ui::main_menu()) == m2::ui::Action::QUIT) {
					GAME.quit = true;
				}
			});
		}
	};
	gfx.pre_draw = [](MAYBE m2::Graphic& gfx) {
		// Check enemy counter, adjust sprite effect
		auto& context = rpg::Context::get_instance();
		gfx.draw_sprite_effect = context.alive_enemy_count ? m2::pb::SpriteEffectType::SPRITE_EFFECT_GRAYSCALE : m2::pb::SpriteEffectType::NO_SPRITE_EFFECT;
	};

	return {};
}
