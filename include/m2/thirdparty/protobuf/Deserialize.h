#pragma once
#include "Message.h"

namespace m2::thirdparty::protobuf {
	template <ProtobufMessage T>
	std::optional<T> ToMessage(const std::vector<uint8_t>& bytes) {
		if (T t; t.ParseFromArray(bytes.data(), bytes.size())) {
			return t;
		}
		return std::nullopt;
	}
}
