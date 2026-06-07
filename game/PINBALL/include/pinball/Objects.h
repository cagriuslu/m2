#pragma once
#include <Simulation.pb.h>
#include <m2/Object.h>
#include <m2/thirdparty/physics/ColliderCategory.h>
#include <cstdint>

int64_t AnimalAllocator(pinball::pb::Animal_Type type);
void AnimalDeallocator(int64_t id);
m2::void_expected LoadBall(m2::Object& obj, const m2::VecF& position);
void LoadEdge();
m2::void_expected LoadFlipper(m2::Object& obj, const m2::VecF& position, float orientation, bool rightFlipper);
m2::void_expected LoadPlayer(m2::Object& obj);
m2::void_expected LoadWall(m2::Object& obj, const m2::VecF& position);
m2::void_expected LoadPlatform(m2::Object& obj, const m2::VecF& position);

// Sensors

/// Load a generic sensor from all the fixture. If the fixture of the sensor contains a chain, ghost collisions could occur
m2::void_expected LoadGenericBallSensor(m2::Object& obj, const m2::VecF& position,
	const std::function<void(m2::Physique& sensor, m2::Physique& ball, const m2::box2d::Contact&)>& onCollisionWithBall,
	const std::function<void(m2::Physique& sensor, m2::Physique& ball)>& offCollisionWithBall,
	uint16_t belongsTo = m2::thirdparty::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT);

/// Returns a deferred action that switches the ball between the ground level and the elevated platform level by
/// adjusting its collision mask (and draw layer), without moving it to another physics world.
std::function<void()> CreateBallLayerSwitcher(m2::ObjectId ballId, bool toPlatform);
void BallLauncherSensorOnCollision(m2::Physique& sensor, m2::Physique& ball, const m2::box2d::Contact&);
void BallLauncherSensorOffCollision(m2::Physique& sensor, m2::Physique& ball);
void PlatformEntrySensorOnCollision(m2::Physique& sensor, m2::Physique& ball, const m2::box2d::Contact&);
void PlatformExitSensorOnCollision(m2::Physique& sensor, m2::Physique& ball, const m2::box2d::Contact&);
void LightSwitchSensorOnCollision(m2::Physique& sensor, m2::Physique& ball, const m2::box2d::Contact&);
void HeaterSwitchSensorOnCollision(m2::Physique& sensor, m2::Physique& ball, const m2::box2d::Contact&);
void WaterSprinklerSensorOnCollision(m2::Physique& sensor, m2::Physique& ball, const m2::box2d::Contact&);
m2::void_expected LoadBumperSensor(m2::Object& obj, const m2::VecF& position);
m2::void_expected LoadDropGate(m2::Object& obj, const m2::VecF& position, float orientation);
m2::void_expected LoadCircularBumperSensor(m2::Object& obj, const m2::VecF& position);
m2::void_expected LoadDoor(m2::Object& obj, const m2::VecF& position, float orientation);
