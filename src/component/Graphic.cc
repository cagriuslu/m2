#include <m2/m3/VecF.h>
#include <m2/Component.h>
#include <m2/Game.h>
#include "m2/component/Graphic.h"
#include <m2/Object.h>

namespace {
	float camera_y_offset_m() {
		// Do not recalculate unless the height or the distance has changed
		static auto prev_camera_height = INFINITY;
		static auto prev_camera_distance = INFINITY;
		static float offset{};
		if (prev_camera_height != m2g::camera_height || prev_camera_distance != m2g::camera_distance) {
			prev_camera_height = m2g::camera_height;
			prev_camera_distance = m2g::camera_distance;
			offset = sqrtf(m2g::camera_distance * m2g::camera_distance - m2g::camera_height * m2g::camera_height);
		}
		return offset;
	}

	float camera_z_offset_m() {
		return m2g::camera_height;
	}
}

m2::VecF m2::camera_to_position_m(const VecF& position) {
	auto* camera = LEVEL.objects.get(LEVEL.camera_id);
	return position - camera->position;
}

m2::VecF m2::camera_to_position_px(const VecF& position) {
	return camera_to_position_m(position) * GAME.dimensions().ppm;
}

m2::VecF m2::screen_origin_to_position_px(const VecF& position) {
	return camera_to_position_px(position) + VecF{GAME.dimensions().window.w / 2, GAME.dimensions().window.h / 2 };
}

m3::VecF m3::camera_position_m() {
	auto* camera = LEVEL.objects.get(LEVEL.camera_id);
	auto raw_camera_position = VecF{camera->position.x, camera->position.y, 0.0f};
	auto camera_position = raw_camera_position + VecF{0.0f, camera_y_offset_m(), camera_z_offset_m()};
	return camera_position;
}
m3::VecF m3::player_position_m() {
	auto* player = LEVEL.player();
	return {player->position.x, player->position.y, 0.0f};
}
float m3::visible_width_m() {
	// Do not recalculate unless the distance or FOV has changed
	static auto prev_camera_distance = INFINITY;
	static auto prev_horizontal_field_of_view = INFINITY;
	static float visible_width{};
	if (prev_camera_distance != m2g::camera_distance || prev_horizontal_field_of_view != m2g::horizontal_field_of_view) {
		prev_camera_distance = m2g::camera_distance;
		prev_horizontal_field_of_view = m2g::horizontal_field_of_view;

		auto tan_of_half_horizontal_fov = tanf(m2::to_radians(m2g::horizontal_field_of_view) / 2.0f);
		visible_width = 2 * m2g::camera_distance * tan_of_half_horizontal_fov;
	}
	return visible_width;
}
float m3::ppm() {
	return (float)GAME.dimensions().game.w / visible_width_m();
}

m3::Line m3::camera_to_position(const VecF& position) {
	return Line::from_points(camera_position_m(), position);
}
m3::Plane m3::player_to_camera() {
	auto camera_position_in_meters = camera_position_m();
	auto player_position_in_meters = player_position_m();
	auto normal = camera_position_in_meters - player_position_in_meters;
	return Plane{normal, player_position_in_meters};
}

std::optional<m3::VecF> m3::player_to_projection_of_position_m(const VecF& position) {
	auto projection_plane = player_to_camera();
	auto camera_to_position_line = camera_to_position(position);
	auto [intersection_of_perspective_projection, forward_intersection] = projection_plane.intersection(camera_to_position_line);
	if (not forward_intersection) {
		return {};
	}
	auto player_to_projection_of_position_in_meters = intersection_of_perspective_projection - player_position_m();
	return player_to_projection_of_position_in_meters;
}
std::optional<m2::VecF> m3::projection_of_position_m(const VecF& position) {
	auto player_to_projection_vector = player_to_projection_of_position_m(position);
	if (not player_to_projection_vector) {
		return {};
	}
	static const auto unit_vector_along_x = VecF{1.0f, 0.0f, 0.0f};
	auto projection_on_x = player_to_projection_vector->dot(unit_vector_along_x);
	auto projection_x = VecF{projection_on_x, 0.0f, 0.0f};

	// y-axis of the projection plane is not along the real y-axis
	// Use dot product, or find the length and the sign of it
	auto projection_y = *player_to_projection_vector - projection_x;
	auto projection_y_length = projection_y.length();
	auto projection_y_sign = 0 <= projection_y.y ? 1.0f : -1.0f;
	auto projection_on_y = projection_y_length * projection_y_sign;

	return m2::VecF{projection_on_x, projection_on_y};
}
std::optional<m2::VecF> m3::projection_of_position_px(const VecF& position) {
	auto projection_of_position_in_meters = projection_of_position_m(position);
	if (not projection_of_position_in_meters) {
		return {};
	}
	auto pixels_per_meter = ppm();
	return *projection_of_position_in_meters * pixels_per_meter;
}
std::optional<m2::VecF> m3::screen_origin_to_projection_of_position_px(const VecF& position) {
	auto projection_of_position_in_pixels = projection_of_position_px(position);
	if (not projection_of_position_in_pixels) {
		return {};
	}
	return *projection_of_position_in_pixels + m2::VecF{GAME.dimensions().window.w / 2, GAME.dimensions().window.h / 2 };
}

