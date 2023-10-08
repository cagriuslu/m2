#include <m2/component/Graphic.h>
#include <m2/Game.h>

namespace {
	inline SDL_Texture* find_texture(const m2::Sprite& sprite, m2::pb::SpriteEffectType effect_type) {
		// Select correct texture
		return effect_type ? sprite.effects_texture() : sprite.sprite_sheet().texture();
	}

	inline m2::RectI find_rect(const m2::Sprite& sprite, m2::pb::SpriteEffectType effect_type) {
		// Select the correct source rectangle
		return effect_type ? sprite.effect_rect(effect_type) : sprite.rect();
	}

	inline m2::VecF total_texture_dimensions(const m2::Sprite& sprite, m2::pb::SpriteEffectType effect_type) {
		if (effect_type) {
			return {sprite.effects_sheet()->texture_width(), sprite.effects_sheet()->texture_height()};
		} else {
			return {sprite.sprite_sheet().surface()->w, sprite.sprite_sheet().surface()->h};
		}
	}
}

m2::VecF m2::sprite_center_to_sprite_origin_px(const Sprite& sprite, pb::SpriteEffectType effect_type) {
	// Convert from source pixels to destination pixels
	return sprite.center_to_origin_px(effect_type) * sprite.sheet_to_screen_pixel_multiplier();
}

m2::VecF m2::screen_origin_to_sprite_center_px(const VecF& position, const Sprite& sprite, pb::SpriteEffectType effect_type) {
	return screen_origin_to_position_px(position) - sprite_center_to_sprite_origin_px(sprite, effect_type);
}

void m2::draw_real_2d(const VecF& position, const Sprite& sprite, pb::SpriteEffectType effect_type, float angle) {
	auto src_rect = find_rect(sprite, effect_type);
	auto sprite_ppm = sprite.ppm();

	auto screen_origin_to_sprite_center_px_vec = screen_origin_to_sprite_center_px(position, sprite, effect_type);
	auto dst_rect = SDL_Rect{
			(int)roundf(screen_origin_to_sprite_center_px_vec.x) - (src_rect.w * GAME.dimensions().ppm / sprite_ppm / 2),
			(int)roundf(screen_origin_to_sprite_center_px_vec.y) - (src_rect.h * GAME.dimensions().ppm / sprite_ppm / 2),
			src_rect.w * GAME.dimensions().ppm / sprite_ppm,
			src_rect.h * GAME.dimensions().ppm / sprite_ppm
	};

	// Sprite is rotated around this point
	auto center_offset = sprite_center_to_sprite_origin_px(sprite, effect_type);
	auto center_point = SDL_Point{
			(int)roundf(center_offset.x) + dst_rect.w / 2 ,
			(int)roundf(center_offset.y) + dst_rect.h / 2
	};

	auto src_rect_sdl = static_cast<SDL_Rect>(src_rect);
	auto original_rotation = sprite.original_rotation_radians();
	if (SDL_RenderCopyEx(GAME.renderer, find_texture(sprite, effect_type), &src_rect_sdl, &dst_rect, m2::to_degrees(angle - original_rotation), &center_point, SDL_FLIP_NONE)) {
		throw M2ERROR("SDL error while drawing: " + std::string(SDL_GetError()));
	}
}

