#pragma once
#include "DynamicTexture.h"

namespace m2 {
	/// Sheet that generates and returns shapes on demand
	class ShapesSheet : private DynamicTexture {
		enum ShapeType : uint16_t {
			NONE = 0,
			CIRCLE,
		};
		struct ShapeKey {
			ShapeType type{};
			SDL_Color color{};
			uint16_t w{}, h{};
			uint16_t piece_count{};

			bool operator==(const ShapeKey& other) const;
		};
		struct ShapeKeyHash {
			size_t operator()(const ShapeKey& k) const;
		};

		std::unordered_map<ShapeKey, SDL_Rect, ShapeKeyHash> _shapes;

	public:
		explicit ShapesSheet(SDL_Renderer* renderer);
		std::pair<SDL_Texture*, SDL_Rect> get_circle(SDL_Color color, uint16_t w, uint16_t h, unsigned piece_count);
	};
}
