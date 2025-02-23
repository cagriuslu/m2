#define _CRT_SECURE_NO_WARNINGS
#include <m2/Vson.h>
#include <m2/String.h>
#include <m2/Log.h>
#include <cstdlib>

#define ISPLAIN(c) (isalnum(c) || (c) == '_' || (c) == '-' || (c) == '.')

m2::Vson m2::Vson::nil() {
	return {.value = vson_nil{}};
}

m2::Vson m2::Vson::object() {
	return {.value = vson_object{}};
}
m2::Vson m2::Vson::array() {
	return {.value = vson_array{}};
}
m2::Vson m2::Vson::string(const std::string& str) {
	return {.value = vson_string{str}};
}

bool m2::Vson::is_nil() const {
	return std::holds_alternative<vson_nil>(value);
}

bool m2::Vson::is_valid() const {
	return !is_nil();
}

bool m2::Vson::is_object() const {
	return std::holds_alternative<vson_object>(value);
}

bool m2::Vson::is_array() const {
	return std::holds_alternative<vson_array>(value);
}

bool m2::Vson::is_string() const {
	return std::holds_alternative<vson_string>(value);
}

const m2::Vson* m2::Vson::query(const std::string& path) const {
	const Vson* v = this;
	for (const auto& path_piece : m2::SplitString(path, '/')) {
		if (std::holds_alternative<vson_object>(value)) {
			const auto& obj = std::get<vson_object>(value);
			const auto it = obj.find(path_piece);
			if (it != obj.end()) {
				v = &it->second;
			} else {
				return nullptr;
			}
		} else if (std::holds_alternative<vson_array>(value)) {
			const auto& arr = std::get<vson_array>(value);
			unsigned long index = strtoul(path_piece.c_str(), nullptr, 10);
			if (index < arr.size()) {
				v = &arr[index];
			} else {
				return nullptr;
			}
		} else {
			return nullptr;
		}
	}
	return v;
}

m2::expected<std::string> m2::Vson::query_string_value(const std::string& path) const {
	const auto* vson = query(path);
	m2_return_unexpected_message_unless(vson, "VSON path not found");
	m2_return_unexpected_message_unless(vson->is_string(), "VSON value not string");
	return vson->string_value();
}

m2::expected<long> m2::Vson::query_long_value(const std::string& path) const {
	const auto* vson = query(path);
	m2_return_unexpected_message_unless(vson, "VSON path not found");
	m2_return_unexpected_message_unless(vson->is_string(), "VSON value not string");
	return vson->long_value();
}

m2::expected<double> m2::Vson::query_double_value(const std::string& path) const {
	const auto* vson = query(path);
	m2_return_unexpected_message_unless(vson, "VSON path not found");
	m2_return_unexpected_message_unless(vson->is_string(), "VSON value not string");
	return vson->double_value();
}

const std::string& m2::Vson::string_value() const {
	return std::get<vson_string>(value);
}

long m2::Vson::long_value() const {
	return std::stol(string_value());
}

double m2::Vson::double_value() const {
	return std::stod(string_value());
}

size_t m2::Vson::object_size() const {
	return std::get<vson_object>(value).size();
}

const m2::Vson& m2::Vson::operator[](const std::string &key) const {
	return std::get<vson_object>(value).at(key);
}

m2::Vson& m2::Vson::operator[](const std::string &key) {
	if (std::holds_alternative<vson_nil>(value)) {
		value = vson_object{};
	}
	return std::get<vson_object>(value)[key];
}

const m2::Vson* m2::Vson::at(const std::string& key) const {
	if (not is_object()) {
		return nullptr;
	}
	const auto& obj = std::get<vson_object>(value);
	auto it = obj.find(key);
	if (it == obj.end()) {
		return nullptr;
	}
	return &it->second;
}

m2::Vson* m2::Vson::at(const std::string& key) {
	if (not is_object()) {
		return nullptr;
	}
	auto& obj = std::get<vson_object>(value);
	auto it = obj.find(key);
	if (it == obj.end()) {
		return nullptr;
	}
	return &it->second;
}

size_t m2::Vson::array_length() const {
	return std::get<vson_array>(value).size();
}

const m2::Vson& m2::Vson::operator[](size_t index) const {
	return std::get<vson_array>(value)[index];
}

m2::Vson& m2::Vson::operator[](size_t index) {
	if (std::holds_alternative<vson_nil>(value)) {
		value = vson_array{};
	}
	auto& array = std::get<vson_array>(value);
	if (array.size() < index + 1) {
		array.resize(index + 1);
	}
	return array[index];
}

const m2::Vson* m2::Vson::at(size_t index) const {
	if (not is_array()) {
		return nullptr;
	}
	const auto& arr = std::get<vson_array>(value);
	if (arr.size() <= index) {
		return nullptr;
	}
	return &arr[index];
}

m2::Vson* m2::Vson::at(size_t index) {
	if (not is_array()) {
		return nullptr;
	}
	auto& arr = std::get<vson_array>(value);
	if (arr.size() <= index) {
		return nullptr;
	}
	return &arr[index];
}

void m2::Vson::push_back(Vson&& v) {
	if (std::holds_alternative<vson_nil>(value)) {
		value = vson_array{};
	}

	if (std::holds_alternative<vson_array>(value)) {
		std::get<vson_array>(value).emplace_back(std::move(v));
	} else {
		throw std::bad_variant_access();
	}
}