void m2::draw_fake_3d(const VecF& position, const Sprite& sprite, pb::SpriteEffectType effect_type, float angle, bool is_foreground, float z) {
	// Draw two triangles in one call
	// 0****1
	// *   **
	// *  * *
	// * *  *
	// **   *
	// 2****3

	auto src_rect = find_rect(sprite, effect_type);
	auto sprite_ppm_f = static_cast<float>(sprite.ppm());
	auto position3 = m3::VecF{position.x, position.y, 0.0f};
	m3::VecF point_0, point_1, point_2, point_3;
	if (is_foreground) {
		auto sprite_x_offset_in_dest_px = sprite_center_to_sprite_origin_px(sprite, effect_type).x;
		auto point_0_not_rotated = m3::VecF{
				position.x - ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
				position.y,
				(float)src_rect.h / (float)sprite.ppm()
		};
		auto point_1_not_rotated = m3::VecF{
				position.x + ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
				position.y,
				(float)src_rect.h / (float)sprite.ppm()
		};
		auto point_2_not_rotated = m3::VecF{
				position.x - ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
				position.y,
				0.0f
		};
		auto point_3_not_rotated = m3::VecF{
				position.x + ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
				position.y,
				0.0f
		};

		auto position_to_point_0_not_rotated = point_0_not_rotated - position3;
		auto position_to_point_1_not_rotated = point_1_not_rotated - position3;
		auto position_to_point_2_not_rotated = point_2_not_rotated - position3;
		auto position_to_point_3_not_rotated = point_3_not_rotated - position3;

		// Apply sprite and object rotation
		auto xz_rotation = sprite.original_rotation_radians();
		auto xy_rotation = angle;
		auto position_to_point_0 = position_to_point_0_not_rotated.rotate_xz(xz_rotation).rotate_xy(xy_rotation);
		auto position_to_point_1 = position_to_point_1_not_rotated.rotate_xz(xz_rotation).rotate_xy(xy_rotation);
		auto position_to_point_2 = position_to_point_2_not_rotated.rotate_xz(xz_rotation).rotate_xy(xy_rotation);
		auto position_to_point_3 = position_to_point_3_not_rotated.rotate_xz(xz_rotation).rotate_xy(xy_rotation);

		point_0 = (position_to_point_0 + position3).offset_z(z);
		point_1 = (position_to_point_1 + position3).offset_z(z);
		point_2 = (position_to_point_2 + position3).offset_z(z);
		point_3 = (position_to_point_3 + position3).offset_z(z);
	} else {
		// Background sprite
		auto sprite_x_offset_in_dest_px = sprite_center_to_sprite_origin_px(sprite, effect_type).x;
		auto sprite_y_offset_in_dest_px = sprite_center_to_sprite_origin_px(sprite, effect_type).y;
		auto point_0_not_rotated = m3::VecF{
				position.x - ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
				position.y - ((float)src_rect.h / sprite_ppm_f / 2.0f) - (sprite_y_offset_in_dest_px / GAME.dimensions().ppm),
				0.0f
		};
		auto point_1_not_rotated = m3::VecF{
				position.x + ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
				position.y - ((float)src_rect.h / sprite_ppm_f / 2.0f) - (sprite_y_offset_in_dest_px / GAME.dimensions().ppm),
				0.0f
		};
		auto point_2_not_rotated = m3::VecF{
				position.x - ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
				position.y + ((float)src_rect.h / sprite_ppm_f / 2.0f) - (sprite_y_offset_in_dest_px / GAME.dimensions().ppm),
				0.0f
		};
		auto point_3_not_rotated = m3::VecF{
				position.x + ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / GAME.dimensions().ppm),
				position.y + ((float)src_rect.h / sprite_ppm_f / 2.0f) - (sprite_y_offset_in_dest_px / GAME.dimensions().ppm),
				0.0f
		};

		auto position_to_point_0_not_rotated = point_0_not_rotated - position3;
		auto position_to_point_1_not_rotated = point_1_not_rotated - position3;
		auto position_to_point_2_not_rotated = point_2_not_rotated - position3;
		auto position_to_point_3_not_rotated = point_3_not_rotated - position3;

		// Apply sprite and object rotation
		auto xy_rotation = angle - sprite.original_rotation_radians();
		auto position_to_point_0 = position_to_point_0_not_rotated.rotate_xy(xy_rotation);
		auto position_to_point_1 = position_to_point_1_not_rotated.rotate_xy(xy_rotation);
		auto position_to_point_2 = position_to_point_2_not_rotated.rotate_xy(xy_rotation);
		auto position_to_point_3 = position_to_point_3_not_rotated.rotate_xy(xy_rotation);

		point_0 = (position_to_point_0 + position3).offset_z(z);
		point_1 = (position_to_point_1 + position3).offset_z(z);
		point_2 = (position_to_point_2 + position3).offset_z(z);
		point_3 = (position_to_point_3 + position3).offset_z(z);
	}

	auto projected_point_0 = m3::screen_origin_to_projection_of_position_px(point_0);
	auto projected_point_1 = m3::screen_origin_to_projection_of_position_px(point_1);
	auto projected_point_2 = m3::screen_origin_to_projection_of_position_px(point_2);
	auto projected_point_3 = m3::screen_origin_to_projection_of_position_px(point_3);

	if (projected_point_0 && projected_point_1 && projected_point_2 && projected_point_3) {
		auto texture_dims = total_texture_dimensions(sprite, effect_type);

		SDL_Vertex vertices[4] = {};
		vertices[0].position = static_cast<SDL_FPoint>(*projected_point_0);
		vertices[0].color = {255, 255, 255, 255};
		vertices[0].tex_coord = SDL_FPoint{
				(float)src_rect.x / texture_dims.x,
				(float)src_rect.y / texture_dims.y,
		};

		vertices[1].position = static_cast<SDL_FPoint>(*projected_point_1);
		vertices[1].color = {255, 255, 255, 255};
		vertices[1].tex_coord = SDL_FPoint{
				(float)(src_rect.x + src_rect.w) / texture_dims.x,
				(float)(src_rect.y) / texture_dims.y,
		};

		vertices[2].position = static_cast<SDL_FPoint>(*projected_point_2);
		vertices[2].color = {255, 255, 255, 255};
		vertices[2].tex_coord = SDL_FPoint{
				(float)src_rect.x / texture_dims.x,
				(float)(src_rect.y + src_rect.h) / texture_dims.y,
		};

		vertices[3].position = static_cast<SDL_FPoint>(*projected_point_3);
		vertices[3].color = {255, 255, 255, 255};
		vertices[3].tex_coord = SDL_FPoint{
				(float)(src_rect.x + src_rect.w) / texture_dims.x,
				(float)(src_rect.y + src_rect.h) / texture_dims.y,
		};

		static const int indices[6] = {0, 1, 2, 2, 1, 3};

		SDL_SetRenderDrawBlendMode(GAME.renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderGeometry(GAME.renderer, find_texture(sprite, effect_type), vertices, 4, indices, 6);
	}
}
