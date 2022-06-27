#ifndef VSON_H
#define VSON_H

#include "Value.h"
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

		[[nodiscard]] static VSON nil();
		[[nodiscard]] static VSON object();
		[[nodiscard]] static VSON array();
		[[nodiscard]] static VSON string(const std::string& str);

		[[nodiscard]] bool is_nil() const;
		[[nodiscard]] bool is_valid() const;
		[[nodiscard]] bool is_object() const;
		[[nodiscard]] bool is_array() const;
		[[nodiscard]] bool is_string() const;

		// Safe queries
		[[nodiscard]] const VSON* query(const std::string& path) const;
		[[nodiscard]] Value<std::string> query_string_value(const std::string& path) const;
		[[nodiscard]] Value<long> query_long_value(const std::string& path) const;
		[[nodiscard]] Value<double> query_double_value(const std::string& path) const;

		// Object
		[[nodiscard]] size_t object_size() const;
		const VSON& operator[](const std::string& key) const;
		VSON& operator[](const std::string& key);
		[[nodiscard]] const VSON* at(const std::string& key) const;
		VSON* at(const std::string& key);

		// Array
		[[nodiscard]] size_t array_length() const;
		const VSON& operator[](size_t index) const;
		VSON& operator[](size_t index);
		[[nodiscard]] const VSON* at(size_t index) const;
		VSON* at(size_t index);

		// String
		[[nodiscard]] const std::string& string_value() const;
		[[nodiscard]] long long_value() const;
		[[nodiscard]] double double_value() const;
		VSON& operator=(const std::string& str);

		[[nodiscard]] std::string dump_to_string() const;
		[[nodiscard]] bool dump_to_file(const std::string& fpath) const;
		static Value<VSON> parse_string(const std::string& str);
		static Value<VSON> parse_file(const std::string& fpath);
	};
}

#endif
