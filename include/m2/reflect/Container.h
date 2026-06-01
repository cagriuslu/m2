#pragma once
#include "Detail.h"
#include <deque>
#include <vector>
#include <list>

namespace m2::reflect {
	template <template <typename> class Container, IsReflective T>
	class SequenceContainer {
		Container<T> _storage;

	public:
		static constexpr auto Type = ContainerType::Sequence;

		SequenceContainer() = default;
		const auto& Get() const { return _storage; }
		auto& Mutate() { return _storage; }

		template <typename Accessor>
		void ReflectContainer(Accessor& accessor, Path& path) const {
			accessor(path, ContainerType::Sequence);
			int i = 0;
			for (auto it = _storage.cbegin(); it != _storage.cend(); ++it) {
				path.emplace_back(ContainerType::Sequence, i++);
				if constexpr (IsPrimitiveReflective<T>) {
					it->ReflectPrimitive(accessor, path);
				} else if constexpr (IsContainerReflective<T>) {
					it->ReflectContainer(accessor, path);
				}  else {
					it->ReflectComposite(accessor, path);
				}
				path.pop_back();
			}
		}
	};

	template <IsReflective T>
	using Deque = SequenceContainer<std::deque, T>;

	template <IsReflective T>
	using List = SequenceContainer<std::list, T>;

	template <IsReflective T>
	using Vector = SequenceContainer<std::vector, T>;
}
