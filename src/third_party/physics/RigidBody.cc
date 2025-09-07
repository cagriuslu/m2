#include <m2/third_party/physics/RigidBody.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <box2d/b2_body.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_chain_shape.h>
#include <box2d/b2_edge_shape.h>

using namespace m2::third_party::physics;

namespace {
	b2BodyType ToBox2dBodyType(const RigidBodyType rbt) {
		switch (rbt) {
			case RigidBodyType::STATIC:
				return b2_staticBody;
			case RigidBodyType::KINEMATIC:
				return b2_kinematicBody;
			case RigidBodyType::DYNAMIC:
				return b2_dynamicBody;
		}
		throw M2_ERROR("Unknown rigid body type");
	}
}

RectangleShape RectangleShape::FromSpriteRectangleFixture(const pb::Fixture_RectangleFixture& rf, const int ppm) {
	RectangleShape rs;
	rs.offset = VecF{rf.sprite_origin_to_fixture_center_vec_px()} / ToFloat(ppm);
	rs.dimensions = VecF{rf.dims_px()} / ToFloat(ppm);
	rs.angleInRads = rf.angle_rads();
	return rs;
}
CircleShape CircleShape::FromSpriteCircleFixture(const pb::Fixture_CircleFixture& cf, const int ppm) {
	CircleShape cs;
	cs.offset = VecF{cf.sprite_origin_to_fixture_center_vec_px()} / ToFloat(ppm);
	cs.radius = cf.radius_px() / ToFloat(ppm);
	return cs;
}
ChainShape ChainShape::FromSpriteChainFixture(const pb::Fixture_ChainFixture& cf, int ppm) {
	ChainShape cs{.points = std::vector<VecF>(cf.points_size())};
	std::ranges::transform(cf.points(), cs.points.begin(), [ppm](const auto& point) {
		return VecF{point} / ToFloat(ppm);
	});
	return cs;
}
std::variant<PolygonShape,RectangleShape,CircleShape,ChainShape,EdgeShape> m2::third_party::physics::ToShape(const pb::Fixture& pbFixture, const int ppm) {
	if (pbFixture.has_rectangle()) {
		return RectangleShape::FromSpriteRectangleFixture(pbFixture.rectangle(), ppm);
	}
	if (pbFixture.has_circle()) {
		return CircleShape::FromSpriteCircleFixture(pbFixture.circle(), ppm);
	}
	if (pbFixture.has_chain()) {
		return ChainShape::FromSpriteChainFixture(pbFixture.chain(), ppm);
	}
	throw M2_ERROR("Unknown shape");
}

RigidBody RigidBody::CreateFromDefinition(const RigidBodyDefinition& definition, PhysiqueId physiqueId,
		const VecF& position, const float angleInRads, const pb::PhysicsLayer pl) {
	b2BodyDef box2dBodyDef = {};
	box2dBodyDef.type = ToBox2dBodyType(definition.bodyType);
	box2dBodyDef.position = static_cast<b2Vec2>(position);
	box2dBodyDef.angle = angleInRads;
	box2dBodyDef.linearVelocity = {};
	box2dBodyDef.angularVelocity = {};
	box2dBodyDef.linearDamping = definition.linearDamping;
	box2dBodyDef.angularDamping = definition.angularDamping;
	box2dBodyDef.allowSleep = definition.allowSleeping;
	box2dBodyDef.awake = definition.initiallyAwake;
	box2dBodyDef.fixedRotation = definition.fixedRotation;
	box2dBodyDef.bullet = definition.isBullet;
	box2dBodyDef.enabled = definition.initiallyEnabled;
	box2dBodyDef.userData.pointer = physiqueId;
	box2dBodyDef.gravityScale = definition.gravityScale;
	b2Body* body = M2_LEVEL.world[I(pl)]->CreateBody(&box2dBodyDef);

	for (const auto& fixture : definition.fixtures) {
		b2FixtureDef box2dFixtureDef = {};
		std::variant<std::monostate,b2PolygonShape,b2CircleShape,b2ChainShape,b2EdgeShape> shape;
		if (std::holds_alternative<PolygonShape>(fixture.shape)) {
			const auto& poly = std::get<PolygonShape>(fixture.shape);
			shape.emplace<b2PolygonShape>();
			std::vector<b2Vec2> points;
			for (const auto& point : poly.points) {
				points.emplace_back(static_cast<b2Vec2>(point));
			}
			std::get<b2PolygonShape>(shape).Set(points.data(), I(poly.points.size()));
			box2dFixtureDef.shape = &std::get<b2PolygonShape>(shape);
		} else if (std::holds_alternative<RectangleShape>(fixture.shape)) {
			const auto& rect = std::get<RectangleShape>(fixture.shape);
			shape.emplace<b2PolygonShape>();
			std::get<b2PolygonShape>(shape).SetAsBox(rect.dimensions.x / 2.0f, rect.dimensions.y / 2.0f,
					static_cast<b2Vec2>(rect.offset), rect.angleInRads);
			box2dFixtureDef.shape = &std::get<b2PolygonShape>(shape);
		} else if (std::holds_alternative<CircleShape>(fixture.shape)) {
			const auto& circ = std::get<CircleShape>(fixture.shape);
			shape.emplace<b2CircleShape>();
			std::get<b2CircleShape>(shape).m_radius = circ.radius;
			std::get<b2CircleShape>(shape).m_p = static_cast<b2Vec2>(circ.offset);
			box2dFixtureDef.shape = &std::get<b2CircleShape>(shape);
		} else if (std::holds_alternative<ChainShape>(fixture.shape)) {
			const auto& chain = std::get<ChainShape>(fixture.shape);
			shape.emplace<b2ChainShape>();
			if (chain.points.size() < 3) {
				throw M2_ERROR("Chain shape must have at least 3 points");
			}
			// Convert to Box2d vertices
			std::vector<b2Vec2> vertices(chain.points.size());
			std::ranges::transform(chain.points, vertices.begin(), [&](const auto& point) {
				return static_cast<b2Vec2>(point);
			});
			// Create loop
			std::get<b2ChainShape>(shape).CreateLoop(vertices.data(), I(vertices.size()));
			box2dFixtureDef.shape = &std::get<b2ChainShape>(shape);
		} else if (std::holds_alternative<EdgeShape>(fixture.shape)) {
			const auto& edge = std::get<EdgeShape>(fixture.shape);
			shape.emplace<b2EdgeShape>();
			std::get<b2EdgeShape>(shape).SetTwoSided(static_cast<b2Vec2>(edge.pointA), static_cast<b2Vec2>(edge.pointB));
			box2dFixtureDef.shape = &std::get<b2EdgeShape>(shape);
		} else {
			throw M2_ERROR("Unknown shape");
		}
		box2dFixtureDef.userData = {};
		box2dFixtureDef.friction = fixture.friction;
		box2dFixtureDef.restitution = fixture.restitution;
		box2dFixtureDef.restitutionThreshold = fixture.restitutionThresholdVelocity;
		box2dFixtureDef.density = 1.0f; // Value not important, mass will be overridden
		box2dFixtureDef.isSensor = fixture.isSensor;
		box2dFixtureDef.filter.categoryBits = fixture.colliderFilter.belongsTo;
		box2dFixtureDef.filter.maskBits = fixture.colliderFilter.collidesWith;
		box2dFixtureDef.filter.groupIndex = {};
		body->CreateFixture(&box2dFixtureDef);
	}

	if (definition.bodyType == RigidBodyType::DYNAMIC) {
		b2MassData massData{};
		massData.mass = definition.mass;
		massData.center = static_cast<b2Vec2>(definition.offsetOfCenterOfMass);
		massData.I = definition.inertia;
		body->SetMassData(&massData);
	}

	return RigidBody{body, pl};
}
RigidBody::RigidBody(RigidBody&& other) noexcept : _ptr(other._ptr), _phyLayer(other._phyLayer) {
	other._ptr = nullptr;
}
RigidBody& RigidBody::operator=(RigidBody&& other) noexcept {
	std::swap(_ptr, other._ptr);
	std::swap(_phyLayer, other._phyLayer);
	return *this;
}
RigidBody::~RigidBody() {
	if (_ptr) {
		if (M2_LEVEL.world[I(_phyLayer)]->IsLocked()) {
			LOG_FATAL("Body destroyed during physics step");
			std::terminate();
		}
		M2_LEVEL.world[I(_phyLayer)]->DestroyBody(static_cast<b2Body*>(_ptr));
	}
}

