#include "m2/component/Physique.h"
#include <m2/box2d/Utils.h>
#include <m2/Game.hh>
#include <m2/Shape.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_shape.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>
#include <m2/Object.h>

m2::Physique::Physique(Id object_id) : Component(object_id), body(nullptr) {}

m2::Physique::Physique(Physique&& other) noexcept : Component(other.object_id), body(other.body), on_collision(std::move(other.on_collision)), off_collision(std::move(other.off_collision)) {
    other.body = nullptr;
}

m2::Physique& m2::Physique::operator=(Physique&& other) noexcept {
    std::swap(object_id, other.object_id);
    std::swap(body, other.body);
	std::swap(on_collision, other.on_collision);
	std::swap(off_collision, other.off_collision);
    return *this;
}

m2::Physique::~Physique() {
	if (body) {
		m2::box2d::destroy_body(body);
	}
}

void m2::Physique::draw_debug_shapes() const {
	if (!body) {
		return;
	}

	auto position = Vec2f{body->GetPosition()};
	for (auto* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
		b2AABB aabb;
		fixture->GetShape()->ComputeAABB(&aabb, body->GetTransform(), 0);

		switch (fixture->GetType()) {
			case b2Shape::Type::e_polygon: {
				const auto* shape = dynamic_cast<const b2PolygonShape*>(fixture->GetShape());
				int rect_w = (int)roundf((aabb.upperBound.x - aabb.lowerBound.x) * (float)GAME.game_ppm);
				int rect_h = (int)roundf((aabb.upperBound.y - aabb.lowerBound.y) * (float)GAME.game_ppm);
				auto [texture, src_rect] = GAME.shapes_sheet->get_rectangle_aa(SDL_Color{255, 0, 0, 255}, rect_w, rect_h);

				auto center_offset_m = Vec2f{shape->m_centroid};

				auto screen_origin_to_sprite_center_px = screen_origin_to_position_px(position + center_offset_m);
				auto dst_rect = SDL_Rect{
					(int)roundf(screen_origin_to_sprite_center_px.x) - (src_rect.w / 2),
					(int)roundf(screen_origin_to_sprite_center_px.y) - (src_rect.h / 2),
					src_rect.w,
					src_rect.h
				};

				if (SDL_RenderCopy(GAME.sdlRenderer, texture, &src_rect, &dst_rect)) {
					throw M2ERROR("SDL error while drawing: " + std::string(SDL_GetError()));
				}
				break;
			}
			case b2Shape::Type::e_circle: {
				const auto* shape = dynamic_cast<const b2CircleShape*>(fixture->GetShape());
				int r = (int)roundf((aabb.upperBound.x - aabb.lowerBound.x) * (float)GAME.game_ppm) / 2;
				auto [texture, src_rect] = GAME.shapes_sheet->get_circle(SDL_Color{255, 0, 0, 255}, r);

				auto center_offset_m = Vec2f{shape->m_p};

				auto screen_origin_to_sprite_center_px = screen_origin_to_position_px(position + center_offset_m);
				auto dst_rect = SDL_Rect{
					(int)roundf(screen_origin_to_sprite_center_px.x) - (src_rect.w / 2),
					(int)roundf(screen_origin_to_sprite_center_px.y) - (src_rect.h / 2),
					src_rect.w,
					src_rect.h
				};

				if (SDL_RenderCopy(GAME.sdlRenderer, texture, &src_rect, &dst_rect)) {
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
	auto& phy_a = GAME.physics[physique_id_a];
	auto& phy_b = GAME.physics[physique_id_b];
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
	auto& phy_a = GAME.physics[physique_id_a];
	auto& phy_b = GAME.physics[physique_id_b];
	if (phy_a.off_collision) {
		phy_a.off_collision(phy_a, phy_b);
	}
	if (phy_b.off_collision) {
		phy_b.off_collision(phy_b, phy_a);
	}
}

float m2::calculate_limited_force(float curr_velocity, float speed_limit) {
	return logf(fabs(speed_limit) - fabs(curr_velocity));
}
