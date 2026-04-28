#pragma once
#include <m2/Meta.h>
#include <google/protobuf/message.h>

namespace m2::thirdparty::protobuf {
	expected<std::string> ToJsonString(const google::protobuf::Message& message);
}
