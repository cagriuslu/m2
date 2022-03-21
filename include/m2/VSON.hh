#ifndef VSON_H
#define VSON_H

#include <string>
#include <variant>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <optional>

namespace m2 {
	struct vson {
		using vson_nil = std::monostate;
		using vson_object = std::unordered_map<std::string, vson>;
		using vson_array = std::vector<vson>;
		using vson_string = std::string;

		std::variant<vson_nil, vson_object, vson_array, vson_string> value;

		[[nodiscard]] const vson* query(const std::string& path) const;

		[[nodiscard]] std::string dump_to_string() const;
		[[nodiscard]] bool dump_to_file(const std::string& fpath) const;

		static vson object();
		static vson array();
		static vson string(const std::string& str);
		static std::optional<vson> parse_string(const std::string& str);
		static std::optional<vson> parse_file(const std::string& fpath);
	};
}

#endif
