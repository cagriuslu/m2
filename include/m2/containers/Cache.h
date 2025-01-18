#pragma once

#include <tuple>
#include <unordered_map>

namespace m2 {
	template <typename Key, typename Value, typename ValueGenerator, typename KeyHashFunc = std::hash<Key>, typename KeyComparator = std::equal_to<Key>>
	class Cache {
		ValueGenerator _generator;
		std::unordered_map<Key, Value, KeyHashFunc, KeyComparator> _storage;

	public:
		explicit Cache(ValueGenerator&& generator = {}) : _generator(std::move(generator)) {}

		// Accessors

		const ValueGenerator& Generator() const { return _generator; }

		// Modifiers

		Value& operator()(const Key& args) {
			// Search if the entry exists in the storage
			if (auto it = _storage.find(args); it != _storage.end()) {
				return it->second;
			}
			// Otherwise, create the object
			auto [new_it, success] = _storage.emplace(args, _generator(args));
			return new_it->second;
		}
	};
}
