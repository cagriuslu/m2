#include <m2/Proto.h>

m2::Value<std::string> m2::proto::message_to_json_string(const google::protobuf::Message& message) {
	std::string str;
	auto status = google::protobuf::util::MessageToJsonString(message, &str);
	if (status.ok()) {
		return str;
	} else {
		return failure(status.ToString());
	}
}

m2::VoidValue m2::proto::message_to_json_file(const google::protobuf::Message& message, const std::string& path) {
	std::string str;
	auto status = google::protobuf::util::MessageToJsonString(message, &str);
	if (status.ok()) {
		return string::write_to_file(str, path);
	} else {
		return failure(status.ToString());
	}
}