m2::Graphic::Graphic(Id object_id) : Component(object_id) {}
m2::Graphic::Graphic(uint64_t object_id, const Sprite& sprite) : Component(object_id), on_draw(default_draw), on_effect(default_draw_addons), sprite(&sprite) {}

void m2::Graphic::default_draw(Graphic& gfx) {
	if (not gfx.sprite) {
		// This function only draws sprites
		return;
	}

	if (m2g::camera_height != 0.0f) {
		// Check if foreground or background
		bool is_foreground = LEVEL.graphics.get_id(&gfx);
		draw_fake_3d(gfx.parent().position, *gfx.sprite, gfx.draw_sprite_effect, gfx.draw_angle, is_foreground, gfx.z);
	} else {
		draw_real_2d(gfx.parent().position, *gfx.sprite, gfx.draw_sprite_effect, gfx.draw_angle);
	}
}

void m2::Graphic::default_draw_addons(Graphic& gfx) {
	if (not gfx.sprite) {
		// This function only works if there is a sprite
		return;
	}
	if (not gfx.draw_addon_health_bar) {
		return;
	}

	SDL_Rect dst_rect{};

	if (m2g::camera_height == 0.0f) {
		auto src_rect = static_cast<SDL_Rect>(gfx.sprite->rect());
		auto screen_origin_to_sprite_center_px_vec = screen_origin_to_sprite_center_px(gfx.parent().position, *gfx.sprite, gfx.draw_sprite_effect);
		dst_rect = SDL_Rect{
				(int) roundf(screen_origin_to_sprite_center_px_vec.x) - (src_rect.w * GAME.dimensions().ppm / gfx.sprite->ppm() / 2),
				(int) roundf(screen_origin_to_sprite_center_px_vec.y) + (src_rect.h * GAME.dimensions().ppm * 11 / gfx.sprite->ppm() / 2 / 10), // Give an offset of 1.1
				GAME.dimensions().ppm,
				GAME.dimensions().ppm * 12 / 100 // 0.15 m height
		};
	} else {
		auto obj_position = gfx.parent().position;
		// Place add-on below the sprite
		auto addon_position = m3::VecF{obj_position.x, obj_position.y, -0.2f};
		auto projected_addon_position = m3::screen_origin_to_projection_of_position_px(addon_position);
		if (projected_addon_position) {
			auto rect = RectI::centered_around(VecI{*projected_addon_position}, GAME.dimensions().ppm, GAME.dimensions().ppm * 12 / 100);
			dst_rect = (SDL_Rect) rect;
		}
	}

	// Black background
	SDL_SetRenderDrawColor(GAME.renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(GAME.renderer, &dst_rect);

	// Green part
	float percentage = (*gfx.draw_addon_health_bar) < 0.0f ? 0.0f : (1.0f < *gfx.draw_addon_health_bar) ? 1.0f : *gfx.draw_addon_health_bar;
	auto green_rect = SDL_Rect{dst_rect.x + 1, dst_rect.y + 1, I(roundf(percentage * F(dst_rect.w - 2))), dst_rect.h - 2};
	SDL_SetRenderDrawColor(GAME.renderer, 0, 255, 0, 255);
	SDL_RenderFillRect(GAME.renderer, &green_rect);
}

void m2::Graphic::color_cell(const VecI& cell, SDL_Color color) {
	auto screen_origin_to_cell_center_px = screen_origin_to_position_px(VecF{cell});
	auto rect = SDL_Rect{
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
	auto screen_origin_to_top_left_px = screen_origin_to_position_px(world_coordinates_m.top_left());
	auto screen_origin_to_bottom_right_px = screen_origin_to_position_px(world_coordinates_m.bottom_right());
	auto rect = SDL_Rect{
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
	auto center_position_px = screen_origin_to_position_px(center_position_m);
	auto radius_px = radius_m * GAME.dimensions().ppm;
	sdl::draw_disk(GAME.renderer, center_position_px, color, radius_px, color);
}

void m2::Graphic::draw_cross(const VecF& world_position, SDL_Color color) {
	SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
	auto draw_position = VecI{screen_origin_to_position_px(world_position)};
	SDL_RenderDrawLine(GAME.renderer, draw_position.x - 9, draw_position.y - 9, draw_position.x + 10, draw_position.y + 10);
	SDL_RenderDrawLine(GAME.renderer, draw_position.x - 9, draw_position.y + 9, draw_position.x + 10, draw_position.y - 10);
}

void m2::Graphic::draw_vertical_line(float x, SDL_Color color) {
	auto x_px = static_cast<int>(roundf(screen_origin_to_position_px(VecF{x, 0.0f}).x));
	SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(GAME.renderer, x_px, GAME.dimensions().game.y, x_px, GAME.dimensions().game.y + GAME.dimensions().game.h);
}

void m2::Graphic::draw_horizontal_line(float y, SDL_Color color) {
	auto y_px = static_cast<int>(roundf(screen_origin_to_position_px(VecF{0.0f, y}).y));
	SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(GAME.renderer, GAME.dimensions().game.x, y_px, GAME.dimensions().game.x + GAME.dimensions().game.w, y_px);
}
