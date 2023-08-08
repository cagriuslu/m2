#pragma once
#include "Meta.h"
#include <string>
#include <variant>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <optional>

namespace m2 {
	struct Vson {
		using vson_nil = std::monostate;
		using vson_object = std::unordered_map<std::string, Vson>;
		using vson_array = std::vector<Vson>;
		using vson_string = std::string;
		std::variant<vson_nil, vson_object, vson_array, vson_string> value;

		[[nodiscard]] static Vson nil();
		[[nodiscard]] static Vson object();
		[[nodiscard]] static Vson array();
		[[nodiscard]] static Vson string(const std::string& str);

		[[nodiscard]] bool is_nil() const;
		[[nodiscard]] bool is_valid() const;
		[[nodiscard]] bool is_object() const;
		[[nodiscard]] bool is_array() const;
		[[nodiscard]] bool is_string() const;

		// Safe queries
		[[nodiscard]] const Vson* query(const std::string& path) const;
		[[nodiscard]] expected<std::string> query_string_value(const std::string& path) const;
		[[nodiscard]] expected<long> query_long_value(const std::string& path) const;
		[[nodiscard]] expected<double> query_double_value(const std::string& path) const;

		// Object
		[[nodiscard]] size_t object_size() const;
		const Vson& operator[](const std::string& key) const;
		Vson& operator[](const std::string& key);
		[[nodiscard]] const Vson* at(const std::string& key) const;
		Vson* at(const std::string& key);

		// Array
		[[nodiscard]] size_t array_length() const;
		const Vson& operator[](size_t index) const;
		Vson& operator[](size_t index);
		[[nodiscard]] const Vson* at(size_t index) const;
		Vson* at(size_t index);
		void push_back(Vson&& v);

		// String
		[[nodiscard]] const std::string& string_value() const;
		[[nodiscard]] long long_value() const;
		[[nodiscard]] double double_value() const;
		Vson& operator=(const std::string& str);

		[[nodiscard]] std::string dump_to_string() const;
		[[nodiscard]] bool dump_to_file(const std::string& fpath) const;
		static expected<Vson> parse_string(const std::string& str);
		static expected<Vson> parse_file(const std::string& fpath);
	};
}
