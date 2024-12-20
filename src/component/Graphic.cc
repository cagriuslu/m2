#include <m2/m3/VecF.h>
#include <m2/Component.h>
#include <m2/Game.h>
#include "m2/component/Graphic.h"
#include <m2/Object.h>

namespace {
	float camera_sin() {
		// Do not recalculate unless the camera offset changed
		static m3::VecF prev_camera_offset;
		static float sin{};

		if (const auto camera_offset = M2_LEVEL.camera_offset(); prev_camera_offset != camera_offset) {
			prev_camera_offset = camera_offset;

			sin = camera_offset.z / camera_offset.length();
		}
		return sin;
	}
	float camera_cos() {
		// Do not recalculate unless the camera offset changed
		static m3::VecF prev_camera_offset;
		static float cos{};

		if (const auto camera_offset = M2_LEVEL.camera_offset(); prev_camera_offset != camera_offset) {
			prev_camera_offset = camera_offset;

			cos = sqrtf(1.0f - camera_sin() * camera_sin());
		}
		return cos;
	}
}

bool m2::is_projection_type_parallel(const pb::ProjectionType pt) {
	return pt == pb::PARALLEL || pt == pb::PARALLEL_ISOMETRIC;
}
bool m2::is_projection_type_perspective(const pb::ProjectionType pt) {
	return pt == pb::PERSPECTIVE_YZ || pt == pb::PERSPECTIVE_XYZ;
}

m2::VecF m2::camera_to_position_m(const VecF& position) {
	const auto* camera = M2_LEVEL.objects.get(M2_LEVEL.camera_id);
	return position - camera->position;
}

m2::VecF m2::camera_to_position_dstpx(const VecF& position) {
	return camera_to_position_m(position) * M2_GAME.Dimensions().RealOutputPixelsPerMeter();
}

m2::VecF m2::screen_origin_to_position_dstpx(const VecF& position) {
	return camera_to_position_dstpx(position) + VecF{M2_GAME.Dimensions().WindowDimensions().x / 2, M2_GAME.Dimensions().WindowDimensions().y / 2 };
}

m2::VecF m2::screen_origin_to_sprite_center_dstpx(const VecF& position, const Sprite& sprite, SpriteVariant sprite_variant) {
	return screen_origin_to_position_dstpx(position) - sprite.CenterToOriginDstpx(sprite_variant);
}

m3::VecF m3::camera_position_m() {
	const auto* camera = M2_LEVEL.objects.get(M2_LEVEL.camera_id);
	const auto raw_camera_position = VecF{camera->position.x, camera->position.y, 0.0f};
	const auto camera_position = raw_camera_position + M2_LEVEL.camera_offset();
	return camera_position;
}
m3::VecF m3::focus_position_m() {
	return {M2_PLAYER.position.x, M2_PLAYER.position.y, M2G_PROXY.focus_point_height};
}
float m3::visible_width_m() {
	// Do not recalculate unless the distance or FOV has changed
	static m3::VecF prev_camera_offset;
	static auto prev_horizontal_fov = INFINITY;

	static float visible_width{};
	if (const auto camera_offset = M2_LEVEL.camera_offset(); prev_camera_offset != camera_offset || prev_horizontal_fov != M2_LEVEL.horizontal_fov()) {
		prev_camera_offset = camera_offset;
		prev_horizontal_fov = M2_LEVEL.horizontal_fov();

		const auto tan_of_half_horizontal_fov = tanf(m2::to_radians(M2_LEVEL.horizontal_fov()) / 2.0f);
		visible_width = 2 * camera_offset.length() * tan_of_half_horizontal_fov;
	}
	return visible_width;
}
float m3::ppm() {
	return static_cast<float>(M2_GAME.Dimensions().Game().w) / visible_width_m();
}

