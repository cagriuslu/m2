#pragma once
#include "Message.h"
#include <google/protobuf/util/message_differencer.h>

namespace m2::thirdparty::protobuf {
	bool IsEqual(const ProtobufMessage auto& m1, const ProtobufMessage auto& m2) {
		return google::protobuf::util::MessageDifferencer::Equals(m1, m2);
	}
}
