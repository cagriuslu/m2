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

		if (const auto camera_offset = LEVEL.camera_offset(); prev_camera_offset != camera_offset) {
			prev_camera_offset = camera_offset;

			sin = camera_offset.z / camera_offset.length();
		}
		return sin;
	}
	float camera_cos() {
		// Do not recalculate unless the camera offset changed
		static m3::VecF prev_camera_offset;
		static float cos{};

		if (const auto camera_offset = LEVEL.camera_offset(); prev_camera_offset != camera_offset) {
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
	const auto* camera = LEVEL.objects.get(LEVEL.camera_id);
	return position - camera->position;
}

m2::VecF m2::camera_to_position_dstpx(const VecF& position) {
	return camera_to_position_m(position) * GAME.dimensions().ppm;
}

m2::VecF m2::screen_origin_to_position_dstpx(const VecF& position) {
	return camera_to_position_dstpx(position) + VecF{GAME.dimensions().window.w / 2, GAME.dimensions().window.h / 2 };
}

m2::VecF m2::screen_origin_to_sprite_center_dstpx(const VecF& position, const Sprite& sprite, const pb::SpriteEffectType effect_type) {
	return screen_origin_to_position_dstpx(position) - sprite.center_to_origin_dstpx(effect_type);
}

m3::VecF m3::camera_position_m() {
	const auto* camera = LEVEL.objects.get(LEVEL.camera_id);
	const auto raw_camera_position = VecF{camera->position.x, camera->position.y, 0.0f};
	const auto camera_position = raw_camera_position + LEVEL.camera_offset();
	return camera_position;
}
m3::VecF m3::focus_position_m() {
	auto* player = LEVEL.player();
	return {player->position.x, player->position.y, m2g::focus_point_height};
}
float m3::visible_width_m() {
	// Do not recalculate unless the distance or FOV has changed
	static m3::VecF prev_camera_offset;
	static auto prev_horizontal_fov = INFINITY;

	static float visible_width{};
	if (const auto camera_offset = LEVEL.camera_offset(); prev_camera_offset != camera_offset || prev_horizontal_fov != LEVEL.horizontal_fov()) {
		prev_camera_offset = camera_offset;
		prev_horizontal_fov = LEVEL.horizontal_fov();

		const auto tan_of_half_horizontal_fov = tanf(m2::to_radians(LEVEL.horizontal_fov()) / 2.0f);
		visible_width = 2 * camera_offset.length() * tan_of_half_horizontal_fov;
	}
	return visible_width;
}
float m3::ppm() {
	return static_cast<float>(GAME.dimensions().game.w) / visible_width_m();
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
	if (LEVEL.projection_type() == m2::pb::PERSPECTIVE_YZ) {
		static const auto unit_vector_along_x = VecF{1.0f, 0.0f, 0.0f};
		horizontal_projection = focus_to_projection->dot(unit_vector_along_x);
		const auto projection_x = VecF{horizontal_projection, 0.0f, 0.0f};

		// y-axis of the projection plane is not along the real y-axis
		// Use dot product, or better, find the length and the sign of it
		const auto projection_y = *focus_to_projection - projection_x;
		const auto projection_y_length = projection_y.length();
		const auto projection_y_sign = 0 <= projection_y.y ? 1.0f : -1.0f;
		vertical_projection = projection_y_length * projection_y_sign;
	} else if (LEVEL.projection_type() == m2::pb::PERSPECTIVE_XYZ) {
		static const auto horizontal_unit_vector = VecF{m2::SQROOT_2, -m2::SQROOT_2, 0.0f};
		horizontal_projection = focus_to_projection->dot(horizontal_unit_vector);

		const auto vertical_unit_vector = VecF{camera_sin() / m2::SQROOT_2, camera_sin() / m2::SQROOT_2, -camera_cos()};
		vertical_projection = focus_to_projection->dot(vertical_unit_vector);
	} else {
		throw M2FATAL("Invalid ProjectionType");
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
	return *focus_to_projection_along_camera_plane_px + m2::VecF{GAME.dimensions().window.w / 2, GAME.dimensions().window.h / 2 };
}

m2::Graphic::Graphic(const Id object_id) : Component(object_id) {}
m2::Graphic::Graphic(const uint64_t object_id, const Sprite& sprite) : Component(object_id), on_draw(default_draw), on_effect(default_draw_addons), sprite(&sprite) {}

void m2::Graphic::default_draw(const Graphic& gfx) {
	if (not gfx.sprite) {
		// This function only draws sprites
		return;
	}

	if (is_projection_type_perspective(LEVEL.projection_type())) {
		// Check if foreground or background
		const bool is_foreground = LEVEL.graphics.get_id(&gfx);
		draw_fake_3d(gfx.parent().position, *gfx.sprite, gfx.draw_sprite_effect, gfx.draw_angle, is_foreground, gfx.z);
	} else {
		draw_real_2d(gfx.parent().position, *gfx.sprite, gfx.draw_sprite_effect, gfx.draw_angle);
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

	if (is_projection_type_parallel(LEVEL.projection_type())) {
		const auto src_rect = static_cast<SDL_Rect>(gfx.sprite->rect());
		const auto screen_origin_to_sprite_center_px_vec = screen_origin_to_sprite_center_dstpx(gfx.parent().position,
				*gfx.sprite, gfx.draw_sprite_effect);
		dst_rect = SDL_Rect{
				(int) roundf(screen_origin_to_sprite_center_px_vec.x) - (src_rect.w * GAME.dimensions().ppm / gfx.sprite->ppm() / 2),
				(int) roundf(screen_origin_to_sprite_center_px_vec.y) + (src_rect.h * GAME.dimensions().ppm * 11 / gfx.sprite->ppm() / 2 / 10), // Give an offset of 1.1
				GAME.dimensions().ppm,
				GAME.dimensions().ppm * 12 / 100 // 0.15 m height
		};
	} else {
		const auto obj_position = gfx.parent().position;
		// Place add-on below the sprite
		const auto addon_position = m3::VecF{obj_position.x, obj_position.y, -0.2f};
		if (const auto projected_addon_position = screen_origin_to_projection_along_camera_plane_dstpx(addon_position)) {
			const auto rect = RectI::centered_around(VecI{*projected_addon_position}, GAME.dimensions().ppm, GAME.dimensions().ppm * 12 / 100);
			dst_rect = static_cast<SDL_Rect>(rect);
		}
	}

	// Black background
	SDL_SetRenderDrawColor(GAME.renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(GAME.renderer, &dst_rect);

	// Green part
	const float percentage = (*gfx.draw_addon_health_bar) < 0.0f ? 0.0f : (1.0f < *gfx.draw_addon_health_bar) ? 1.0f : *gfx.draw_addon_health_bar;
	const auto green_rect = SDL_Rect{dst_rect.x + 1, dst_rect.y + 1, I(roundf(percentage * F(dst_rect.w - 2))), dst_rect.h - 2};
	SDL_SetRenderDrawColor(GAME.renderer, 0, 255, 0, 255);
	SDL_RenderFillRect(GAME.renderer, &green_rect);
}

void m2::Graphic::color_cell(const VecI& cell, SDL_Color color) {
	const auto screen_origin_to_cell_center_px = screen_origin_to_position_dstpx(VecF{cell});
	const auto rect = SDL_Rect{
		(int)roundf(screen_origin_to_cell_center_px.x) - (GAME.dimensions().ppm / 2),
		(int)roundf(screen_origin_to_cell_center_px.y) - (GAME.dimensions().ppm / 2),
		GAME.dimensions().ppm,
		GAME.dimensions().ppm
	};

	SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(GAME.renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderFillRect(GAME.renderer, &rect);
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

	SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(GAME.renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderFillRect(GAME.renderer, &rect);
}

void m2::Graphic::color_disk(const VecF& center_position_m, float radius_m, const SDL_Color& color) {
	const auto center_position_px = screen_origin_to_position_dstpx(center_position_m);
	const auto radius_px = radius_m * GAME.dimensions().ppm;
	sdl::draw_disk(GAME.renderer, center_position_px, color, radius_px, color);
}

void m2::Graphic::draw_cross(const VecF& world_position, SDL_Color color) {
	SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
	const auto draw_position = VecI{screen_origin_to_position_dstpx(world_position)};
	SDL_RenderDrawLine(GAME.renderer, draw_position.x - 9, draw_position.y - 9, draw_position.x + 10, draw_position.y + 10);
	SDL_RenderDrawLine(GAME.renderer, draw_position.x - 9, draw_position.y + 9, draw_position.x + 10, draw_position.y - 10);
}

void m2::Graphic::draw_line(const VecF& world_position_1, const VecF& world_position_2, SDL_Color color) {
	SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
	if (is_projection_type_parallel(LEVEL.projection_type())) {
		const auto p1 = static_cast<VecI>(screen_origin_to_position_dstpx(world_position_1));
		const auto p2 = static_cast<VecI>(screen_origin_to_position_dstpx(world_position_2));
		SDL_RenderDrawLine(GAME.renderer, p1.x, p1.y, p2.x, p2.y);
	} else {
		const auto p1 = m3::screen_origin_to_projection_along_camera_plane_dstpx(m3::VecF{world_position_1});
		const auto p2 = m3::screen_origin_to_projection_along_camera_plane_dstpx(m3::VecF{world_position_2});
		if (p1 && p2) {
			SDL_RenderDrawLineF(GAME.renderer, p1->x, p1->y, p2->x, p2->y);
		}
	}
}

void m2::Graphic::draw_vertical_line(float x, SDL_Color color) {
	const auto x_px = static_cast<int>(roundf(screen_origin_to_position_dstpx(VecF{x, 0.0f}).x));
	SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(GAME.renderer, x_px, GAME.dimensions().game.y, x_px, GAME.dimensions().game.y + GAME.dimensions().game.h);
}

void m2::Graphic::draw_horizontal_line(float y, SDL_Color color) {
	const auto y_px = static_cast<int>(roundf(screen_origin_to_position_dstpx(VecF{0.0f, y}).y));
	SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(GAME.renderer, GAME.dimensions().game.x, y_px, GAME.dimensions().game.x + GAME.dimensions().game.w, y_px);
}
