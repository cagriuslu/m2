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

m2::VecF m2::Graphic::sprite_center_to_sprite_origin_px() const {
	if (sprite) {
		VecF vector_in_source_pixels;
		if (draw_sprite_effect == pb::SPRITE_EFFECT_FOREGROUND_COMPANION && sprite->has_foreground_companion()) {
			vector_in_source_pixels = sprite->foreground_companion_center_offset_px();
		} else {
			vector_in_source_pixels = VecF{sprite->sprite().center_offset_px()};
			if (sprite->original_rotation_radians() != 0.0f) {
				vector_in_source_pixels = vector_in_source_pixels.rotate(sprite->original_rotation_radians());
			}
		}

		// Convert from source pixels to destination pixels
		auto vector_in_destination_pixels = vector_in_source_pixels * (float)GAME.dimensions().ppm / (float)sprite->ppm();
		return vector_in_destination_pixels;
	} else {
		return {};
	}
}

m2::VecF m2::Graphic::screen_origin_to_sprite_center_px() const {
	return screen_origin_to_position_px(parent().position) - sprite_center_to_sprite_origin_px();
}

void m2::Graphic::default_draw(Graphic& gfx) {
	if (not gfx.sprite) {
		// This function only draws sprites
		return;
	}

	// Select correct texture
	auto* texture = gfx.sprite->sprite_sheet().texture();
	if (gfx.draw_sprite_effect) {
		texture = gfx.sprite->effects_texture();
	}

	// Select the correct source rectangle
	auto src_rect = sdl::to_rect(gfx.sprite->sprite().rect());
	if (gfx.draw_sprite_effect) {
		src_rect = gfx.sprite->effect_rect(gfx.draw_sprite_effect);
	}

	if (m2g::camera_height != 0.0f) {
		// Draw two triangles in one call
		// 0****1
		// *   **
		// *  * *
		// * *  *
		// **   *
		// 2****3

		auto position = gfx.parent().position;
		auto position3 = m3::VecF{position.x, position.y, 0.0f};

		m3::VecF point_0, point_1, point_2, point_3;

		// Check if foreground or background
		bool is_foreground = LEVEL.graphics.get_id(&gfx);
		if (is_foreground) {
			auto sprite_x_offset_in_dest_px = gfx.sprite_center_to_sprite_origin_px().x;
			auto point_0_not_rotated = m3::VecF{
					position.x - ((float)src_rect.w / (float)gfx.sprite->ppm() / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
					position.y,
					(float)src_rect.h / (float)gfx.sprite->ppm()
			};
			auto point_1_not_rotated = m3::VecF{
					position.x + ((float)src_rect.w / (float)gfx.sprite->ppm() / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
					position.y,
					(float)src_rect.h / (float)gfx.sprite->ppm()
			};
			auto point_2_not_rotated = m3::VecF{
					position.x - ((float)src_rect.w / (float)gfx.sprite->ppm() / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
					position.y,
					0.0f
			};
			auto point_3_not_rotated = m3::VecF{
					position.x + ((float)src_rect.w / (float)gfx.sprite->ppm() / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
					position.y,
					0.0f
			};

			auto position_to_point_0_not_rotated = point_0_not_rotated - position3;
			auto position_to_point_1_not_rotated = point_1_not_rotated - position3;
			auto position_to_point_2_not_rotated = point_2_not_rotated - position3;
			auto position_to_point_3_not_rotated = point_3_not_rotated - position3;

			// Apply sprite and object rotation
			auto xz_rotation = gfx.sprite->original_rotation_radians();
			auto xy_rotation = gfx.draw_angle;
			auto position_to_point_0 = position_to_point_0_not_rotated.rotate_xz(xz_rotation).rotate_xy(xy_rotation);
			auto position_to_point_1 = position_to_point_1_not_rotated.rotate_xz(xz_rotation).rotate_xy(xy_rotation);
			auto position_to_point_2 = position_to_point_2_not_rotated.rotate_xz(xz_rotation).rotate_xy(xy_rotation);
			auto position_to_point_3 = position_to_point_3_not_rotated.rotate_xz(xz_rotation).rotate_xy(xy_rotation);

			point_0 = (position_to_point_0 + position3).offset_z(gfx.z);
			point_1 = (position_to_point_1 + position3).offset_z(gfx.z);
			point_2 = (position_to_point_2 + position3).offset_z(gfx.z);
			point_3 = (position_to_point_3 + position3).offset_z(gfx.z);
		} else {
			// Background sprite
			auto sprite_x_offset_in_dest_px = gfx.sprite_center_to_sprite_origin_px().x;
			auto sprite_y_offset_in_dest_px = gfx.sprite_center_to_sprite_origin_px().y;
			auto point_0_not_rotated = m3::VecF{
					position.x - ((float)src_rect.w / (float)gfx.sprite->ppm() / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
					position.y - ((float)src_rect.h / (float)gfx.sprite->ppm() / 2.0f) - (sprite_y_offset_in_dest_px / GAME.dimensions().ppm),
					0.0f
			};
			auto point_1_not_rotated = m3::VecF{
					position.x + ((float)src_rect.w / (float)gfx.sprite->ppm() / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
					position.y - ((float)src_rect.h / (float)gfx.sprite->ppm() / 2.0f) - (sprite_y_offset_in_dest_px / GAME.dimensions().ppm),
					0.0f
			};
			auto point_2_not_rotated = m3::VecF{
					position.x - ((float)src_rect.w / (float)gfx.sprite->ppm() / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
					position.y + ((float)src_rect.h / (float)gfx.sprite->ppm() / 2.0f) - (sprite_y_offset_in_dest_px / GAME.dimensions().ppm),
					0.0f
			};
			auto point_3_not_rotated = m3::VecF{
					position.x + ((float)src_rect.w / (float)gfx.sprite->ppm() / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
					position.y + ((float)src_rect.h / (float)gfx.sprite->ppm() / 2.0f) - (sprite_y_offset_in_dest_px / GAME.dimensions().ppm),
					0.0f
			};

			auto position_to_point_0_not_rotated = point_0_not_rotated - position3;
			auto position_to_point_1_not_rotated = point_1_not_rotated - position3;
			auto position_to_point_2_not_rotated = point_2_not_rotated - position3;
			auto position_to_point_3_not_rotated = point_3_not_rotated - position3;

			// Apply sprite and object rotation
			auto xy_rotation = gfx.draw_angle - gfx.sprite->original_rotation_radians();
			auto position_to_point_0 = position_to_point_0_not_rotated.rotate_xy(xy_rotation);
			auto position_to_point_1 = position_to_point_1_not_rotated.rotate_xy(xy_rotation);
			auto position_to_point_2 = position_to_point_2_not_rotated.rotate_xy(xy_rotation);
			auto position_to_point_3 = position_to_point_3_not_rotated.rotate_xy(xy_rotation);

			point_0 = (position_to_point_0 + position3).offset_z(gfx.z);
			point_1 = (position_to_point_1 + position3).offset_z(gfx.z);
			point_2 = (position_to_point_2 + position3).offset_z(gfx.z);
			point_3 = (position_to_point_3 + position3).offset_z(gfx.z);
		}

		auto projected_point_0 = m3::screen_origin_to_projection_of_position_px(point_0);
		auto projected_point_1 = m3::screen_origin_to_projection_of_position_px(point_1);
		auto projected_point_2 = m3::screen_origin_to_projection_of_position_px(point_2);
		auto projected_point_3 = m3::screen_origin_to_projection_of_position_px(point_3);

		if (projected_point_0 && projected_point_1 && projected_point_2 && projected_point_3) {
			auto texture_width = (float) (gfx.draw_sprite_effect ?
					gfx.sprite->effects_sheet()->texture_width() : gfx.sprite->sprite_sheet().surface()->w);
			auto texture_height = (float) (gfx.draw_sprite_effect ?
					gfx.sprite->effects_sheet()->texture_height() : gfx.sprite->sprite_sheet().surface()->h);

			SDL_Vertex vertices[4] = {};
			vertices[0].position = static_cast<SDL_FPoint>(*projected_point_0);
			vertices[0].color = {255, 255, 255, 255};
			vertices[0].tex_coord = SDL_FPoint{
					(float)src_rect.x / texture_width,
					(float)src_rect.y / texture_height,
			};

			vertices[1].position = static_cast<SDL_FPoint>(*projected_point_1);
			vertices[1].color = {255, 255, 255, 255};
			vertices[1].tex_coord = SDL_FPoint{
					(float)(src_rect.x + src_rect.w) / texture_width,
					(float)(src_rect.y) / texture_height,
			};

			vertices[2].position = static_cast<SDL_FPoint>(*projected_point_2);
			vertices[2].color = {255, 255, 255, 255};
			vertices[2].tex_coord = SDL_FPoint{
					(float)src_rect.x / texture_width,
					(float)(src_rect.y + src_rect.h) / texture_height,
			};

			vertices[3].position = static_cast<SDL_FPoint>(*projected_point_3);
			vertices[3].color = {255, 255, 255, 255};
			vertices[3].tex_coord = SDL_FPoint{
					(float)(src_rect.x + src_rect.w) / texture_width,
					(float)(src_rect.y + src_rect.h) / texture_height,
			};

			static const int indices[6] = {0, 1, 2, 2, 1, 3};

			SDL_SetRenderDrawBlendMode(GAME.renderer, SDL_BLENDMODE_BLEND);
			SDL_RenderGeometry(GAME.renderer, texture, vertices, 4, indices, 6);
		}
	} else {
		auto screen_origin_to_sprite_center_px = gfx.screen_origin_to_sprite_center_px();
		auto dst_rect = SDL_Rect{
				(int)roundf(screen_origin_to_sprite_center_px.x) - (src_rect.w * GAME.dimensions().ppm / gfx.sprite->ppm() / 2),
				(int)roundf(screen_origin_to_sprite_center_px.y) - (src_rect.h * GAME.dimensions().ppm / gfx.sprite->ppm() / 2),
				src_rect.w * GAME.dimensions().ppm / gfx.sprite->ppm(),
				src_rect.h * GAME.dimensions().ppm / gfx.sprite->ppm()
		};

		// Sprite is rotated around this point
		auto center_offset = gfx.sprite_center_to_sprite_origin_px();
		auto center_point = SDL_Point{
				(int)roundf(center_offset.x) + dst_rect.w / 2 ,
				(int)roundf(center_offset.y) + dst_rect.h / 2
		};

		auto original_rotation = gfx.sprite->original_rotation_radians();
		if (SDL_RenderCopyEx(GAME.renderer, texture, &src_rect, &dst_rect, m2::to_degrees(gfx.draw_angle - original_rotation), &center_point, SDL_FLIP_NONE)) {
			throw M2ERROR("SDL error while drawing: " + std::string(SDL_GetError()));
		}
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
		auto src_rect = sdl::to_rect(gfx.sprite->sprite().rect());
		auto screen_origin_to_sprite_center_px = gfx.screen_origin_to_sprite_center_px();
		dst_rect = SDL_Rect{
				(int) roundf(screen_origin_to_sprite_center_px.x) - (src_rect.w * GAME.dimensions().ppm / gfx.sprite->ppm() / 2),
				(int) roundf(screen_origin_to_sprite_center_px.y) + (src_rect.h * GAME.dimensions().ppm * 11 / gfx.sprite->ppm() / 2 / 10), // Give an offset of 1.1
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
