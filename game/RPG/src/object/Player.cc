#include <rpg/object/Player.h>
#include <m2/Object.h>
#include "m2/Log.h"
#include "m2/Game.h"
#include <rpg/Graphic.h>
#include <rpg/Detail.h>
#include "m2/Controls.h"
#include <m2/game/CharacterMovement.h>
#include <rpg/Objects.h>
#include <m2/box2d/Detail.h>
#include <m2/M2.h>
#include <Item.pb.h>
#include <rpg/Defs.h>
#include <array>


using namespace rpg;
using namespace m2g;
using namespace m2g::pb;

rpg::Player::Player(m2::Object& obj) : animation_fsm(m2g::pb::ANIMATION_TYPE_PLAYER_MOVEMENT, obj.graphic_id()) {}

m2::void_expected rpg::Player::init(m2::Object& obj) {
	auto id = obj.id();
	auto main_sprite_type = M2_GAME.object_main_sprites[m2g::pb::PLAYER];
	const auto& mainSprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(main_sprite_type));

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(mainSprite.BackgroundColliderCircRadiusM());
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_ON_BACKGROUND);
	bp.mutable_foreground_fixture()->mutable_circ()->set_radius(mainSprite.ForegroundColliderCircRadiusM());
	bp.mutable_foreground_fixture()->mutable_circ()->mutable_center_offset()->set_x(mainSprite.OriginToForegroundColliderOriginVecM().x);
	bp.mutable_foreground_fixture()->mutable_circ()->mutable_center_offset()->set_y(mainSprite.OriginToForegroundColliderOriginVecM().y);
	bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_ON_FOREGROUND);
	bp.set_mass(PLAYER_MASS);
	bp.set_linear_damping(PLAYER_LINEAR_DAMPING);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);

	auto& gfx = obj.add_graphic(main_sprite_type);

	auto& chr = obj.add_full_character();
	chr.add_named_item(M2_GAME.GetNamedItem(m2g::pb::ITEM_REUSABLE_DASH_2S));
	if (M2_LEVEL.Identifier() != "MeleeTutorialClosed") {
		// 4th level is melee tutorial
		chr.add_named_item(M2_GAME.GetNamedItem(m2g::pb::ITEM_REUSABLE_GUN));
	}
	chr.add_named_item(M2_GAME.GetNamedItem(m2g::pb::ITEM_REUSABLE_SWORD));
	chr.add_named_item(M2_GAME.GetNamedItem(m2g::pb::ITEM_AUTOMATIC_DASH_ENERGY));
	chr.add_named_item(M2_GAME.GetNamedItem(m2g::pb::ITEM_AUTOMATIC_RANGED_ENERGY));
	chr.add_named_item(M2_GAME.GetNamedItem(m2g::pb::ITEM_AUTOMATIC_MELEE_ENERGY));
	chr.add_resource(m2g::pb::RESOURCE_HP, 1.0f);
	chr.set_max_resource(m2g::pb::RESOURCE_HP, 1.0f);
	chr.add_resource(m2g::pb::RESOURCE_DASH_ENERGY, 2.0f);

	obj.impl = std::make_unique<rpg::Player>(obj);
	auto& impl = dynamic_cast<Player&>(*obj.impl);

	phy.pre_step = [&, id=id](m2::Physique& phy) {
		auto& chr = obj.character();
		auto vector_to_mouse = (M2_GAME.MousePositionWorldM() - obj.position).normalize();

		auto [direction_enum, direction_vector] = m2::calculate_character_movement(m2::Key::LEFT, m2::Key::RIGHT, m2::Key::UP, m2::Key::DOWN);
		float move_force;
		// Check if dash
		if (direction_vector && M2_GAME.events.pop_key_press(m2::Key::DASH) && chr.use_item(chr.find_items(m2g::pb::ITEM_REUSABLE_DASH_2S))) {
			move_force = PLAYER_DASH_FORCE;
		} else {
			// Character movement
			auto anim_state_type = detail::to_animation_state_type(direction_enum);
			impl.animation_fsm.signal(m2::AnimationFsmSignal{anim_state_type});
			move_force = PLAYER_WALK_FORCE;
		}
		if (direction_vector) {
			// Apply force
			phy.body->ApplyForceToCenter(static_cast<b2Vec2>(direction_vector * (move_force * M2_GAME.DeltaTimeS())), true);
		}

		// Primary weapon
		if (M2_GAME.events.is_mouse_button_down(m2::MouseButton::PRIMARY)) {
			auto shoot = [&](const m2::Item& weapon) {
				rpg::create_projectile(*m2::create_object(obj.position, {}, id),
					vector_to_mouse, weapon, true);
				// Knock-back
				phy.body->ApplyForceToCenter(static_cast<b2Vec2>(m2::VecF::from_angle(vector_to_mouse.angle_rads() + m2::PI) * 50000.0f), true);
			};

			// Check if there is a special ranged weapon and try to use the item
			auto special_it = chr.find_items(m2g::pb::ITEM_CATEGORY_SPECIAL_RANGED_WEAPON);
			if (special_it) {
				if (chr.use_item(special_it)) {
					shoot(*special_it);
				}
			} else {
				// Find default weapon and try to use it
				auto default_it = chr.find_items(m2g::pb::ITEM_CATEGORY_DEFAULT_RANGED_WEAPON);
				if (default_it && chr.use_item(default_it)) {
					shoot(*default_it);
				}
			}
		}

		// Secondary weapon
		if (M2_GAME.events.is_mouse_button_down(m2::MouseButton::SECONDARY)) {
			auto slash = [&](const m2::Item& weapon) {
				rpg::create_blade(*m2::create_object(obj.position, {}, id),
					vector_to_mouse, weapon, true);
			};

			// Check if there is a special melee weapon and try to use the item
			auto special_it = chr.find_items(m2g::pb::ITEM_CATEGORY_SPECIAL_MELEE_WEAPON);
			if (special_it) {
				// Try to use the weapon
				if (chr.use_item(special_it)) {
					slash(*special_it);
				}
			} else {
				// Find default melee weapon and try to use it
				auto default_it = chr.find_items(m2g::pb::ITEM_CATEGORY_DEFAULT_MELEE_WEAPON);
				if (default_it && chr.use_item(default_it)) {
					slash(*default_it);
				}
			}
		}
	};
	chr.update = [](MAYBE m2::Character& chr) {
		// Check if died
		if (not chr.has_resource(m2g::pb::RESOURCE_HP)) {
			LOG_INFO("You died");
			if (m2::UiPanel::create_and_run_blocking(M2G_PROXY.you_died_menu()).IsQuit()) {
				M2_GAME.quit = true;
			}
		}
		// Check if special ammo finished
		if (auto special_it = chr.find_items(m2g::pb::ITEM_CATEGORY_SPECIAL_RANGED_WEAPON);
			special_it && !chr.has_resource(m2g::pb::RESOURCE_SPECIAL_RANGED_WEAPON_AMMO)) {
			// Remove weapon if no ammo left
			chr.remove_item(special_it);
		}
		// Show/hide ammo display
		M2G_PROXY.set_ammo_display_state((bool) chr.find_items(m2g::pb::ITEM_CATEGORY_SPECIAL_RANGED_WEAPON));
	};
	phy.on_collision = [](MAYBE m2::Physique& me, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (auto* other_char = other.owner().get_character(); other_char && 10.0f < m2::VecF{me.body->GetLinearVelocity()}.length()) {
			InteractionData data;
			data.set_stun_duration(2.0f);
			other_char->execute_interaction(data);
		}
	};
	chr.on_interaction = [](m2::Character& self, MAYBE m2::Character* other, const InteractionData& data) -> std::optional<m2g::pb::InteractionData> {
		if (data.has_hit_damage()) {
			// Get hit by an enemy
			self.remove_resource(m2g::pb::RESOURCE_HP, data.hit_damage());
		} else if (data.has_item_type()) {
			const auto& item = M2_GAME.GetNamedItem(data.item_type());
			// Player can hold only one special weapon of certain type, get rid of the previous one
			constexpr std::array<ItemCategory, 2> special_categories = {ITEM_CATEGORY_SPECIAL_RANGED_WEAPON, ITEM_CATEGORY_SPECIAL_MELEE_WEAPON};
			constexpr std::array<ResourceType, 2> special_ammo_type = {RESOURCE_SPECIAL_RANGED_WEAPON_AMMO, NO_RESOURCE};
			for (size_t i = 0; i < special_categories.size(); ++i) {
				if (auto sp = special_categories[i]; sp == item.category()) {
					if (auto it = self.find_items(sp); it) {
						self.remove_item(it); // Remove weapon
						self.clear_resource(special_ammo_type[i]); // Also remove any ammo
					}
					break;
				}
			}
			// Add item
			self.add_named_item(item);
		}
		return std::nullopt;
	};
	gfx.pre_draw = [&](MAYBE m2::Graphic& gfx) {
		impl.animation_fsm.time(M2_GAME.DeltaTimeS());
	};
	gfx.on_draw = [&](m2::Graphic& gfx) {
		// Draw the sprite itself
		m2::Graphic::DefaultDrawCallback(gfx);
		// Draw the HP addon
		DrawAddons(gfx, chr.get_resource(RESOURCE_HP));
	};

	M2_LEVEL.playerId = M2_LEVEL.objects.get_id(&obj);
	return {};
}
