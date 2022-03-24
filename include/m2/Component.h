#ifndef COMPONENT_H
#define COMPONENT_H

#include <m2/Cfg.hh>
#include <m2/Vec2f.h>
#include <m2/Box2D.hh>
#include <b2_body.h>
#include <SDL.h>
#include <stdint.h>

struct Component {
	ID object_id;

	Component() = default;
	explicit Component(ID object_id);
};

struct Physique : public Component {
	b2Body* body;
	void (*onCollision)(Physique&, Physique&);

	Physique() = default;
	explicit Physique(ID object_id);
    // Copy not allowed
    Physique(const Physique& other) = delete;
    Physique& operator=(const Physique& other) = delete;
    // Move constructors
    Physique(Physique&& other) noexcept;
    Physique& operator=(Physique&& other) noexcept;
    // Destructor
	~Physique();

	static void contact_cb(b2Contact* contact);
};

struct Graphic : public Component {
	SDL_Texture *texture;
	SDL_Rect textureRect;
	m2::Vec2f center_px;
	float angle;
	void (*draw)(Graphic&);

	Graphic() = default;
	explicit Graphic(ID object_id);

	static void default_draw(Graphic& gfx);
	static void default_draw_healthbar(Graphic& gfx, float healthRatio);
	static int ycomparator_cb(ID gfxIdA, ID gfxIdB);
};

struct Light : public Component {
	float radius_m;
	void (*draw)(Light&);

	Light() = default;
	explicit Light(ID object_id);

	static void default_draw(Light& lig);
};

namespace m2 {
    struct Defense : public Component {
        Defense() = default;
        explicit Defense(ID object_id);
    };

    struct Offense : public Component {
        Offense() = default;
        explicit Offense(ID object_id);
    };
}

#endif
