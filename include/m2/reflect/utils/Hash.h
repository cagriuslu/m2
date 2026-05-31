#pragma once
#include "../Detail.h"
#include <m2/reflect/Composite.h>
#include <m2/reflect/Container.h>
#include <m2/reflect/Primitive.h>

namespace m2::reflect::util {
	namespace detail {
		struct HashAccessor {
			int32_t hash;
			void operator()(const Path&);
			void operator()(const Path&, bool);
			void operator()(const Path&, uint8_t);
			void operator()(const Path&, int8_t);
			void operator()(const Path&, int16_t);
			void operator()(const Path&, int32_t);
			void operator()(const Path&, int64_t);
			void operator()(const Path&, float);
			void operator()(const Path&, double);
			void operator()(const Path&, ContainerType);
			void operator()(const Path&, CompositeType);
		};
		static_assert(Accessor<HashAccessor>);
	}

	int32_t Hash(const IsCompositeReflective auto& value, const int32_t initialHash) {
		detail::HashAccessor accessor{initialHash};
		Path path;
		path.reserve(16);
		value.ReflectComposite(accessor, path);
		return accessor.hash;
	}
}
