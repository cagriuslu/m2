#ifndef VSON_H
#define VSON_H

#include <string>
#include <variant>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <optional>

namespace m2 {
	struct VSON {
		using vson_nil = std::monostate;
		using vson_object = std::unordered_map<std::string, VSON>;
		using vson_array = std::vector<VSON>;
		using vson_string = std::string;
		std::variant<vson_nil, vson_object, vson_array, vson_string> value;

		static VSON nil();
		static VSON object();
		static VSON array();
		static VSON string(const std::string& str);

		[[nodiscard]] const VSON* query(const std::string& path) const;
		[[nodiscard]] bool is_nil() const;
		[[nodiscard]] bool is_valid() const;
		[[nodiscard]] const std::string& string_value() const;

		VSON& operator[](const std::string& key);
		VSON& operator[](const std::string_view& key);
		VSON& operator[](size_t index);
		VSON& operator=(const std::string& str);

		[[nodiscard]] std::string dump_to_string() const;
		[[nodiscard]] bool dump_to_file(const std::string& fpath) const;
		static std::optional<VSON> parse_string(const std::string& str);
		static std::optional<VSON> parse_file(const std::string& fpath);
	};
}

#endif
