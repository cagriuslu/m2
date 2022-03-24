#ifndef M2_GRAPHIC_H
#define M2_GRAPHIC_H

#include "../Component.h"

namespace m2::component {
	struct Graphic : public Component {
		struct SDL_Texture *texture;
		SDL_Rect textureRect;
		Vec2f center_px;
		float angle;
		void (*draw)(Graphic&);

		Graphic() = default;
		explicit Graphic(uint64_t object_id);

		static void default_draw(Graphic& gfx);
		static void default_draw_healthbar(Graphic& gfx, float healthRatio);
		static int ycomparator_cb(uint64_t gfxIdA, uint64_t gfxIdB);
	};
}

#endif //M2_GRAPHIC_H
