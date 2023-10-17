#pragma once

#include "SpatialObjectLoader.h"

namespace m2 {
	class DynamicGridLinesLoader : public SpatialObjectLoader {
		SDL_Color _color;
		int _step;

	public:
		inline explicit DynamicGridLinesLoader(SDL_Color color, int step = 1) : _color(color), _step(step) {}

	protected:
		ObjectId load_vertical(int x) override;
		ObjectId load_horizontal(int y) override;
		void unload(ObjectId id) override;
	};
}
