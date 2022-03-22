#ifndef COMPONENT_H
#define COMPONENT_H

#include <m2/Cfg.hh>
#include <m2/Vec2F.h>
#include <m2/Box2D.hh>
#include <b2_body.h>
#include <SDL.h>
#include <stdint.h>

struct Component {
	ID object_id;

	Component() = default;
	explicit Component(ID object_id);
};

struct Monitor : public Component {
	void (*prePhysics)(Monitor&);
	void (*postPhysics)(Monitor&);
	void (*preGraphics)(Monitor&);
	void (*postGraphics)(Monitor&);

	Monitor() = default;
	explicit Monitor(ID object_id);
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
	m2::vec2f center_px;
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
    struct component_defense : public Component {
        component_defense() = default;
        explicit component_defense(ID object_id);
    };

    struct component_offense : public Component {
        component_offense() = default;
        explicit component_offense(ID object_id);
    };
}

#endif
