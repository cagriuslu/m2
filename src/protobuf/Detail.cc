#include <m2/protobuf/Detail.h>

m2::expected<std::string> m2::protobuf::message_to_json_string(const google::protobuf::Message& message) {
	std::string str;
	auto status = google::protobuf::util::MessageToJsonString(message, &str);
	if (status.ok()) {
		return str;
	} else {
		return make_unexpected(status.ToString());
	}
}

m2::void_expected m2::protobuf::message_to_json_file(const google::protobuf::Message& message, const std::filesystem::path& path) {
	std::string str;
	auto status = google::protobuf::util::MessageToJsonString(message, &str);
	if (status.ok()) {
		return write_to_file(str, path);
	} else {
		return make_unexpected(status.ToString());
	}
}
