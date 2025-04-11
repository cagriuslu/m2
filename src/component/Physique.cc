#include "m2/component/Physique.h"
#include <m2/Game.h>
#include <box2d/b2_contact.h>
#include <m2/Object.h>
#include <m2/box2d/Shape.h>
#include <m2/third_party/physics/ColliderCategory.h>

m2::Physique::Physique(Id object_id) : Component(object_id) {}

m2::Physique::Physique(Physique&& other) noexcept
		: Component(other._owner_id), preStep(std::move(other.preStep)), postStep(std::move(other.postStep)),
		body(std::move(other.body)), rigidBodyIndex(std::move(other.rigidBodyIndex)),
		onCollision(std::move(other.onCollision)), offCollision(std::move(other.offCollision)) {
    other.body.reset();
	other.rigidBodyIndex.reset();
}

m2::Physique& m2::Physique::operator=(Physique&& other) noexcept {
    std::swap(_owner_id, other._owner_id);
    std::swap(preStep, other.preStep);
    std::swap(postStep, other.postStep);
    std::swap(body, other.body);
    std::swap(rigidBodyIndex, other.rigidBodyIndex);
	std::swap(onCollision, other.onCollision);
	std::swap(offCollision, other.offCollision);
    return *this;
}

void m2::Physique::DefaultDebugDraw(Physique& phy) {
	if (not phy.body) {
		return;
	}
	auto* body = static_cast<b2Body*>(phy.body->GetThirdPartObject());
	auto objectPosition = VecF{body->GetPosition()};
	auto objectOrientation = body->GetAngle();
	for (auto* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
		b2AABB aabb;
		fixture->GetShape()->ComputeAABB(&aabb, body->GetTransform(), 0);

		// Pick different color for background and foreground colliders
		auto color = third_party::physics::DoesBelongToBackground(fixture->GetFilterData().categoryBits) ?
			SDL_Color{255, 0, 0, 255} :
			SDL_Color{255, 255, 0, 255};

		switch (fixture->GetType()) {
			case b2Shape::Type::e_polygon: {
				const auto* shape = dynamic_cast<const b2PolygonShape*>(fixture->GetShape());
				const auto objectOriginToPolygonCenter = VecF{shape->m_centroid};
				// Polygons that are rotated wrt the body are not supported
				const auto polygonCenter = objectPosition + objectOriginToPolygonCenter.rotate(objectOrientation);
				const auto polygonWidth = VecF{shape->m_vertices[0]}.distance(VecF{shape->m_vertices[1]});
				const auto polygonHeight = VecF{shape->m_vertices[1]}.distance(VecF{shape->m_vertices[2]});

				if (IsProjectionTypeParallel(M2_LEVEL.ProjectionType())) {
					Graphic::DrawRectangle(polygonCenter, polygonWidth, polygonHeight, objectOrientation, RGBA{color});
				} else {
					// Decompose the "object origin" to "shape centroid" vector
					// (shape doesn't know where the object origin is, or the angle of the object)
					auto centroid_offset_m = VecF{shape->m_centroid};
					auto centroid_offset_length_m = centroid_offset_m.length();
					auto centroid_offset_angle = centroid_offset_m.angle_rads();
					// Current angle of the object in the world
					auto current_angle = body->GetTransform().q.GetAngle();
					// Compose the "object origin" to "current shape centroid" vector
					auto center_offset_m = VecF::from_angle(centroid_offset_angle + current_angle).with_length(centroid_offset_length_m);
					auto center_position_2d = objectPosition + center_offset_m;
					auto center_position = m3::VecF{center_position_2d};
					// Draw a rectangle
					auto point_0 = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(
							center_position.offset_x(-polygonWidth / 2.0f).offset_y(-polygonHeight / 2.0f));
					auto point_1 = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(
							center_position.offset_x(polygonWidth / 2.0f).offset_y(-polygonHeight / 2.0f));
					auto point_2 = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(
							center_position.offset_x(-polygonWidth / 2.0f).offset_y(polygonHeight / 2.0f));
					auto point_3 = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(
							center_position.offset_x(polygonWidth / 2.0f).offset_y(polygonHeight / 2.0f));
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
				const auto objectOriginToCircleCenter = VecF{shape->m_p};
				const auto circleCenter = objectPosition + objectOriginToCircleCenter.rotate(objectOrientation);
				const auto circleRadius = shape->m_radius;

				if (IsProjectionTypeParallel(M2_LEVEL.ProjectionType())) {
					const int r = RoundI(M2_GAME.Dimensions().OutputPixelsPerMeter() * circleRadius);
					const auto srcRect = static_cast<SDL_Rect>(M2_GAME.GetShapeCache().Create(std::make_shared<Circle>(r)));
					auto* texture = M2_GAME.GetShapeCache().Texture();
					auto screenOriginToSpriteCenter = ScreenOriginToPositionVecPx(circleCenter);
					auto dstRect = SDL_Rect{
							RoundI(screenOriginToSpriteCenter.x) - (srcRect.w / 2),
							RoundI(screenOriginToSpriteCenter.y) - (srcRect.h / 2),
							srcRect.w,
							srcRect.h};
					SDL_RenderCopy(M2_GAME.renderer, texture, &srcRect, &dstRect);
				} else {
					auto center_position = m3::VecF{circleCenter};
					// Draw a diamond instead of circle
					auto horizontal_point_a = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(
							center_position.offset_x(-circleRadius));
					auto horizontal_point_b = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(
							center_position.offset_x(circleRadius));
					auto vertical_point_a = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(
							center_position.offset_y(-circleRadius));
					auto vertical_point_b = m3::ScreenOriginToProjectionAlongCameraPlaneDstpx(
							center_position.offset_y(circleRadius));
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
			case b2Shape::Type::e_chain: {
				if (IsProjectionTypeParallel(M2_LEVEL.ProjectionType())) {
					if (const auto* shape = dynamic_cast<const b2ChainShape*>(fixture->GetShape()); 3 < shape->m_count) {
						for (int i = 1; i < shape->m_count; ++i) {
							const auto point1 = objectPosition + VecF{shape->m_vertices[i-1]}.rotate(objectOrientation);
							const auto point2 = objectPosition + VecF{shape->m_vertices[i]}.rotate(objectOrientation);
							Graphic::DrawLine(point1, point2, color);
						}
					}
				}
				break;
			}
			case b2Shape::Type::e_edge: {
				const auto* shape = dynamic_cast<const b2EdgeShape*>(fixture->GetShape());
				auto point1 = static_cast<VecF>(shape->m_vertex1);
				auto point2 = static_cast<VecF>(shape->m_vertex2);
				if (IsProjectionTypeParallel(M2_LEVEL.ProjectionType())) {
					auto point1OnScreen = ScreenOriginToPositionVecPx(objectPosition + point1);
					auto point2OnScreen = ScreenOriginToPositionVecPx(objectPosition + point2);
					SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
					SDL_RenderDrawLine(M2_GAME.renderer, RoundI(point1OnScreen.x), RoundI(point1OnScreen.y),
							RoundI(point2OnScreen.x), RoundI(point2OnScreen.y));
				}
				break;
			}
			default:
				throw M2_ERROR("Unsupported shape");
		}
	}
}

void m2::Physique::DefaultBeginContactCallback(b2Contact& b2_contact) {
	box2d::Contact contact{b2_contact};

	Id physique_id_a = b2_contact.GetFixtureA()->GetBody()->GetUserData().pointer;
	Id physique_id_b = b2_contact.GetFixtureB()->GetBody()->GetUserData().pointer;
	auto& phy_a = M2_LEVEL.physics[physique_id_a];
	auto& phy_b = M2_LEVEL.physics[physique_id_b];
	if (phy_a.onCollision) {
		phy_a.onCollision(phy_a, phy_b, contact);
	}
	if (phy_b.onCollision) {
		phy_b.onCollision(phy_b, phy_a, contact);
	}
}
void m2::Physique::DefaultEndContactCallback(b2Contact& b2_contact) {
	Id physique_id_a = b2_contact.GetFixtureA()->GetBody()->GetUserData().pointer;
	Id physique_id_b = b2_contact.GetFixtureB()->GetBody()->GetUserData().pointer;
	auto& phy_a = M2_LEVEL.physics[physique_id_a];
	auto& phy_b = M2_LEVEL.physics[physique_id_b];
	if (phy_a.offCollision) {
		phy_a.offCollision(phy_a, phy_b);
	}
	if (phy_b.offCollision) {
		phy_b.offCollision(phy_b, phy_a);
	}
}

float m2::CalculateLimitedForce(float curr_speed, float speed_limit) {
	return logf(abs(speed_limit) - abs(curr_speed) + 1.0f); // ln(1) = 0
}
