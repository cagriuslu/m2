#define _CRT_SECURE_NO_WARNINGS
#include <m2/VSON.hh>
#include <m2/string.hh>
#include <m2/Def.hh>
#include <cstdlib>

#define ISPLAIN(c) (isalnum(c) || (c) == '_' || (c) == '-' || (c) == '.')

const m2::vson* m2::vson::query(const std::string& path) const {
	const vson* v = this;
	for (const auto& path_piece : m2::string::split(path, '/')) {
		if (std::holds_alternative<vson_object>(value)) {
			const auto& obj = std::get<vson_object>(value);
			const auto& it = obj.find(path_piece);
			if (it != obj.end()) {
				v = &it->second;
			} else {
				return nullptr;
			}
		} else if (std::holds_alternative<vson_array>(value)) {
			const auto& arr = std::get<vson_array>(value);
			long index = strtol(path_piece.c_str(), nullptr, 10);
			if (index < arr.size()) {
				v = &arr[index];
			} else {
				return nullptr;
			}
		} else if (std::holds_alternative<vson_string>(value)) {
			return nullptr;
		}
	}
	return v;

}

static void dump_any_value(const m2::vson& v, std::stringstream& ss) {
	auto print_string = [](std::stringstream& ss, const std::string& str){
		ss << '"';
		for (auto c : str) {
			if (c == '\\') {
				ss << R"(\\)";
			} else if (c == '"') {
				ss << R"(\")";
			} else {
				ss << c;
			}
		}
		ss << '"';
	};

	if (std::holds_alternative<m2::vson::vson_object>(v.value)) {
		ss << '{';
		for (const auto& kv : std::get<m2::vson::vson_object>(v.value)) {
			print_string(ss, kv.first);
			ss << ':';
			dump_any_value(kv.second, ss);
			ss << ',';
		}
		ss << '}';
	} else if (std::holds_alternative<m2::vson::vson_array>(v.value)) {
		ss << '[';
		for (const auto& elem : std::get<m2::vson::vson_array>(v.value)) {
			dump_any_value(elem, ss);
			ss << ',';
		}
		ss << ']';
	} else if (std::holds_alternative<m2::vson::vson_string>(v.value)) {
		print_string(ss, std::get<m2::vson::vson_string>(v.value));
	}
}

std::string m2::vson::dump_to_string() const {
	std::stringstream ss;
	dump_any_value(*this, ss);
	return ss.str();
}
bool m2::vson::dump_to_file(const std::string& fpath) const {
	FILE* file = fopen(fpath.c_str(), "w");
	if (!file) {
		LOG_ERROR_M2VV(M2ERR_FILE_INACCESSIBLE, CString, fpath.c_str(), M2ERR_ERRNO, CString, strerror(errno));
		return false;
	}
	std::string str = dump_to_string();
	bool success = (str.size() == fwrite(str.data(), 1, str.size(), file));
	fclose(file);
	return success;
}

m2::vson m2::vson::object() {
	return {.value = vson_object{}};
}
m2::vson m2::vson::array() {
	return {.value = vson_array{}};
}
m2::vson m2::vson::string(const std::string& str) {
	return {.value = vson_string{str}};
}

static std::optional<m2::vson> parse_unknown_value(std::stringstream& ss);