m3::Line m3::camera_to_position_line(const VecF& position) {
	return Line::from_points(camera_position_m(), position);
}
m3::Plane m3::focus_to_camera_plane() {
	const auto focus_position = focus_position_m();
	const auto normal = camera_position_m() - focus_position;
	return Plane{normal, focus_position};
}
std::optional<m3::VecF> m3::projection(const VecF& position) {
	auto [intersection_point, forward_intersection] = focus_to_camera_plane()
			.intersection(camera_to_position_line(position));
	if (not forward_intersection) {
		return {};
	}
	return intersection_point;
}
std::optional<m3::VecF> m3::focus_to_projection_m(const VecF& position) {
	const auto proj = projection(position);
	if (not proj) {
		return std::nullopt;
	}
	return *proj - focus_position_m();
}
std::optional<m2::VecF> m3::focus_to_projection_in_camera_plane_coordinates_m(const VecF& position) {
	const auto focus_to_projection = focus_to_projection_m(position);
	if (not focus_to_projection) {
		return {};
	}

	float horizontal_projection, vertical_projection;
	if (M2_LEVEL.projection_type() == m2::pb::PERSPECTIVE_YZ) {
		static const auto unit_vector_along_x = VecF{1.0f, 0.0f, 0.0f};
		horizontal_projection = focus_to_projection->dot(unit_vector_along_x);
		const auto projection_x = VecF{horizontal_projection, 0.0f, 0.0f};

		// y-axis of the projection plane is not along the real y-axis
		// Use dot product, or better, find the length and the sign of it
		const auto projection_y = *focus_to_projection - projection_x;
		const auto projection_y_length = projection_y.length();
		const auto projection_y_sign = 0 <= projection_y.y ? 1.0f : -1.0f;
		vertical_projection = projection_y_length * projection_y_sign;
	} else if (M2_LEVEL.projection_type() == m2::pb::PERSPECTIVE_XYZ) {
		static const auto horizontal_unit_vector = VecF{m2::SQROOT_2, -m2::SQROOT_2, 0.0f};
		horizontal_projection = focus_to_projection->dot(horizontal_unit_vector);

		const auto vertical_unit_vector = VecF{camera_sin() / m2::SQROOT_2, camera_sin() / m2::SQROOT_2, -camera_cos()};
		vertical_projection = focus_to_projection->dot(vertical_unit_vector);
	} else {
		throw M2_ERROR("Invalid ProjectionType");
	}
	return m2::VecF{horizontal_projection, vertical_projection};
}
std::optional<m2::VecF> m3::focus_to_projection_in_camera_plane_coordinates_dstpx(const VecF& position) {
	const auto focus_to_projection_along_camera_plane = focus_to_projection_in_camera_plane_coordinates_m(position);
	if (not focus_to_projection_along_camera_plane) {
		return {};
	}
	const auto pixels_per_meter = ppm();
	return *focus_to_projection_along_camera_plane * pixels_per_meter;
}
std::optional<m2::VecF> m3::screen_origin_to_projection_along_camera_plane_dstpx(const VecF& position) {
	const auto focus_to_projection_along_camera_plane_px = focus_to_projection_in_camera_plane_coordinates_dstpx(position);
	if (not focus_to_projection_along_camera_plane_px) {
		return {};
	}
	return *focus_to_projection_along_camera_plane_px + m2::VecF{M2_GAME.Dimensions().WindowDimensions().x / 2, M2_GAME.Dimensions().WindowDimensions().y / 2 };
}

m2::Graphic::Graphic(const Id object_id) : Component(object_id) {}
m2::Graphic::Graphic(const uint64_t object_id, const Sprite& sprite) : Component(object_id), on_draw(default_draw), on_addon(default_draw_addons), sprite(&sprite) {}

namespace detail {
	void default_draw(const m2::Graphic* gfx, m2::SpriteVariant sprite_variant) {
		if (m2::is_projection_type_perspective(M2_LEVEL.projection_type())) {
			// Check if foreground or background
			const bool is_foreground = M2_LEVEL.graphics.get_id(gfx);
			draw_fake_3d(gfx->owner().position, *gfx->sprite, sprite_variant, gfx->draw_angle, is_foreground, gfx->z);
		} else {
			draw_real_2d(gfx->owner().position, *gfx->sprite, sprite_variant, gfx->draw_angle);
		}
	}
}

