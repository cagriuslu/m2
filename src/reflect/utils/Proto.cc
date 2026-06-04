#include <m2/reflect/utils/Proto.h>
#include <m2/reflect/Composite.h>
#include <m2/math/primitives/Exact.h>
#include <m2/Error.h>

using namespace m2;
using namespace m2::reflect;
using namespace m2::reflect::util;
using namespace m2::reflect::util::detail;

namespace {
	pb::AnyReflective* MutablePath(const Path& path, pb::Reflective& root) {
		if (path.empty()) {
			throw M2_ERROR("Path is empty");
		}

		using MapItem = google::protobuf::Map<int32_t, pb::AnyReflective>*;
		using SequenceItem = google::protobuf::RepeatedPtrField<pb::AnyReflective>*;
		using MapOrSequenceItem = std::variant<MapItem, SequenceItem>;
		MapOrSequenceItem current = [&]() -> google::protobuf::Map<int32_t, pb::AnyReflective>* {
			if (root.has_struct_()) {
				return root.mutable_struct_()->mutable_fields();
			} else if (root.has_variant()) {
				return root.mutable_variant()->mutable_variants();
			}
			throw M2_ERROR("Unknown root member");
		}();

		pb::AnyReflective* retval = nullptr;
		for (auto it = path.cbegin(); it != path.cend(); ++it) {
			const auto node = *it;
			const auto isLast = (it + 1) == path.cend();
			// Get pointer to any reflective pointed by the index in case this is the last step
			if (std::holds_alternative<MapItem>(current)) {
				// Missing items will automatically be created
				retval = &(*std::get<MapItem>(current))[std::get<int>(node)];
			} else if (std::holds_alternative<SequenceItem>(current)) {
				// We have to create the missing item
				auto* seq = std::get<SequenceItem>(current);
				const auto idx = std::get<int>(node);
				if (idx > seq->size()) {
					throw M2_ERROR("Sequence index is out of bounds");
				}
				if (idx == seq->size()) {
					seq->Add();
				}
				retval = seq->Mutable(idx);
			} else {
				throw M2_ERROR("Unknown path item");
			}
			// Point to next map or sequence item in case there are further indexes
			if (not isLast) {
				if (std::holds_alternative<CompositeType>(std::get<NodeType>(node))) {
					if (std::get<CompositeType>(std::get<NodeType>(node)) == CompositeType::Struct) {
						current = retval->mutable_struct_()->mutable_fields();
					} else if (std::get<CompositeType>(std::get<NodeType>(node)) == CompositeType::Variant) {
						current = retval->mutable_variant()->mutable_variants();
					} else {
						throw M2_ERROR("Unknown composite type");
					}
				} else if (std::holds_alternative<ContainerType>(std::get<NodeType>(node))) {
					if (std::get<ContainerType>(std::get<NodeType>(node)) == ContainerType::Sequence) {
						current = retval->mutable_sequence()->mutable_items();
					} else {
						throw M2_ERROR("Unknown composite type");
					}
				} else {
					throw M2_ERROR("Unknown node type");
				}
			}
		}
		return retval;
	}
}

void StoreAccessor::operator()(const Path& path) {
	MutablePath(path, *root)->set_empty(true);
}
void StoreAccessor::operator()(const Path& path, const bool v) {
	MutablePath(path, *root)->set_bool_(v);
}
void StoreAccessor::operator()(const Path& path, const uint8_t v) {
	MutablePath(path, *root)->set_byte(v);
}
void StoreAccessor::operator()(const Path& path, const int8_t v) {
	MutablePath(path, *root)->set_int8(v);
}
void StoreAccessor::operator()(const Path& path, const int16_t v) {
	MutablePath(path, *root)->set_int16(v);
}
void StoreAccessor::operator()(const Path& path, const int32_t v) {
	MutablePath(path, *root)->set_int32(v);
}
void StoreAccessor::operator()(const Path& path, const int64_t v) {
	MutablePath(path, *root)->set_int64(v);
}
void StoreAccessor::operator()(const Path& path, const float v) {
	MutablePath(path, *root)->set_float_(v);
}
void StoreAccessor::operator()(const Path& path, const double v) {
	MutablePath(path, *root)->set_double_(v);
}
void StoreAccessor::operator()(const Path& path, const Exact v) {
	MutablePath(path, *root)->set_exact(v.ToRawValue());
}
void StoreAccessor::operator()(const Path& path, const UserDefinedEnumValue e) {
	MutablePath(path, *root)->set_user_defined_enum(e.value);
}
void StoreAccessor::operator()(const Path& path, ContainerType) {
	// Make sure path to container is initialized
	MutablePath(path, *root);
}
void StoreAccessor::operator()(const Path& path, const CompositeType ct) {
	if (path.empty()) {
		if (ct == CompositeType::Struct) {
			root->mutable_struct_();
		} else if (ct == CompositeType::Variant) {
			root->mutable_variant();
		} else {
			throw M2_ERROR("Unknown composite type");
		}
	} else {
		// Make sure path to composite is initialized
		MutablePath(path, *root);
	}
}
