#include "m2/component/Physique.h"
#include <m2/box2d/Detail.h>
#include <m2/Game.h>
#include <m2/Shape.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_shape.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>
#include <m2/Object.h>

m2::Physique::Physique(Id object_id) : Component(object_id), body(nullptr) {}

m2::Physique::Physique(Physique&& other) noexcept : Component(other.object_id), pre_step(std::move(other.pre_step)), post_step(std::move(other.post_step)), body(std::move(other.body)), on_collision(std::move(other.on_collision)), off_collision(std::move(other.off_collision)) {
    other.body = nullptr;
}

m2::Physique& m2::Physique::operator=(Physique&& other) noexcept {
    std::swap(object_id, other.object_id);
    std::swap(pre_step, other.pre_step);
    std::swap(post_step, other.post_step);
    std::swap(body, other.body);
	std::swap(on_collision, other.on_collision);
	std::swap(off_collision, other.off_collision);
    return *this;
}

void m2::Physique::draw_debug_shapes() const {
	if (!body) {
		return;
	}

	auto position = Vec2f{body->GetPosition()}; // Position of the object origin
	for (auto* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
		b2AABB aabb;
		fixture->GetShape()->ComputeAABB(&aabb, body->GetTransform(), 0);

		// Pick different color for background and foreground colliders
		auto color = box2d::does_category_have_background_bits(fixture->GetFilterData().categoryBits) ?
			SDL_Color{255, 0, 0, 255} :
			SDL_Color{255, 255, 0, 255};

		switch (fixture->GetType()) {
			case b2Shape::Type::e_polygon: {
				const auto* shape = dynamic_cast<const b2PolygonShape*>(fixture->GetShape());
				int rect_w = (int)roundf((aabb.upperBound.x - aabb.lowerBound.x) * (float)GAME.dimensions().ppm);
				int rect_h = (int)roundf((aabb.upperBound.y - aabb.lowerBound.y) * (float)GAME.dimensions().ppm);

				// Decompose the "object origin" to "shape centroid" vector
				// (shape doesn't know where the object origin is, or the angle of the object)
				auto centroid_offset_m = Vec2f{shape->m_centroid};
				auto centroid_offset_length_m = centroid_offset_m.length();
				auto centroid_offset_angle = centroid_offset_m.angle_rads();
				// Current angle of the object in the world
				auto current_angle = body->GetTransform().q.GetAngle();
				// Compose the "object origin" to "current shape centroid" vector
				auto center_offset_m = Vec2f::from_angle(centroid_offset_angle + current_angle).with_length(centroid_offset_length_m);

				auto screen_origin_to_sprite_center_px = screen_origin_to_position_px(position + center_offset_m);
				auto dst_rect = SDL_Rect{
					(int)roundf(screen_origin_to_sprite_center_px.x) - (rect_w / 2),
					(int)roundf(screen_origin_to_sprite_center_px.y) - (rect_h / 2),
					rect_w,
					rect_h
				};
				SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
				SDL_RenderDrawRect(GAME.renderer, &dst_rect);
				break;
			}
			case b2Shape::Type::e_circle: {
				const auto* shape = dynamic_cast<const b2CircleShape*>(fixture->GetShape());
				int R = (int)roundf((aabb.upperBound.x - aabb.lowerBound.x) * (float)GAME.dimensions().ppm);
				auto [texture, src_rect] = GAME.shapes_sheet->get_circle(color, R, R, 16);

				auto center_offset_m = Vec2f{shape->m_p};

				auto screen_origin_to_sprite_center_px = screen_origin_to_position_px(position + center_offset_m);
				auto dst_rect = SDL_Rect{
					(int)roundf(screen_origin_to_sprite_center_px.x) - (src_rect.w / 2),
					(int)roundf(screen_origin_to_sprite_center_px.y) - (src_rect.h / 2),
					src_rect.w,
					src_rect.h
				};

				if (SDL_RenderCopy(GAME.renderer, texture, &src_rect, &dst_rect)) {
					throw M2ERROR("SDL error while drawing: " + std::string(SDL_GetError()));
				}
				break;
			}
			default:
				throw M2FATAL("Unsupported shape");
		}
	}
}

void m2::Physique::default_begin_contact_cb(b2Contact& b2_contact) {
	box2d::Contact contact{b2_contact};

	Id physique_id_a = b2_contact.GetFixtureA()->GetBody()->GetUserData().pointer;
	Id physique_id_b = b2_contact.GetFixtureB()->GetBody()->GetUserData().pointer;
	auto& phy_a = LEVEL.physics[physique_id_a];
	auto& phy_b = LEVEL.physics[physique_id_b];
	if (phy_a.on_collision) {
		phy_a.on_collision(phy_a, phy_b, contact);
	}
	if (phy_b.on_collision) {
		phy_b.on_collision(phy_b, phy_a, contact);
	}
}
void m2::Physique::default_end_contact_cb(b2Contact& b2_contact) {
	Id physique_id_a = b2_contact.GetFixtureA()->GetBody()->GetUserData().pointer;
	Id physique_id_b = b2_contact.GetFixtureB()->GetBody()->GetUserData().pointer;
	auto& phy_a = LEVEL.physics[physique_id_a];
	auto& phy_b = LEVEL.physics[physique_id_b];
	if (phy_a.off_collision) {
		phy_a.off_collision(phy_a, phy_b);
	}
	if (phy_b.off_collision) {
		phy_b.off_collision(phy_b, phy_a);
	}
}

float m2::calculate_limited_force(float curr_speed, float speed_limit) {
	return logf(abs(speed_limit) - abs(curr_speed) + 1.0f); // ln(1) = 0
}
