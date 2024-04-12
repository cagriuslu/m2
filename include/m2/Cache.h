#pragma once

#include <tuple>
#include <unordered_map>

namespace m2 {
	template <typename Generator, typename T, typename HashFunc, typename ...Args>
	class Cache {
		std::unordered_map<std::tuple<Args...>, T, HashFunc> _storage;

	public:
		const T& operator()(Args... args) {
			if (auto it = _storage.find(std::make_tuple(args...)); it != _storage.end()) {
				return it->second;
			}
			auto [new_it, success] = _storage.emplace(std::make_tuple(args...), Generator{}(args...));
			return new_it->second;
		}
	};
}
