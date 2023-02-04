#ifndef M2_PROTOBUF_UTILS_H
#define M2_PROTOBUF_UTILS_H

#include "../Value.h"
#include "../String.h"
#include <google/protobuf/util/json_util.h>
#include <string>

namespace m2::proto {
	template <typename ProtoType>
	Value<ProtoType> json_string_to_message(const std::string& str) {
		ProtoType message;
		auto status = google::protobuf::util::JsonStringToMessage(str, &message);
		if (status.ok()) {
			return message;
		} else {
			return failure(status.ToString());
		}
	}

	template <typename ProtoType>
	Value<ProtoType> json_file_to_message(const std::string& path) {
		auto str = string::read_file(path);
		m2_reflect_failure(str);
		ProtoType message;
		auto status = google::protobuf::util::JsonStringToMessage(str.value(), &message);
		if (status.ok()) {
			return message;
		} else {
			return failure(status.ToString());
		}
	}

	Value<std::string> message_to_json_string(const google::protobuf::Message& message);
	VoidValue message_to_json_file(const google::protobuf::Message& message, const std::string& path);

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

	template <typename T>
	T* mutable_get_or_create(::google::protobuf::RepeatedField<T>* mutable_repeated_field, int index, const T& fill_value = {}) {
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
}

#endif //M2_PROTOBUF_UTILS_H
