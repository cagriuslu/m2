#include <m2/component/Graphic.h>
#include <m2/Game.h>

void m2::draw_real_2d(const VecF& position, const Sprite& sprite, SpriteVariant sprite_variant, float angle) {
	auto src_rect = sprite.Rect(sprite_variant);
	auto sprite_ppm = sprite.Ppm();

	auto screen_origin_to_sprite_center_px_vec = screen_origin_to_sprite_center_dstpx(position, sprite, sprite_variant);
	auto dst_rect = SDL_Rect{
		I(screen_origin_to_sprite_center_px_vec.x - (F(src_rect.w) * F(M2_GAME.Dimensions().RealOutputPixelsPerMeter()) / F(sprite_ppm) / 2.0f)),
		I(screen_origin_to_sprite_center_px_vec.y - (F(src_rect.h) * F(M2_GAME.Dimensions().RealOutputPixelsPerMeter()) / F(sprite_ppm) / 2.0f)),
		(int)ceilf(F(src_rect.w) * F(M2_GAME.Dimensions().RealOutputPixelsPerMeter()) / F(sprite_ppm)),
		(int)ceilf(F(src_rect.h) * F(M2_GAME.Dimensions().RealOutputPixelsPerMeter()) / F(sprite_ppm))
		// TODO using I() and ceilf() here is quite problematic, but I couldn't find any other way of ensuring not
		//  leaving any gaps between sprites
		// TODO unfortunately, we can't draw pixel perfect sprites with floating point scaling. However, the game can
		//  avoid flickering by avoiding highly repeating patterns.
	};

	// Sprite is rotated around this point
	auto center_offset = sprite.CenterToOriginDstpx(sprite_variant);
	auto center_point = SDL_Point{
			(int)roundf(center_offset.x) + dst_rect.w / 2 ,
			(int)roundf(center_offset.y) + dst_rect.h / 2
	};

	auto src_rect_sdl = static_cast<SDL_Rect>(src_rect);
	auto original_rotation = sprite.OriginalRotationRadians();
	if (SDL_RenderCopyEx(M2_GAME.renderer, sprite.Texture(sprite_variant), &src_rect_sdl, &dst_rect, m2::to_degrees(angle - original_rotation), &center_point, SDL_FLIP_NONE)) {
		throw M2_ERROR("SDL error while drawing: " + std::string(SDL_GetError()));
	}
}

void m2::draw_fake_3d(const VecF& position, const Sprite& sprite, SpriteVariant sprite_variant, float angle, bool is_foreground, float z) {
	// Draw two triangles in one call
	// 0****1
	// *   **
	// *  * *
	// * *  *
	// **   *
	// 2****3

	auto src_rect = sprite.Rect(sprite_variant);
	auto sprite_ppm_f = static_cast<float>(sprite.Ppm());
	auto position3 = m3::VecF{position.x, position.y, 0.0f};
	m3::VecF point_0, point_1, point_2, point_3;
	if (is_foreground) {
		auto sprite_x_offset_in_dest_px = sprite.CenterToOriginDstpx(sprite_variant).x;
		auto point_0_not_rotated = m3::VecF{
				position.x - ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				position.y,
				(float)src_rect.h / (float)sprite.Ppm()
		};
		auto point_1_not_rotated = m3::VecF{
				position.x + ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				position.y,
				(float)src_rect.h / (float)sprite.Ppm()
		};
		auto point_2_not_rotated = m3::VecF{
				position.x - ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				position.y,
				0.0f
		};
		auto point_3_not_rotated = m3::VecF{
				position.x + ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				position.y,
				0.0f
		};

		auto position_to_point_0_not_rotated = point_0_not_rotated - position3;
		auto position_to_point_1_not_rotated = point_1_not_rotated - position3;
		auto position_to_point_2_not_rotated = point_2_not_rotated - position3;
		auto position_to_point_3_not_rotated = point_3_not_rotated - position3;

		// Apply sprite and object rotation
		auto xz_rotation = sprite.OriginalRotationRadians();
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
		auto sprite_x_offset_in_dest_px = sprite.CenterToOriginDstpx(sprite_variant).x;
		auto sprite_y_offset_in_dest_px = sprite.CenterToOriginDstpx(sprite_variant).y;
		auto point_0_not_rotated = m3::VecF{
				position.x - ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				position.y - ((float)src_rect.h / sprite_ppm_f / 2.0f) - (sprite_y_offset_in_dest_px / M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				0.0f
		};
		auto point_1_not_rotated = m3::VecF{
				position.x + ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				position.y - ((float)src_rect.h / sprite_ppm_f / 2.0f) - (sprite_y_offset_in_dest_px / M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				0.0f
		};
		auto point_2_not_rotated = m3::VecF{
				position.x - ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				position.y + ((float)src_rect.h / sprite_ppm_f / 2.0f) - (sprite_y_offset_in_dest_px / M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				0.0f
		};
		auto point_3_not_rotated = m3::VecF{
				position.x + ((float)src_rect.w / sprite_ppm_f / 2.0f) - (sprite_x_offset_in_dest_px / M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				position.y + ((float)src_rect.h / sprite_ppm_f / 2.0f) - (sprite_y_offset_in_dest_px / M2_GAME.Dimensions().RealOutputPixelsPerMeter()),
				0.0f
		};

		auto position_to_point_0_not_rotated = point_0_not_rotated - position3;
		auto position_to_point_1_not_rotated = point_1_not_rotated - position3;
		auto position_to_point_2_not_rotated = point_2_not_rotated - position3;
		auto position_to_point_3_not_rotated = point_3_not_rotated - position3;

		// Apply sprite and object rotation
		auto xy_rotation = angle - sprite.OriginalRotationRadians();
		auto position_to_point_0 = position_to_point_0_not_rotated.rotate_xy(xy_rotation);
		auto position_to_point_1 = position_to_point_1_not_rotated.rotate_xy(xy_rotation);
		auto position_to_point_2 = position_to_point_2_not_rotated.rotate_xy(xy_rotation);
		auto position_to_point_3 = position_to_point_3_not_rotated.rotate_xy(xy_rotation);

		point_0 = (position_to_point_0 + position3).offset_z(z);
		point_1 = (position_to_point_1 + position3).offset_z(z);
		point_2 = (position_to_point_2 + position3).offset_z(z);
		point_3 = (position_to_point_3 + position3).offset_z(z);
	}

	auto projected_point_0 = m3::screen_origin_to_projection_along_camera_plane_dstpx(point_0);
	auto projected_point_1 = m3::screen_origin_to_projection_along_camera_plane_dstpx(point_1);
	auto projected_point_2 = m3::screen_origin_to_projection_along_camera_plane_dstpx(point_2);
	auto projected_point_3 = m3::screen_origin_to_projection_along_camera_plane_dstpx(point_3);

	if (projected_point_0 && projected_point_1 && projected_point_2 && projected_point_3) {
		auto texture_dims = sprite.TextureTotalDims(sprite_variant);

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

		SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderGeometry(M2_GAME.renderer, sprite.Texture(sprite_variant), vertices, 4, indices, 6);
	}
}
