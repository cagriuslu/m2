#pragma once

#include "Detail.h"
#include <google/protobuf/repeated_ptr_field.h>
#include <vector>
#include <filesystem>

namespace m2::pb {
	template <typename ProtoItemT, typename LoadedItemT>
	class LUT {
		std::vector<LoadedItemT> _vector;

		explicit LUT(std::vector<LoadedItemT>&& vec) : _vector(std::move(vec)) {}

	public:
		LUT() = default;

		template <typename KeyT = decltype(std::declval<ProtoItemT>().type())>
		const LoadedItemT& operator[](KeyT key) const {
			auto index = pb::enum_index(key);
			return _vector[index];
		}

		template <typename EnvelopeT, typename... LoadedItemArgs>
		static LUT load(const std::filesystem::path& envelope_path, const ::google::protobuf::RepeatedPtrField<ProtoItemT>& (EnvelopeT::*list_accessor)() const, LoadedItemArgs... args) {
			auto envelope = pb::json_file_to_message<EnvelopeT>(envelope_path);
			if (!envelope) {
				throw M2ERROR(envelope.error());
			}

			using KeyT = decltype(std::declval<ProtoItemT>().type());
			std::vector<LoadedItemT> items(pb::enum_value_count<KeyT>());
			std::vector<bool> is_loaded(pb::enum_value_count<KeyT>());

			for (const auto& item : ((*envelope).*list_accessor)()) {
				auto index = pb::enum_index(item.type());
				// Check if the item is already loaded
				if (is_loaded[index]) {
					throw M2ERROR("Item has duplicate definition: " + pb::enum_name(item.type()));
				}
				// Load item
				items[index] = LoadedItemT{item, args...};
				is_loaded[index] = true;
			}

			// Check if every item is loaded
			for (int i = 0; i < pb::enum_value_count<KeyT>(); ++i) {
				if (!is_loaded[i]) {
					throw M2ERROR("Item is not defined: " + pb::enum_name<KeyT>(i));
				}
			}

			return LUT{std::move(items)};
		}
	};
}
