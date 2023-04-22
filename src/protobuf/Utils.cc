#include <m2/protobuf/Utils.h>

m2::Value<std::string> m2::protobuf::message_to_json_string(const google::protobuf::Message& message) {
	std::string str;
	auto status = google::protobuf::util::MessageToJsonString(message, &str);
	if (status.ok()) {
		return str;
	} else {
		return failure(status.ToString());
	}
}

m2::VoidValue m2::protobuf::message_to_json_file(const google::protobuf::Message& message, const std::string& path) {
	std::string str;
	auto status = google::protobuf::util::MessageToJsonString(message, &str);
	if (status.ok()) {
		return write_to_file(str, path);
	} else {
		return failure(status.ToString());
	}
}
