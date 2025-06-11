#include <m2/component/Light.h>
#include <m2/Component.h>
#include <m2/Game.h>
#include <m2/box2d/Query.h>
#include <m2/box2d/RayCast.h>
#include <m2/Proxy.h>
#include <m2/Object.h>

m2::Light::Light(Id object_id) : Component(object_id), radiusM(0.0f), onDraw(DefaultDrawCallback) {}

void m2::Light::DefaultDrawCallback(Light& lig) {
	if (not M2G_PROXY.lightning) {
		return;
	}

	auto& obj = M2_LEVEL.objects[lig.OwnerId()];

	// Check if dynamic lightning
	auto category_bits = lig.dynamicCategoryBits;
	if (category_bits) {
		// Check if the object is inside the object
		bool inside_body = false;
		box2d::Query(*M2_LEVEL.world[I(PhysicsLayer::P0)], AABB{obj.position, 0.005f}, [&inside_body, category_bits](b2Fixture& fixture) {
			if (fixture.GetFilterData().categoryBits & category_bits) {
				inside_body = true;
				return false;
			}
			return true;
		});

		if (not inside_body) {
			auto position_px = ScreenOriginToPositionVecPx(obj.position);

			std::vector<SDL_Vertex> vertices;
			// The vector that'll be rotated for raycasting
			VecF full_span_m{lig.radiusM, 0.0f};
			constexpr int steps = 360;
			for (unsigned i = 0; i < steps; ++i) {
				uint8_t max_brightness = 255;
				// Center point of the triangle
				vertices.push_back(SDL_Vertex{.position = static_cast<SDL_FPoint>(position_px), .color = {max_brightness, max_brightness, max_brightness, 0}});

				// Ray cast towards full_span_m
				auto distance = box2d::CheckDistance(*M2_LEVEL.world[I(PhysicsLayer::P0)], obj.position, obj.position + full_span_m, category_bits);
				// Calculate brightness based on collision distance
				auto brightness = (uint8_t)roundf((float)max_brightness * (1.0f - distance / lig.radiusM));
				// Cut-off vector
				auto span_m = full_span_m.WithLength(distance);
				// Cut-off vector in pixels
				auto span_px = span_m * M2_GAME.Dimensions().OutputPixelsPerMeter();
				// Second point of the triangle
				vertices.push_back(SDL_Vertex{.position = static_cast<SDL_FPoint>(position_px + span_px), .color = {brightness, brightness, brightness, 0}});

				// Rotate full_span_m for next iteration
				full_span_m = full_span_m.Rotate(PI_MUL2 / static_cast<float>(steps));
				// Rotate cut-off vector
				auto span2_px = span_px.Rotate(PI_MUL2 / static_cast<float>(steps));
				// Third point of the triangle
				vertices.push_back(SDL_Vertex{.position = static_cast<SDL_FPoint>(position_px + span2_px), .color = {brightness, brightness, brightness, 0}});
			}
			SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_MUL);
			SDL_RenderGeometry(M2_GAME.renderer, nullptr, vertices.data(), (int)vertices.size(), nullptr, 0);
			SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
		}
	} else {
		auto position_px = ScreenOriginToPositionVecPx(obj.position);
		MAYBE auto dstrect = SDL_Rect{
				(int)roundf(position_px.x - lig.radiusM * M2_GAME.Dimensions().OutputPixelsPerMeter()),
				(int)roundf(position_px.y - lig.radiusM * M2_GAME.Dimensions().OutputPixelsPerMeter()),
				(int)roundf((float)lig.radiusM * M2_GAME.Dimensions().OutputPixelsPerMeter() * 2.0f),
				(int)roundf((float)lig.radiusM * M2_GAME.Dimensions().OutputPixelsPerMeter() * 2.0f)
		};
		SDL_RenderCopy(M2_GAME.renderer, M2_GAME.light_texture, nullptr, &dstrect);
	}
}
