#include <m2/thirdparty/protobuf/Json.h>
#include <m2/protobuf/Detail.h>

using namespace m2;
using namespace m2::thirdparty;
using namespace m2::thirdparty::protobuf;

expected<std::string> protobuf::ToJsonString(const google::protobuf::Message& message) {
	return pb::message_to_json_string(message);
}
