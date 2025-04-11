#include <rpg/Objects.h>
#include <m2/Log.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <m2/Game.h>

m2::void_expected rpg::init_finish_point(m2::Object& obj) {
	auto sprite_type = M2_GAME.object_main_sprites[obj.GetType()];
	auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(sprite_type));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::STATIC,
		.fixtures = {m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::CircleShape::FromSpriteCircleFixture(sprite.OriginalPb().regular().fixtures(0).circle(), sprite.Ppm()),
			.isSensor = true,
			.colliderFilter = m2::third_party::physics::gColliderCategoryToParams[m2::I(m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_FOREGROUND_FRIENDLY_ITEM)]
		}},
		.allowSleeping = true,
		.initiallyAwake = false,
		.isBullet = false
	};
	phy.body = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation);

	auto& gfx = obj.AddGraphic(sprite_type);
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
