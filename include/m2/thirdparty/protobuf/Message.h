#pragma once
#include <google/protobuf/message.h>
#include <concepts>

namespace m2::thirdparty::protobuf {
	template <typename T>
	concept ProtobufMessage = std::derived_from<T, google::protobuf::Message>;
}
