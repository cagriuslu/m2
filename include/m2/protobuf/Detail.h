#pragma once
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/util/json_util.h>

#include <string>
#include <vector>

#include "../Error.h"
#include "../FileSystem.h"
#include "../Meta.h"

namespace m2 {
	template <typename T>
	concept ProtoEnum = google::protobuf::is_proto_enum<T>::value;

	namespace pb {
		template <typename ProtoType>
		expected<ProtoType> json_string_to_message(const std::string& str) {
			ProtoType message;
			auto status = google::protobuf::util::JsonStringToMessage(str, &message);
			if (status.ok()) {
				return message;
			} else {
				return make_unexpected(status.ToString());
			}
		}

		template <typename ProtoType>
		expected<ProtoType> json_file_to_message(const std::filesystem::path& path) {
			auto str = ReadFile(path);
			m2ReflectUnexpected(str);

			ProtoType message;
			auto status = google::protobuf::util::JsonStringToMessage(str.value(), &message);
			if (status.ok()) {
				return message;
			} else {
				return make_unexpected(status.ToString());
			}
		}

		expected<std::string> message_to_json_string(const google::protobuf::Message& message);
		void_expected message_to_json_file(const google::protobuf::Message& message, const std::filesystem::path& path);

		template <typename T>
		T* mutable_get_or_create(
		    ::google::protobuf::RepeatedField<T>* mutable_repeated_field, int index, const T& fill_value = {}) {
			T* t = nullptr;
			while (mutable_repeated_field->size() <= index) {
				t = mutable_repeated_field->Add();
				*t = fill_value;
			}
			if (!t) {
				t = &mutable_repeated_field->at(index);
			}
			return t;
		}

		template <typename T>
		T* mutable_get_or_create(::google::protobuf::RepeatedPtrField<T>* mutable_repeated_field, int index) {
			T* t = nullptr;
			while (mutable_repeated_field->size() <= index) {
				t = mutable_repeated_field->Add();
			}
			if (!t) {
				t = &mutable_repeated_field->at(index);
			}
			return t;
		}

		/// Inserts a new object right before the given index. Returns null if index is out-of-bounds
		template <typename T>
		T* mutable_insert(::google::protobuf::RepeatedField<T>* mutable_repeated_field, int index) {
			if (index <= mutable_repeated_field->Size()) {
				// Append element
				auto* new_element = mutable_repeated_field->Add();
				// Swap elements
				for (int i = mutable_repeated_field->Size() - 1; index < i--;) {
					mutable_repeated_field->SwapElements(i, i + 1);
				}
				return new_element;
			} else {
				return nullptr;
			}
		}

		/// Inserts a new object right before the given index. Returns null if index is out-of-bounds
		template <typename T>
		T* mutable_insert(::google::protobuf::RepeatedPtrField<T>* mutable_repeated_field, int index) {
			if (index <= mutable_repeated_field->size()) {
				// Append element
				auto* new_element = mutable_repeated_field->Add();
				// Swap elements
				for (int i = mutable_repeated_field->size() - 1; index < i--;) {
					mutable_repeated_field->SwapElements(i, i + 1);
				}
				return new_element;
			} else {
				return nullptr;
			}
		}

		template <ProtoEnum T>
		int enum_value_count() {
			static const auto* const descriptor = ::google::protobuf::GetEnumDescriptor<T>();
			return descriptor->value_count();
		}

		template <ProtoEnum T>
		int enum_index(T enum_value) {
			static const auto* const descriptor = ::google::protobuf::GetEnumDescriptor<T>();
			return descriptor->FindValueByNumber(enum_value)->index();
		}

		template <ProtoEnum T>
		T enum_value(int index) {
			static const auto* const descriptor = ::google::protobuf::GetEnumDescriptor<T>();
			return static_cast<T>(descriptor->value(index)->number());
		}

		template <ProtoEnum T, typename ParserT>
		expected<T> _enum_value(ParserT* parser, const std::string& name) {
			T en;
			if (parser(name, &en)) {
				return en;
			} else {
				return make_unexpected("Unable to parse as enum: " + name);
			}
		}

		template <ProtoEnum T>
		const std::string& enum_name(int index) {
			static const auto* const descriptor = ::google::protobuf::GetEnumDescriptor<T>();
			return descriptor->value(index)->name();
		}

		template <ProtoEnum T>
		const std::string& enum_name(T enum_value) {
			static const auto* const descriptor = ::google::protobuf::GetEnumDescriptor<T>();
			return descriptor->FindValueByNumber(enum_value)->name();
		}

		template <ProtoEnum T>
		void for_each_enum_value(const std::function<void(T)>& op) {
			for (int i = 0; i < enum_value_count<T>(); ++i) {
				op(enum_value<T>(i));
			}
		}
	}  // namespace pb

	template <ProtoEnum T>
	std::string ToString(const T& enum_val) {
		return pb::enum_name(enum_val);
	}

	template <ProtoEnum T>
	std::string ToString(const std::vector<T>& enum_arr) {
		std::string s = "[";
		for (const auto& enum_val : enum_arr) {
			s = s + ToString(enum_val) + ",";
		}
		s += "]";
		return s;
	}
}  // namespace m2

#define m2_pb_enum_value(type, enum_name) (m2::pb::_enum_value<type>(type##_Parse, (enum_name)))
