#pragma once
#include "../Detail.h"
#include <m2/PropagateConst.h>
#include <Reflective.pb.h>

namespace m2::reflect::util {
	namespace detail {
		struct StoreAccessor {
			PropagateConstRef<pb::Reflective> root;
			void operator()(const Path&);
			void operator()(const Path&, bool);
			void operator()(const Path&, uint8_t);
			void operator()(const Path&, int8_t);
			void operator()(const Path&, int16_t);
			void operator()(const Path&, int32_t);
			void operator()(const Path&, int64_t);
			void operator()(const Path&, float);
			void operator()(const Path&, double);
			void operator()(const Path&, m2::Exact);
			void operator()(const Path&, UserDefinedEnumValue);
			void operator()(const Path&, ContainerType);
			void operator()(const Path&, CompositeType);
		};
		static_assert(Accessor<StoreAccessor>);
	}

	void StoreToProto(const IsCompositeReflective auto& value, pb::Reflective& reflective) {
		detail::StoreAccessor accessor{reflective};
		Path path;
		path.reserve(16);
		value.ReflectComposite(accessor, path);
	}
}
