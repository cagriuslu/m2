#pragma once
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/util/json_util.h>

#include <string>
#include <vector>

#include "../Error.h"
#include "../FileSystem.h"
#include "../Meta.h"

namespace m2 {
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
			auto str = read_file(path);
			m2_reflect_unexpected(str);

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

		/// Returns null if index is out-of-bounds
		template <typename T>
		T* mutable_insert(::google::protobuf::RepeatedField<T>* mutable_repeated_field, int index) {
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

		/// Returns null if index is out-of-bounds
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

		template <typename EnumT>
		int enum_value_count() {
			static const auto* const descriptor = ::google::protobuf::GetEnumDescriptor<EnumT>();
			return descriptor->value_count();
		}

		template <typename EnumT>
		int enum_index(EnumT enum_value) {
			static const auto* const descriptor = ::google::protobuf::GetEnumDescriptor<EnumT>();
			return descriptor->FindValueByNumber(enum_value)->index();
		}

		template <typename EnumT>
		EnumT enum_value(int index) {
			static const auto* const descriptor = ::google::protobuf::GetEnumDescriptor<EnumT>();
			return static_cast<EnumT>(descriptor->value(index)->number());
		}

		template <typename EnumT, typename ParserT>
		expected<EnumT> _enum_value(ParserT* parser, const std::string& name) {
			EnumT en;
			if (parser(name, &en)) {
				return en;
			} else {
				return make_unexpected("Unable to parse as enum: " + name);
			}
		}

		template <typename EnumT>
		const std::string& enum_name(int index) {
			static const auto* const descriptor = ::google::protobuf::GetEnumDescriptor<EnumT>();
			return descriptor->value(index)->name();
		}

		template <typename EnumT>
		const std::string& enum_name(EnumT enum_value) {
			static const auto* const descriptor = ::google::protobuf::GetEnumDescriptor<EnumT>();
			return descriptor->FindValueByNumber(enum_value)->name();
		}

		template <typename EnumT>
		void for_each_enum_value(const std::function<void(EnumT)>& op) {
			for (int i = 0; i < enum_value_count<EnumT>(); ++i) {
				op(enum_value<EnumT>(i));
			}
		}
	}  // namespace pb

	template <typename EnumT>
	std::string ToString(const EnumT& enum_val) {
		return pb::enum_name(enum_val);
	}

	template <typename EnumT>
	std::string ToString(const std::vector<EnumT>& enum_arr) {
		std::string s = "[";
		for (const auto& enum_val : enum_arr) {
			s = s + ToString(enum_val) + ",";
		}
		s += "]";
		return s;
	}
}  // namespace m2

#define m2_pb_enum_value(type, enum_name) (m2::pb::_enum_value<type>(type##_Parse, (enum_name)))
