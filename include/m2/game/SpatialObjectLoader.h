#ifndef M2_SPATIALOBJECTLOADER_H
#define M2_SPATIALOBJECTLOADER_H

#include "../Object.h"
#include "../RectF.h"
#include <unordered_map>
#include <optional>

namespace m2 {
	class SpatialObjectLoader {
		std::unordered_map<VecI, ObjectId, VecIHash> _loaded_objs;

	public:
		void move(const RectF& viewport);
		virtual ~SpatialObjectLoader() = default;

	protected:
		virtual ObjectId load(const VecI& position) = 0;
		virtual void unload(ObjectId id) = 0;
	};
}

#endif //M2_SPATIALOBJECTLOADER_H
