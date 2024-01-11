#pragma once

#include "../Object.h"
#include "../RectF.h"
#include <unordered_map>
#include <optional>
#include <deque>

namespace m2 {
	class SpatialObjectLoader {
		RectI _prev_viewport;
		std::unordered_map<VecI, ObjectId, VecIHash> _loaded_objects;
		std::map<int, ObjectId> _loaded_verticals, _loaded_horizontals;

	public:
		void move(const RectF& viewport);
		virtual ~SpatialObjectLoader() = default;

	protected:
		/// Load can return 0, which means that position does not have an object.
		/// The position is marked as loaded nevertheless.
		virtual ObjectId load(const VecI& position);
		virtual ObjectId load_vertical(int x);
		virtual ObjectId load_horizontal(int y);

		virtual void unload(ObjectId id) = 0;
	};
}
