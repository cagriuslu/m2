#pragma once

#include "Detail.h"
#include <google/protobuf/repeated_ptr_field.h>
#include <vector>
#include <filesystem>

namespace m2::pb {
	template <typename ProtoItemT, typename LoadedItemT = int>
	class MessageLUT {
		std::vector<LoadedItemT> _vector;

		explicit MessageLUT(std::vector<LoadedItemT>&& vec) : _vector(std::move(vec)) {}

	public:
		MessageLUT() = default;

		[[nodiscard]] std::vector<LoadedItemT>::size_type size() const { return _vector.size(); }

		template <typename KeyT = decltype(std::declval<ProtoItemT>().type())>
		const LoadedItemT& operator[](KeyT key) const {
			auto index = pb::enum_index(key);
			return _vector[index];
		}

		template <typename EnvelopeT>
		static expected<std::vector<ProtoItemT>> LoadProtoItems(
				const EnvelopeT* inMemoryEnvelope, const std::filesystem::path& overrideEnvelopePath,
				const ::google::protobuf::RepeatedPtrField<ProtoItemT>& (EnvelopeT::*list_accessor)() const) {
			const EnvelopeT* envelopeToUse{};

			// Try to load the envelope from the file, as it overrides the in-memory one
			auto envelopeFromFile = pb::json_file_to_message<EnvelopeT>(overrideEnvelopePath);
			if (envelopeFromFile) {
				envelopeToUse = &*envelopeFromFile;
			} else {
				if (inMemoryEnvelope) {
					envelopeToUse = inMemoryEnvelope;
				} else {
					m2ReflectUnexpected(envelopeFromFile);
				}
			}

			using KeyT = decltype(std::declval<ProtoItemT>().type());
			std::vector<ProtoItemT> protoItems(pb::enum_value_count<KeyT>());
			std::vector<bool> isLoaded(pb::enum_value_count<KeyT>());

			for (const auto& protoItem : ((*envelopeToUse).*list_accessor)()) {
				const auto key = protoItem.type(); // Enum value
				const auto keyIndex = pb::enum_index(key); // Index of the enum value

				// Check if the item is already loaded
				if (isLoaded[keyIndex]) {
					return make_unexpected("Item has duplicate definition: " + pb::enum_name(key));
				}

				// Save proto item
				protoItems[keyIndex] = protoItem;
				isLoaded[keyIndex] = true;
			}

			// Check if every item is loaded
			for (int i = 0; i < pb::enum_value_count<KeyT>(); ++i) {
				if (not isLoaded[i]) {
					return make_unexpected("Item is not defined: " + pb::enum_name<KeyT>(pb::enum_value<KeyT>(i)));
				}
			}

			return protoItems;
		}

		template <typename EnvelopeT, typename... LoadedItemArgs>
		static MessageLUT load(const EnvelopeT* inMemoryEnvelope, const std::filesystem::path& overrideEnvelopePath,
				const ::google::protobuf::RepeatedPtrField<ProtoItemT>& (EnvelopeT::*listAccessor)() const,
				LoadedItemArgs... args) {
			const auto protoItems = LoadProtoItems(inMemoryEnvelope, overrideEnvelopePath, listAccessor);
			if (not protoItems) {
				throw M2_ERROR(protoItems.error());
			}

			// Transform proto items into loaded items
			std::vector<LoadedItemT> items(protoItems->size());
			for (const auto& protoItem : *protoItems) {
				const auto key = protoItem.type(); // Enum value
				const auto keyIndex = pb::enum_index(key); // Index of the enum value
				items[keyIndex] = LoadedItemT{protoItem, args...};
			}
			return MessageLUT{std::move(items)};
		}
	};
}
