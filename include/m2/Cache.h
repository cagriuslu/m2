#pragma once

#include <tuple>
#include <unordered_map>

namespace m2 {
	template <typename T, typename TGenerator, typename HashFunc, typename ...Args>
	class Cache {
		std::unordered_map<std::tuple<Args...>, T, HashFunc> _storage;

	public:
		T& operator()(Args... args) {
			// Search if the entry exists in the storage
			if (auto it = _storage.find(std::make_tuple(args...)); it != _storage.end()) {
				return it->second;
			}
			// Otherwise, create the object
			auto [new_it, success] = _storage.emplace(std::make_tuple(args...), TGenerator{}(args...));
			return new_it->second;
		}
	};
}