bool RigidBody::IsEnabled() const {
	return static_cast<b2Body*>(_ptr)->IsEnabled();
}
m2::VecF RigidBody::GetPosition() const {
	return VecF{static_cast<b2Body*>(_ptr)->GetPosition()};
}
float RigidBody::GetAngle() const {
	return static_cast<b2Body*>(_ptr)->GetAngle();
}
m2::VecF RigidBody::GetLinearVelocity() const {
	return VecF{static_cast<b2Body*>(_ptr)->GetLinearVelocity()};
}
float RigidBody::GetAngularVelocity() const {
	return static_cast<b2Body*>(_ptr)->GetAngularVelocity();
}
bool RigidBody::HasJoint() const {
	return static_cast<b2Body*>(_ptr)->GetJointList();
}

uint16_t RigidBody::GetAllLayersBelongingTo() const {
	uint16_t out{};
	const auto* body = static_cast<b2Body*>(_ptr);
	for (const auto* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
		out |= fixture->GetFilterData().categoryBits;
	}
	return out;
}
uint16_t RigidBody::GetAllLayersCollidingTo() const {
	uint16_t out{};
	const auto* body = static_cast<b2Body*>(_ptr);
	for (const auto* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
		out |= fixture->GetFilterData().maskBits;
	}
	return out;
}

void RigidBody::SetEnabled(const bool state) {
	static_cast<b2Body*>(_ptr)->SetEnabled(state);
}
void RigidBody::SetPosition(const VecF& pos) {
	const auto currentAngle = static_cast<b2Body*>(_ptr)->GetAngle();
	static_cast<b2Body*>(_ptr)->SetTransform(static_cast<b2Vec2>(pos), currentAngle);
}
void RigidBody::SetAngle(const float angle) {
	const auto& currentPosition = static_cast<b2Body*>(_ptr)->GetPosition();
	static_cast<b2Body*>(_ptr)->SetTransform(currentPosition, angle);
}
void RigidBody::SetLinearVelocity(const VecF& vel) {
	static_cast<b2Body*>(_ptr)->SetLinearVelocity(static_cast<b2Vec2>(vel));
}
void RigidBody::SetAngularVelocity(const float w) {
	static_cast<b2Body*>(_ptr)->SetAngularVelocity(w);
}
void RigidBody::ApplyForceToCenter(const VecF& f) {
	static_cast<b2Body*>(_ptr)->ApplyForceToCenter(static_cast<b2Vec2>(f), true);
}

void RigidBody::TeleportToAnother(const RigidBody& other) {
	auto* thisBody = static_cast<b2Body*>(_ptr);
	const auto* otherBody = static_cast<b2Body*>(other._ptr);
	thisBody->SetTransform(otherBody->GetPosition(), otherBody->GetAngle());
	thisBody->SetLinearVelocity(otherBody->GetLinearVelocity());
	thisBody->SetAngularVelocity(otherBody->GetAngularVelocity());
	thisBody->SetAwake(otherBody->IsAwake());
}