static std::string fetch_string_plain(std::stringstream& ss) {
	std::stringstream buffer;
	int c;
	while ((c = ss.get()) != EOF) {
		if (ISPLAIN(c)) {
			ss << (char)c;
		} else {
			ss.unget();
		}
	}
	return ss.str();
}
static std::optional<m2::vson> parse_object(std::stringstream& ss) {
	m2::vson v = m2::vson::object();
	auto& obj = std::get<m2::vson::vson_object>(v.value);

	const int EXPECT_KEY = 0;
	const int EXPECT_COLON = 1;
	const int EXPECT_VALUE = 2;
	const int EXPECT_COMMA_OR_SPACE = 3;

	std::string key;
	int c, braceClosed = 0, state = EXPECT_KEY;
	while ((c = ss.get()) != EOF) {
		if (state == EXPECT_KEY) {
			if (c == '}') {
				braceClosed = 1;
				break;
			} else if (isspace(c)) {
				// Do nothing
			} else if (ISPLAIN(c)) {
				// Create key-value pair
				ss.unget();
				key = fetch_string_plain(ss);
				// Next state
				state = EXPECT_COLON;
			} else {
				return {};
			}
		} else if (state == EXPECT_COLON) {
			if (isspace(c)) {
				// Do nothing
			} else if (c == ':') {
				// Next state
				state = EXPECT_VALUE;
			} else {
				return {};
			}
		} else if (state == EXPECT_VALUE) {
			if (isspace(c)) {
				// Do nothing
			} else {
				// Parse value
				ss.unget();
				auto value = parse_unknown_value(ss);
				if (value) {
					obj[key] = *value;
				} else {
					return {};
				}
				// Next state
				state = EXPECT_COMMA_OR_SPACE;
			}
		} else {
			if (c == '}') {
				braceClosed = 1;
				break;
			} else if (c == ',' || isspace(c)) {
				// Next state
				state = EXPECT_KEY;
			} else {
				return {};
			}
		}
	}
	if (!braceClosed) {
		return {};
	}
	return v;
}
static std::optional<m2::vson> parse_array(std::stringstream& ss) {
	auto v = m2::vson::array();
	auto& arr = std::get<m2::vson::vson_array>(v.value);

	const int EXPECT_VALUE = 0;
	const int EXPECT_COMMA_OR_SPACE = 1;

	int c, bracketClosed = 0, state = EXPECT_VALUE;
	while ((c = ss.get()) != EOF) {
		if (state == EXPECT_VALUE) {
			if (c == ']') {
				bracketClosed = 1;
				break;
			} else if (isspace(c)) {
				// Do nothing
			} else {
				ss.unget();
				auto value = parse_unknown_value(ss);
				if (value) {
					arr.push_back(*value);
				} else {
					return {};
				}
				// Next state
				state = EXPECT_COMMA_OR_SPACE;
			}
		} else {
			if (c == ']') {
				bracketClosed = 1;
				break;
			} else if (c == ',' || isspace(c)) {
				// Next state
				state = EXPECT_VALUE;
			} else {
				return {};
			}
		}
	}
	if (!bracketClosed) {
		return {};
	}
	return v;
}
static std::optional<m2::vson> parse_string_quoted(std::stringstream& ss) {
	std::stringstream buffer;
	int c, escaping = 0, quoteClosed = 0;
	while ((c = ss.get()) != EOF) {
		if (escaping) {
			if (c == '"' || c == '\\') {
				buffer << (char)c;
				escaping = 0;
			} else {
				// unexpected character
				return {};
			}
		} else {
			if (c == '"') {
				// String finished
				quoteClosed = 1;
				break;
			} else if (c == '\\') {
				escaping = 1;
			} else if (isprint(c)) {
				buffer << (char)c;
			} else {
				// unexpected character
				return {};
			}
		}
	}
	if (!quoteClosed) {
		// Quote not closed
		return {};
	}
	return m2::vson::string(buffer.str());
}
static std::optional<m2::vson> parse_string_plain(std::stringstream& ss) {
	auto str = fetch_string_plain(ss);
	return m2::vson::string(fetch_string_plain(ss));
}
static std::optional<m2::vson> parse_unknown_value(std::stringstream& ss) {
	int c;
	while ((c = ss.get()) != EOF) {
		if (not isspace(c)) {
			if (c == '{') {
				return parse_object(ss);
			} else if (c == '[') {
				return parse_array(ss);
			} else if (c == '"') {
				return parse_string_quoted(ss);
			} else if (ISPLAIN(c)) {
				ss.unget();
				return parse_string_plain(ss);
			} else {
				return {};
			}
		}
	}
	return {};
}

std::optional<m2::vson> m2::vson::parse_string(const std::string& str) {
	std::stringstream ss(str);
	auto optional_vson = parse_unknown_value(ss);
	if (optional_vson) {
		int c;
		while ((c = ss.get()) != EOF) {
			if (not isspace(c)) {
				return {};
			}
		}
		return optional_vson;
	} else {
		return {};
	}
}
std::optional<m2::vson> m2::vson::parse_file(const std::string &fpath) {
	FILE* file = fopen(fpath.c_str(), "r");
	if (!file) {
		LOG_ERROR_M2VV(M2ERR_FILE_INACCESSIBLE, CString, fpath.c_str(), M2ERR_ERRNO, CString, strerror(errno));
		return {};
	}

	std::stringstream ss;
	while (not feof(file)) {
		char buffer[512];
		long n = static_cast<long>(fread(buffer, 1, 512, file));
		ss.write(buffer, n);
	}
	fclose(file);
	return parse_string(ss.str());
}
