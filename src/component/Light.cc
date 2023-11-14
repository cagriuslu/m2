#include <m2/component/Light.h>
#include <m2/Component.h>
#include <m2/Game.h>
#include <m2/box2d/Query.h>
#include <m2/box2d/RayCast.h>
#include <m2/Proxy.h>
#include <m2/Object.h>

m2::Light::Light(Id object_id) : Component(object_id), radius_m(0.0f), on_draw(default_draw) {}

void m2::Light::default_draw(Light& lig) {
	if (not m2g::lightning) {
		return;
	}

	auto& obj = LEVEL.objects[lig.object_id];

	// Check if dynamic lightning
	auto category_bits = lig.dynamic_category_bits;
	if (category_bits) {
		// Check if the object is inside the object
		bool inside_body = false;
		box2d::query(*LEVEL.world, Aabb{obj.position, 0.005f}, [&inside_body, category_bits](b2Fixture& fixture) {
			if (fixture.GetFilterData().categoryBits & category_bits) {
				inside_body = true;
				return false;
			}
			return true;
		});

		if (not inside_body) {
			auto position_px = screen_origin_to_position_dstpx(obj.position);

			std::vector<SDL_Vertex> vertices;
			// The vector that'll be rotated for raycasting
			VecF full_span_m{lig.radius_m, 0.0f};
			constexpr int steps = 360;
			for (unsigned i = 0; i < steps; ++i) {
				uint8_t max_brightness = 255;
				// Center point of the triangle
				vertices.push_back(SDL_Vertex{.position = static_cast<SDL_FPoint>(position_px), .color = {max_brightness, max_brightness, max_brightness, 0}});

				// Ray cast towards full_span_m
				auto distance = box2d::check_distance(*LEVEL.world, obj.position, obj.position + full_span_m, category_bits);
				// Calculate brightness based on collision distance
				auto brightness = (uint8_t)roundf((float)max_brightness * (1.0f - distance / lig.radius_m));
				// Cut-off vector
				auto span_m = full_span_m.with_length(distance);
				// Cut-off vector in pixels
				auto span_px = span_m * GAME.dimensions().ppm;
				// Second point of the triangle
				vertices.push_back(SDL_Vertex{.position = static_cast<SDL_FPoint>(position_px + span_px), .color = {brightness, brightness, brightness, 0}});

				// Rotate full_span_m for next iteration
				full_span_m = full_span_m.rotate(PI_MUL2 / static_cast<float>(steps));
				// Rotate cut-off vector
				auto span2_px = span_px.rotate(PI_MUL2 / static_cast<float>(steps));
				// Third point of the triangle
				vertices.push_back(SDL_Vertex{.position = static_cast<SDL_FPoint>(position_px + span2_px), .color = {brightness, brightness, brightness, 0}});
			}
			SDL_SetRenderDrawBlendMode(GAME.renderer, SDL_BLENDMODE_MUL);
			SDL_RenderGeometry(GAME.renderer, nullptr, vertices.data(), (int)vertices.size(), nullptr, 0);
			SDL_SetRenderDrawBlendMode(GAME.renderer, SDL_BLENDMODE_BLEND);
		}
	} else {
		auto position_px = screen_origin_to_position_dstpx(obj.position);
		MAYBE auto dstrect = SDL_Rect{
				(int)roundf(position_px.x - lig.radius_m * GAME.dimensions().ppm),
				(int)roundf(position_px.y - lig.radius_m * GAME.dimensions().ppm),
				(int)roundf((float)lig.radius_m * GAME.dimensions().ppm * 2.0f),
				(int)roundf((float)lig.radius_m * GAME.dimensions().ppm * 2.0f)
		};
		SDL_RenderCopy(GAME.renderer, GAME.light_texture, nullptr, &dstrect);
	}
}
