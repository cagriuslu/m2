#ifndef M2_SHAPE_H
#define M2_SHAPE_H

#include "DynamicSheet.h"

namespace m2 {
	/// Sheet that generates and returns shapes on demand
	class ShapesSheet : private DynamicSheet {
		enum class ShapeType {
			NONE = 0,
			LINE,
			RECTANGLE_AA,
			CIRCLE,
		};
		struct ShapeKey {
			ShapeType type{};
			int x1{}, y1{};
			int w{}, h{};
			int r{};

			bool operator==(const ShapeKey& other) const;
		};
		struct ShapeKeyHash {
			size_t operator()(const ShapeKey& k) const;
		};

		std::unordered_map<ShapeKey, SDL_Rect, ShapeKeyHash> _shapes;

	public:
		explicit ShapesSheet(SDL_Renderer* renderer);
		std::pair<SDL_Texture*, SDL_Rect> get_line(SDL_Color color, int x1, int y1);
		std::pair<SDL_Texture*, SDL_Rect> get_rectangle_aa(SDL_Color color, int w, int h); // Axis-aligned
		std::pair<SDL_Texture*, SDL_Rect> get_circle(SDL_Color color, int radius);
	};
}

#endif //M2_SHAPE_H