m2::Vson& m2::Vson::operator=(const std::string& str) {
	if (std::holds_alternative<vson_nil>(value) || std::holds_alternative<vson_string>(value)) {
		value = str;
		return *this;
	} else {
		throw std::bad_variant_access();
	}
}

static void dump_any_value(const m2::Vson& v, std::stringstream& ss) {
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

	if (std::holds_alternative<m2::Vson::vson_object>(v.value)) {
		ss << '{';
		for (const auto& kv : std::get<m2::Vson::vson_object>(v.value)) {
			print_string(ss, kv.first);
			ss << ':';
			dump_any_value(kv.second, ss);
			ss << ',';
		}
		ss << '}';
	} else if (std::holds_alternative<m2::Vson::vson_array>(v.value)) {
		ss << '[';
		for (const auto& elem : std::get<m2::Vson::vson_array>(v.value)) {
			dump_any_value(elem, ss);
			ss << ',';
		}
		ss << ']';
	} else if (std::holds_alternative<m2::Vson::vson_string>(v.value)) {
		print_string(ss, std::get<m2::Vson::vson_string>(v.value));
	}
}

std::string m2::Vson::dump_to_string() const {
	std::stringstream ss;
	dump_any_value(*this, ss);
	return ss.str();
}
bool m2::Vson::dump_to_file(const std::string& fpath) const {
	FILE* file = fopen(fpath.c_str(), "w");
	if (!file) {
		LOGF_ERROR("Unable to open file for writing: %s %s", fpath.c_str(), strerror(errno));
		return false;
	}
	std::string str = dump_to_string();
	bool success = (str.size() == fwrite(str.data(), 1, str.size(), file));
	fclose(file);
	return success;
}

static m2::expected<m2::Vson> parse_unknown_value(std::stringstream& ss);

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
static m2::expected<m2::Vson> parse_object(std::stringstream& ss) {
	m2::Vson v = m2::Vson::object();
	auto& obj = std::get<m2::Vson::vson_object>(v.value);

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
				return m2::make_unexpected("Unexpected character in object");
			}
		} else if (state == EXPECT_COLON) {
			if (isspace(c)) {
				// Do nothing
			} else if (c == ':') {
				// Next state
				state = EXPECT_VALUE;
			} else {
				return m2::make_unexpected("Unexpected character in object");
			}
		} else if (state == EXPECT_VALUE) {
			if (isspace(c)) {
				// Do nothing
			} else {
				// Parse value
				ss.unget();
				auto value = parse_unknown_value(ss);
				m2_reflect_unexpected(value);
				obj[key] = *value;
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
				return m2::make_unexpected("Unexpected character in object");
			}
		}
	}
	if (!braceClosed) {
		return m2::make_unexpected("Unclosed braces");
	}
	return v;
}
static m2::expected<m2::Vson> parse_array(std::stringstream& ss) {
	auto v = m2::Vson::array();
	auto& arr = std::get<m2::Vson::vson_array>(v.value);

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
				m2_reflect_unexpected(value);
				arr.push_back(*value);
				state = EXPECT_COMMA_OR_SPACE; // Next state
			}
		} else {
			if (c == ']') {
				bracketClosed = 1;
				break;
			} else if (c == ',' || isspace(c)) {
				state = EXPECT_VALUE; // Next state
			} else {
				return m2::make_unexpected("Unexpected character in array");
			}
		}
	}
	if (!bracketClosed) {
		return m2::make_unexpected("Unclosed bracket");
	}
	return v;
}
static m2::expected<m2::Vson> parse_string_quoted(std::stringstream& ss) {
	std::stringstream buffer;
	int c, escaping = 0, quoteClosed = 0;
	while ((c = ss.get()) != EOF) {
		if (escaping) {
			if (c == '"' || c == '\\') {
				buffer << (char)c;
				escaping = 0;
			} else {
				return m2::make_unexpected("Unexpected character");
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
				return m2::make_unexpected("Unexpected character");
			}
		}
	}
	if (!quoteClosed) {
		return m2::make_unexpected("Unclosed quote");
	}
	return m2::Vson::string(buffer.str());
}
static m2::expected<m2::Vson> parse_string_plain(std::stringstream& ss) {
	auto str = fetch_string_plain(ss);
	return m2::Vson::string(fetch_string_plain(ss));
}
static m2::expected<m2::Vson> parse_unknown_value(std::stringstream& ss) {
	int c;
	while ((c = ss.get()) != EOF) {
		if (isspace(c)) {
			continue;
		}
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
			return m2::make_unexpected("Unexpected character");
		}
	}
	return m2::make_unexpected("Unexpected end of file");
}

m2::expected<m2::Vson> m2::Vson::parse_string(const std::string& str) {
	std::stringstream ss(str);
	auto optional_vson = parse_unknown_value(ss);
	m2_reflect_unexpected(optional_vson);
	int c;
	while ((c = ss.get()) != EOF) {
		m2_return_unexpected_message_unless(isspace(c), "Extra characters after VSON");
	}
	return optional_vson;
}
m2::expected<m2::Vson> m2::Vson::parse_file(const std::string &fpath) {
	FILE* file = fopen(fpath.c_str(), "r");
	m2_return_unexpected_message_unless(file, "Unable to open file");
	std::stringstream ss;
	while (not feof(file)) {
		char buffer[512];
		long n = static_cast<long>(fread(buffer, 1, 512, file));
		ss.write(buffer, n);
	}
	fclose(file);
	return parse_string(ss.str());
}