void m2::Graphic::default_draw(const Graphic& gfx) {
	if (not gfx.sprite) {
		// This function only draws sprites
		return;
	}

	// Dim the sprite if dimming mode is enabled. TODO Dimming is implemented only for default variant.
	bool dimmed = dim_rendering_if_necessary(gfx.owner_id(), gfx.sprite->Texture(SpriteVariant{}));

	bool is_anything_drawn = false;
	for (size_t i = 0; i < gfx.variant_draw_order.size(); ++i) {
		if (const auto& sprite_variant = gfx.variant_draw_order[i]) {
			::detail::default_draw(&gfx, *sprite_variant);
			is_anything_drawn = true;
		}
	}
	// If nothing is drawn, fallback to default variant draw order of the sprite
	if (not is_anything_drawn) {
		// If the default variant draw order list is empty, draw the default variant. This is the most common drawing branch.
		if (gfx.sprite->DefaultVariantDrawOrder().empty()) {
			::detail::default_draw(&gfx, SpriteVariant{});
		} else {
			for (const auto& sprite_variant : gfx.sprite->DefaultVariantDrawOrder()) {
				if (sprite_variant == pb::SpriteEffectType::__NO_SPRITE_EFFECT) {
					::detail::default_draw(&gfx, SpriteVariant{});
				} else {
					::detail::default_draw(&gfx, SpriteVariant{sprite_variant});
				}
			}
		}
	}

	// If dimming was active, we need to un-dim.
	if (dimmed) {
		undim_rendering(gfx.sprite->Texture(SpriteVariant{}));
	}
}

