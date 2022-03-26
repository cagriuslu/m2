#include <m2/box2d/Utils.h>
#include <m2/Box2D.hh>
#include <b2_circle_shape.h>
#include <b2_fixture.h>
#include <b2_polygon_shape.h>
#include <b2_world.h>

b2Body* m2::box2d::create_body(
        b2World& world,
        ID physique_id,
        bool is_disk,
        bool is_dynamic,
        m2::Vec2f position,
        bool allow_sleep,
        bool is_bullet,
        bool is_sensor,
        uint16_t category_bits,
        uint16_t mask_bits,
        m2::Vec2f box_dims,
        m2::Vec2f box_center_offset,
        float box_angle,
        float disk_radius,
        float mass,
        float linearDamping,
        bool fixed_rotation) {
    b2BodyDef bodyDef;
    if (is_dynamic) {
        bodyDef.type = b2_dynamicBody;
    }
    bodyDef.position.Set(position.x, position.y);
    bodyDef.allowSleep = allow_sleep;
    bodyDef.userData.pointer = physique_id;
    bodyDef.bullet = is_bullet;
    b2Body* body = world.CreateBody(&bodyDef);

    b2FixtureDef fixtureDef;
    b2CircleShape circle_shape;
    b2PolygonShape polygon_shape;
    if (is_disk) {
        circle_shape.m_radius = disk_radius;
        fixtureDef.shape = &circle_shape;
    } else {
        m2::Vec2f halfDims = box_dims * 0.5f;
        polygon_shape.SetAsBox(halfDims.x, halfDims.y, static_cast<b2Vec2>(box_center_offset), box_angle);
        fixtureDef.shape = &polygon_shape;
    }
    fixtureDef.friction = 0.05f;
    fixtureDef.filter.categoryBits = category_bits;
    // If mask is not provided, infer it
    if (mask_bits == 0) {
        using namespace m2::box2d;
        switch (category_bits) {
            case CATEGORY_STATIC_OBJECT:
                mask_bits = CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF | CATEGORY_PLAYER | CATEGORY_PLAYER_BULLET | CATEGORY_ENEMY | CATEGORY_ENEMY_BULLET;
                break;
            case CATEGORY_STATIC_CLIFF:
                mask_bits = CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF | CATEGORY_PLAYER | CATEGORY_ENEMY;
                break;
            case CATEGORY_PLAYER:
                mask_bits = CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF | CATEGORY_PLAYER | CATEGORY_ENEMY | CATEGORY_ENEMY_BULLET | CATEGORY_ENEMY_MELEE_WEAPON;
                break;
            case CATEGORY_PLAYER_BULLET:
                mask_bits = CATEGORY_STATIC_OBJECT | CATEGORY_ENEMY;
                break;
            case CATEGORY_PLAYER_MELEE_WEAPON:
                mask_bits = CATEGORY_ENEMY;
                break;
            case CATEGORY_ENEMY:
                mask_bits = CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF | CATEGORY_PLAYER | CATEGORY_PLAYER_BULLET | CATEGORY_PLAYER_MELEE_WEAPON | CATEGORY_ENEMY;
                break;
            case CATEGORY_ENEMY_BULLET:
                mask_bits = CATEGORY_STATIC_OBJECT | CATEGORY_PLAYER;
                break;
            case CATEGORY_ENEMY_MELEE_WEAPON:
                mask_bits = CATEGORY_PLAYER;
                break;
            default:
                break;
        }
    }
    fixtureDef.filter.maskBits = mask_bits;
    b2Fixture* fixture = body->CreateFixture(&fixtureDef);
    fixture->SetSensor(is_sensor);

    if (is_dynamic) {
        body->SetLinearDamping(linearDamping);
        body->SetAngularDamping(0.0f);
        body->SetFixedRotation(fixed_rotation);
        b2MassData massData = { mass, b2Vec2{0.0f, 0.0f}, 0.0f };
        body->SetMassData(&massData);
    }

    return body;
}

b2Body* m2::box2d::create_static_box(b2World& world, ID physique_id, m2::Vec2f position, bool is_sensor, uint16_t category, m2::Vec2f box_dims) {
    return create_body(world, physique_id, false, false, position, true, false, is_sensor, category, 0, box_dims, {}, 0.0f, 0.0f, 0.0f, 0.0f, true);
}

b2Body* m2::box2d::create_static_disk(b2World& world, ID physique_id, m2::Vec2f position, bool is_sensor, uint16_t category, float disk_radius) {
    return create_body(world, physique_id, true, false, position, true, false, is_sensor, category, 0, {}, {}, 0.0f, disk_radius, 0.0f, 0.0f, true);
}

b2Body* m2::box2d::create_dynamic_box(b2World& world, ID physique_id, m2::Vec2f position, bool is_sensor, bool allow_sleep, uint16_t category_bits, m2::Vec2f box_dims, float mass, float linear_damping) {
    return create_body(world, physique_id, false, true, position, allow_sleep, false, is_sensor, category_bits, 0, box_dims, {}, 0.0f, 0.0f, mass, linear_damping, true);
}

b2Body* m2::box2d::create_dynamic_disk(b2World& world, ID physique_id, m2::Vec2f position, bool is_sensor, bool allow_sleep, uint16_t category_bits, float disk_radius, float mass, float linear_damping) {
    return create_body(world, physique_id, true, true, position, allow_sleep, false, is_sensor, category_bits, 0, {}, {}, 0.0f, disk_radius, mass, linear_damping, true);
}

b2Body* m2::box2d::create_bullet(b2World& world, ID physique_id, m2::Vec2f position, bool is_sensor, uint16_t category_bits, float disk_radius, float mass, float linear_damping) {
    return create_body(world, physique_id, true, true, position, false, true, is_sensor, category_bits, 0, {}, {}, 0.0f, disk_radius, mass, linear_damping, true);
}

bool m2::box2d::check_eye_sight(b2World& world, m2::Vec2f from, m2::Vec2f to, uint16_t category_bits) {
    if (from == to) {
        return true;
    } else {
        bool result = true;
        RayCastCallback rccb([&](b2Fixture* fixture, m2::Vec2f point, m2::Vec2f normal, float fraction) -> float {
            result = false;
            return 0.0f;
            }, category_bits);
        world.RayCast(&rccb, static_cast<b2Vec2>(from), static_cast<b2Vec2>(to));
        return result;
    }
}
