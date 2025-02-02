#include "m2/component/Physique.h"
#include <m2/box2d/Detail.h>
#include <m2/Game.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_shape.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_edge_shape.h>
#include <m2/Object.h>

m2::Physique::Physique(Id object_id) : Component(object_id), body(nullptr) {}

m2::Physique::Physique(Physique&& other) noexcept : Component(other._owner_id), pre_step(std::move(other.pre_step)), post_step(std::move(other.post_step)), body(std::move(other.body)), rigidBodyIndex(std::move(other.rigidBodyIndex)), on_collision(std::move(other.on_collision)), off_collision(std::move(other.off_collision)) {
    other.body = nullptr;
	other.rigidBodyIndex = std::nullopt;
}

m2::Physique& m2::Physique::operator=(Physique&& other) noexcept {
    std::swap(_owner_id, other._owner_id);
    std::swap(pre_step, other.pre_step);
    std::swap(post_step, other.post_step);
    std::swap(body, other.body);
    std::swap(rigidBodyIndex, other.rigidBodyIndex);
	std::swap(on_collision, other.on_collision);
	std::swap(off_collision, other.off_collision);
    return *this;
}

void m2::Physique::default_debug_draw(Physique& phy) {
	if (!phy.body) {
		return;
	}

	auto position = VecF{phy.body->GetPosition()}; // Position of the object origin
	for (auto* fixture = phy.body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
		b2AABB aabb;
		fixture->GetShape()->ComputeAABB(&aabb, phy.body->GetTransform(), 0);

		// Pick different color for background and foreground colliders
		auto color = box2d::does_category_have_background_bits(fixture->GetFilterData().categoryBits) ?
			SDL_Color{255, 0, 0, 255} :
			SDL_Color{255, 255, 0, 255};

		switch (fixture->GetType()) {
			case b2Shape::Type::e_polygon: {
				const auto* shape = dynamic_cast<const b2PolygonShape*>(fixture->GetShape());
				auto width = aabb.upperBound.x - aabb.lowerBound.x;
				auto height = aabb.upperBound.y - aabb.lowerBound.y;

				// Decompose the "object origin" to "shape centroid" vector
				// (shape doesn't know where the object origin is, or the angle of the object)
				auto centroid_offset_m = VecF{shape->m_centroid};
				auto centroid_offset_length_m = centroid_offset_m.length();
				auto centroid_offset_angle = centroid_offset_m.angle_rads();
				// Current angle of the object in the world
				auto current_angle = phy.body->GetTransform().q.GetAngle();
				// Compose the "object origin" to "current shape centroid" vector
				auto center_offset_m = VecF::from_angle(centroid_offset_angle + current_angle).with_length(centroid_offset_length_m);

				if (is_projection_type_parallel(M2_LEVEL.ProjectionType())) {
					int rect_w = I(roundf(width * F(M2_GAME.Dimensions().OutputPixelsPerMeter())));
					int rect_h = I(roundf(height * F(M2_GAME.Dimensions().OutputPixelsPerMeter())));
					auto screen_origin_to_sprite_center_px = ScreenOriginToPositionVecPx(position + center_offset_m);
					auto dst_rect = SDL_Rect{
							(int)roundf(screen_origin_to_sprite_center_px.x) - (rect_w / 2),
							(int)roundf(screen_origin_to_sprite_center_px.y) - (rect_h / 2),
							rect_w,
							rect_h
					};
					SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
					SDL_RenderDrawRect(M2_GAME.renderer, &dst_rect);
				} else {
					auto center_position_2d = position + center_offset_m;
					auto center_position = m3::VecF{center_position_2d};
					// Draw a rectangle
					auto point_0 = m3::screen_origin_to_projection_along_camera_plane_dstpx(
							center_position.offset_x(-width / 2.0f).offset_y(-height / 2.0f));
					auto point_1 = m3::screen_origin_to_projection_along_camera_plane_dstpx(
							center_position.offset_x(width / 2.0f).offset_y(-height / 2.0f));
					auto point_2 = m3::screen_origin_to_projection_along_camera_plane_dstpx(
							center_position.offset_x(-width / 2.0f).offset_y(height / 2.0f));
					auto point_3 = m3::screen_origin_to_projection_along_camera_plane_dstpx(
							center_position.offset_x(width / 2.0f).offset_y(height / 2.0f));
					if (point_0 && point_1 && point_2 && point_3) {
						SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
						std::array<SDL_FPoint, 5> points = {
								SDL_FPoint{point_0->x, point_0->y},
								SDL_FPoint{point_1->x, point_1->y},
								SDL_FPoint{point_3->x, point_3->y},
								SDL_FPoint{point_2->x, point_2->y},
								SDL_FPoint{point_0->x, point_0->y}
						};
						SDL_RenderDrawLinesF(M2_GAME.renderer, points.data(), I(points.size()));
					}
				}
				break;
			}
			case b2Shape::Type::e_circle: {
				const auto* shape = dynamic_cast<const b2CircleShape*>(fixture->GetShape());
				auto center_offset_m = VecF{shape->m_p}; // Offset of shape from object's origin
				auto circumference = aabb.upperBound.x - aabb.lowerBound.x;
				auto radius = circumference / 2.0f;

				if (is_projection_type_parallel(M2_LEVEL.ProjectionType())) {
					const int r = iround(M2_GAME.Dimensions().OutputPixelsPerMeter() * radius);
					const auto srcRect = static_cast<SDL_Rect>(M2_GAME.ShapeCache().Create(std::make_shared<Circle>(r)));
					auto* texture = M2_GAME.ShapeCache().Texture();
					auto screenOriginToSpriteCenter = ScreenOriginToPositionVecPx(position + center_offset_m);
					auto dstRect = SDL_Rect{
							iround(screenOriginToSpriteCenter.x) - (srcRect.w / 2),
							iround(screenOriginToSpriteCenter.y) - (srcRect.h / 2),
							srcRect.w,
							srcRect.h};
					SDL_RenderCopy(M2_GAME.renderer, texture, &srcRect, &dstRect);
				} else {
					auto center_position_2d = position + center_offset_m;
					auto center_position = m3::VecF{center_position_2d};
					// Draw a diamond instead of circle
					auto horizontal_point_a = m3::screen_origin_to_projection_along_camera_plane_dstpx(
							center_position.offset_x(-radius));
					auto horizontal_point_b = m3::screen_origin_to_projection_along_camera_plane_dstpx(
							center_position.offset_x(radius));
					auto vertical_point_a = m3::screen_origin_to_projection_along_camera_plane_dstpx(
							center_position.offset_y(-radius));
					auto vertical_point_b = m3::screen_origin_to_projection_along_camera_plane_dstpx(
							center_position.offset_y(radius));
					if (horizontal_point_a && horizontal_point_b && vertical_point_a && vertical_point_b) {
						SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
						std::array<SDL_FPoint, 5> points = {
								SDL_FPoint{horizontal_point_a->x, horizontal_point_a->y},
								SDL_FPoint{vertical_point_a->x, vertical_point_a->y},
								SDL_FPoint{horizontal_point_b->x, horizontal_point_b->y},
								SDL_FPoint{vertical_point_b->x, vertical_point_b->y},
								SDL_FPoint{horizontal_point_a->x, horizontal_point_a->y}
						};
						SDL_RenderDrawLinesF(M2_GAME.renderer, points.data(), I(points.size()));
					}
				}
				break;
			}
			case b2Shape::Type::e_edge: {
				const auto* shape = dynamic_cast<const b2EdgeShape*>(fixture->GetShape());
				auto point1 = static_cast<VecF>(shape->m_vertex1);
				auto point2 = static_cast<VecF>(shape->m_vertex2);
				if (is_projection_type_parallel(M2_LEVEL.ProjectionType())) {
					auto point1OnScreen = ScreenOriginToPositionVecPx(position + point1);
					auto point2OnScreen = ScreenOriginToPositionVecPx(position + point2);
					SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
					SDL_RenderDrawLine(M2_GAME.renderer, iround(point1OnScreen.x), iround(point1OnScreen.y),
							iround(point2OnScreen.x), iround(point2OnScreen.y));
				}
				break;
			}
			default:
				throw M2_ERROR("Unsupported shape");
		}
	}
}

void m2::Physique::default_begin_contact_cb(b2Contact& b2_contact) {
	box2d::Contact contact{b2_contact};

	Id physique_id_a = b2_contact.GetFixtureA()->GetBody()->GetUserData().pointer;
	Id physique_id_b = b2_contact.GetFixtureB()->GetBody()->GetUserData().pointer;
	auto& phy_a = M2_LEVEL.physics[physique_id_a];
	auto& phy_b = M2_LEVEL.physics[physique_id_b];
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
	auto& phy_a = M2_LEVEL.physics[physique_id_a];
	auto& phy_b = M2_LEVEL.physics[physique_id_b];
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