void m2::Graphic::default_draw_addons(const Graphic& gfx) {
	if (not gfx.sprite) {
		// This function only works if there is a sprite
		return;
	}
	if (not gfx.draw_addon_health_bar) {
		return;
	}

	SDL_Rect dst_rect{};

	if (is_projection_type_parallel(M2_LEVEL.projection_type())) {
		const auto src_rect = static_cast<SDL_Rect>(gfx.sprite->Rect());
		const auto screen_origin_to_sprite_center_px_vec = screen_origin_to_sprite_center_dstpx(gfx.owner().position,
				*gfx.sprite, SpriteVariant{});
		dst_rect = SDL_Rect{
				iround(screen_origin_to_sprite_center_px_vec.x - (F(src_rect.w) * M2_GAME.Dimensions().RealOutputPixelsPerMeter() / F(gfx.sprite->Ppm()) / 2.0f)),
				iround(screen_origin_to_sprite_center_px_vec.y + (F(src_rect.h) * M2_GAME.Dimensions().RealOutputPixelsPerMeter() * 11.0f / F(gfx.sprite->Ppm()) / 2.0f / 10.0f)), // Give an offset of 1.1
				iround(M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				iround(M2_GAME.Dimensions().RealOutputPixelsPerMeter() * 12.0f / 100.0f) // 0.15 m height
		};
	} else {
		const auto obj_position = gfx.owner().position;
		// Place add-on below the sprite
		const auto addon_position = m3::VecF{obj_position.x, obj_position.y, -0.2f};
		if (const auto projected_addon_position = screen_origin_to_projection_along_camera_plane_dstpx(addon_position)) {
			const auto rect = RectI::centered_around(VecI{*projected_addon_position}, iround(M2_GAME.Dimensions().RealOutputPixelsPerMeter()), iround(M2_GAME.Dimensions().RealOutputPixelsPerMeter() * 12.0f / 100.0f));
			dst_rect = static_cast<SDL_Rect>(rect);
		}
	}

	// Black background
	SDL_SetRenderDrawColor(M2_GAME.renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(M2_GAME.renderer, &dst_rect);

	// Green part
	const float percentage = (*gfx.draw_addon_health_bar) < 0.0f ? 0.0f : (1.0f < *gfx.draw_addon_health_bar) ? 1.0f : *gfx.draw_addon_health_bar;
	const auto green_rect = SDL_Rect{dst_rect.x + 1, dst_rect.y + 1, I(roundf(percentage * F(dst_rect.w - 2))), dst_rect.h - 2};
	SDL_SetRenderDrawColor(M2_GAME.renderer, 0, 255, 0, 255);
	SDL_RenderFillRect(M2_GAME.renderer, &green_rect);
}

void m2::Graphic::color_cell(const VecI& cell, SDL_Color color) {
	const auto screen_origin_to_cell_center_px = screen_origin_to_position_dstpx(VecF{cell});
	const auto rect = SDL_Rect{
		iround(screen_origin_to_cell_center_px.x - (M2_GAME.Dimensions().RealOutputPixelsPerMeter() / 2.0f)),
		iround(screen_origin_to_cell_center_px.y - (M2_GAME.Dimensions().RealOutputPixelsPerMeter() / 2.0f)),
		iround(M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
		iround(M2_GAME.Dimensions().RealOutputPixelsPerMeter())
	};

	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderFillRect(M2_GAME.renderer, &rect);
}

void m2::Graphic::color_rect(const RectF& world_coordinates_m, SDL_Color color) {
	const auto screen_origin_to_top_left_px = screen_origin_to_position_dstpx(world_coordinates_m.top_left());
	const auto screen_origin_to_bottom_right_px = screen_origin_to_position_dstpx(world_coordinates_m.bottom_right());
	const auto rect = SDL_Rect{
			(int)roundf(screen_origin_to_top_left_px.x),
			(int)roundf(screen_origin_to_top_left_px.y),
			(int)roundf(screen_origin_to_bottom_right_px.x - screen_origin_to_top_left_px.x),
			(int)roundf(screen_origin_to_bottom_right_px.y - screen_origin_to_top_left_px.y)
	};

	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderFillRect(M2_GAME.renderer, &rect);
}
void m2::Graphic::color_rect(const RectF& world_coordinates_m, const RGB& color) {
	color_rect(world_coordinates_m, SDL_Color{color.r, color.g, color.b, 255});
}

void m2::Graphic::color_disk(const VecF& center_position_m, float radius_m, const SDL_Color& color) {
	const auto center_position_px = screen_origin_to_position_dstpx(center_position_m);
	const auto radius_px = radius_m * M2_GAME.Dimensions().RealOutputPixelsPerMeter();
	sdl::draw_disk(M2_GAME.renderer, center_position_px, color, radius_px, color);
}

void m2::Graphic::draw_cross(const VecF& world_position, SDL_Color color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	const auto draw_position = VecI{screen_origin_to_position_dstpx(world_position)};
	SDL_RenderDrawLine(M2_GAME.renderer, draw_position.x - 9, draw_position.y - 9, draw_position.x + 10, draw_position.y + 10);
	SDL_RenderDrawLine(M2_GAME.renderer, draw_position.x - 9, draw_position.y + 9, draw_position.x + 10, draw_position.y - 10);
}

void m2::Graphic::draw_line(const VecF& world_position_1, const VecF& world_position_2, SDL_Color color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	if (is_projection_type_parallel(M2_LEVEL.projection_type())) {
		const auto p1 = static_cast<VecI>(screen_origin_to_position_dstpx(world_position_1));
		const auto p2 = static_cast<VecI>(screen_origin_to_position_dstpx(world_position_2));
		SDL_RenderDrawLine(M2_GAME.renderer, p1.x, p1.y, p2.x, p2.y);
	} else {
		const auto p1 = m3::screen_origin_to_projection_along_camera_plane_dstpx(m3::VecF{world_position_1});
		const auto p2 = m3::screen_origin_to_projection_along_camera_plane_dstpx(m3::VecF{world_position_2});
		if (p1 && p2) {
			SDL_RenderDrawLineF(M2_GAME.renderer, p1->x, p1->y, p2->x, p2->y);
		}
	}
}

void m2::Graphic::draw_vertical_line(float x, SDL_Color color) {
	const auto x_px = static_cast<int>(roundf(screen_origin_to_position_dstpx(VecF{x, 0.0f}).x));
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(M2_GAME.renderer, x_px, M2_GAME.Dimensions().Game().y, x_px, M2_GAME.Dimensions().Game().y + M2_GAME.Dimensions().Game().h);
}

void m2::Graphic::draw_horizontal_line(float y, SDL_Color color) {
	const auto y_px = static_cast<int>(roundf(screen_origin_to_position_dstpx(VecF{0.0f, y}).y));
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(M2_GAME.renderer, M2_GAME.Dimensions().Game().x, y_px, M2_GAME.Dimensions().Game().x + M2_GAME.Dimensions().Game().w, y_px);
}

bool m2::Graphic::dim_rendering_if_necessary(Id object_id, SDL_Texture* texture) {
	// Dim the sprite if dimming mode is enabled
	if (const auto& dimming_exceptions = M2_LEVEL.dimming_exceptions()) {
		if (not dimming_exceptions->contains(object_id)) {
			static uint8_t mod = uround(M2G_PROXY.dimming_factor * F(255));
			SDL_SetTextureColorMod(texture, mod, mod, mod);
			return true;
		}
	}
	return false;
}

void m2::Graphic::undim_rendering(SDL_Texture* texture) {
	SDL_SetTextureColorMod(texture, 255, 255, 255);
}
