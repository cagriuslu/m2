#include <rpg/Objects.h>
#include <m2/Log.h>
#include <m2/box2d/Detail.h>

m2::void_expected rpg::init_finish_point(m2::Object& obj) {
	auto sprite_type = M2_GAME.object_main_sprites[obj.object_type()];
	auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(sprite_type));

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::STATIC);
	bp.set_allow_sleep(true);
	bp.set_is_bullet(false);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(sprite.BackgroundColliderCircRadiusM());
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_ITEM_ON_FOREGROUND);
	bp.mutable_background_fixture()->set_is_sensor(true);
	phy.body = m2::box2d::CreateBody(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);

	auto& gfx = obj.add_graphic(sprite_type);
	gfx.variantDrawOrder[0] = m2::pb::SpriteEffectType::SPRITE_EFFECT_GRAYSCALE;

	phy.onCollision = [](MAYBE m2::Physique& self, MAYBE m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		// Check enemy counter
		if (M2G_PROXY.alive_enemy_count == 0) {
			// Finish game
			M2_DEFER([]() {
				auto level_duration = M2_LEVEL.GetLevelDuration();
				LOG_INFO("Level duration", level_duration);
				M2G_PROXY.progress.mutable_level_completion_times()->operator[](M2_LEVEL.Name()) = level_duration;
				LOG_INFO("Saving progress...");
				M2G_PROXY.save_progress();
				LOG_INFO("Progress saved");
				M2_LEVEL.MarkForDeletion();
			});
		}
	};
	gfx.preDraw = [](MAYBE m2::Graphic& gfx) {
		// Check enemy counter, adjust sprite effect
		if (M2G_PROXY.alive_enemy_count) {
			gfx.variantDrawOrder[0] = m2::pb::SpriteEffectType::SPRITE_EFFECT_GRAYSCALE;
		} else {
			gfx.variantDrawOrder[0] = std::nullopt;
		}
	};

	return {};
}
