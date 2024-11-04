#include <m2/Vm.h>
#include <m2/FileSystem.h>
#include <m2/Error.h>
#include <m2/Log.h>
#include <sstream>

namespace {
	bool is_whitespace(int c) {
		return (c == ' ') || (c == '\t') || (c == '\n');
	}

	bool is_identifier_char(int c) {
		return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || (c == '_');
	}

	std::string fetch_identifier(std::stringstream& ss) {
		std::stringstream id;

		// Parse until non-identifier character is fetched
		int c;
		for (c = ss.get(); is_identifier_char(c); c = ss.get()) {
			id << (char)c;
		}

		if (is_whitespace(c)) {
			// Success
			return id.str();
		} else {
			throw M2_ERROR("Unexpected identifier character: " + std::to_string(c));
		}
	}

	/// Fetches a string from the stream until the limiter character is encountered.
	/// The limiter character is not present in the returned string.
	std::string fetch_until_char(std::stringstream& ss, int limiter) {
		std::stringstream out;

		bool escaping = false;
		for (int c = ss.get(); c != EOF && c != limiter; c = ss.get()) {
			if (!escaping && c == m2::vm::_ESCAPE) {
				escaping = true;
				continue;
			} else if (escaping) {
				switch (c) {
					case 'a':
						c = '\a';
						break;
					case 'b':
						c = '\b';
						break;
					case 'f':
						c = '\f';
						break;
					case 'n':
						c = '\n';
						break;
					case 'r':
						c = '\r';
						break;
					case 't':
						c = '\t';
						break;
					case 'v':
						c = '\v';
						break;
					case '"':
						c = '"';
						break;
					case '\\':
						c = '\\';
						break;
					default:
						throw M2_ERROR(std::string("Unexpected escape character") + (char) c);
				}
			}
			out << (char)c;
			escaping = false;
		}

		return out.str();
	}

	std::variant<int,float> fetch_number(std::stringstream& ss) {
		std::stringstream n_str;

		// Parse until non-digit character is fetched
		int c, n = 0;
		bool decimal = false;
		for (c = ss.get(); (n == 0 && c == '-') || isdigit(c) || (!decimal && c == '.'); c = ss.get(), ++n) {
			if (c == '.') {
				decimal = true;
			}
			n_str << (char)c;
		}

		if (is_whitespace(c)) {
			// Success
			auto str = n_str.str();
			if (decimal) {
				return strtof(str.c_str(), nullptr);
			} else {
				return (int) strtol(str.c_str(), nullptr, 10);
			}
		} else {
			throw M2_ERROR("Unexpected digit character: " + std::to_string(c));
		}
	}

	int fetch_integer(std::stringstream& ss) {
		auto n = fetch_number(ss);
		return std::visit(m2::overloaded {
			[](int i) -> int { return i; },
			[](float f) -> int { throw M2_ERROR("Expected integer: " + std::to_string(f)); }
		}, n);
	}

	m2::vm::InstructionType determine_instruction_type(char c) {
		using namespace m2::vm;
		switch (c) {
			case ADD:
			case SUBTRACT:
			case MULTIPLY:
			case DIVIDE:
			case RETURN:
			case BINARY_AND:
			case BINARY_OR:
			case BINARY_XOR:
			case BINARY_COMPLEMENT:
			case MODULO:
			case PUSH_EMPTY:
			case GET_INDIRECT:
			case SET_INDIRECT:
			case POP:
				return SIMPLE;

			case DEFINE_FUNC:
			case DEFINE_LABEL:
			case CALL_FUNC:
			case JUMP_POSITIVE:
			case JUMP_NEGATIVE:
			case JUMP_ZERO:
			case JUMP:
			case GET_MEMBER:
			case SET_MEMBER:
				return WITH_STRING_ARG;

			case STACK_GET:
			case STACK_SET:
				return WITH_INTEGER_ARG;

			case NUMBER:
				return NUMBER;

			case STRING:
				return STRING;

			case COMMENT:
				return COMMENT;

			default: {
				if (is_whitespace(c)) {
					return WHITESPACE;
				}
				throw M2_ERROR(std::string("Unexpected character: ") + c);
			}
		}
	}
}

m2::void_expected m2::Vm::add_script(const std::string& script) {
	using namespace vm;

	std::optional<std::string> current_function;
	Commands current_commands;
	Labels current_labels;

	auto save_function = [&]() {
		if (current_function) {
			// Save current function
			if (_functions.contains(*current_function)) {
				// Check for multiple definitions
				throw M2_ERROR(std::string("Multiple definition of function: ") + *current_function);
			}
			_functions[*current_function] = std::make_pair(current_commands, current_labels);
			current_commands.clear();
			current_labels.clear();
		}
	};

	try {
		std::stringstream ss{script};
		int c;
		while ((c = ss.get()) != EOF) {
			if (c == COMMENT) {
				// Skip comment line
				fetch_until_char(ss, '\n');
			} else if (c == DEFINE_FUNC) {
				// Begin new function
				auto func_name = fetch_identifier(ss);
				if (!func_name.empty()) {
					save_function();
					// Start current function
					current_function = func_name;
				} else {
					throw M2_ERROR("Expected function name");
				}
			} else if (c == DEFINE_LABEL) {
				// Record label
				auto label = fetch_identifier(ss);
				if (current_labels.contains(label)) {
					// Check for multiple definitions
					throw M2_ERROR(std::string("Multiple definition of label: ") + label);
				}
				current_labels[label] = current_commands.size();
			} else if (current_function) {
				// Save command
				LOGF_TRACE("Trying to determine the type of instruction '%c'...", c);
				auto inst_type = determine_instruction_type((char) c);
				switch (inst_type) {
					case WHITESPACE:
						break;
					case SIMPLE:
						current_commands.emplace_back(c);
						break;
					case WITH_STRING_ARG:
						current_commands.emplace_back(c, fetch_identifier(ss));
						break;
					case WITH_INTEGER_ARG:
						current_commands.emplace_back(c, fetch_integer(ss));
						break;
					case NUMBER:
						std::visit(m2::overloaded {
							[&](int i) {current_commands.emplace_back(c, i);},
							[&](float f) {current_commands.emplace_back(c, f);}
						}, fetch_number(ss));
						break;
					case STRING:
						current_commands.emplace_back(c, fetch_until_char(ss, '"'));
						break;
					default:
						throw M2_ERROR(std::string("Unexpected instruction type: ") + std::to_string(inst_type));
				}
			} else {
				throw M2_ERROR("Expected function before instruction");
			}
		}
		save_function();
	} catch (const Error& e) {
		return make_unexpected(e.what());
	}
	return {};
}

m2::void_expected m2::Vm::add_script_file(const std::string& path) {
	auto str = read_file(path);
	m2_reflect_unexpected(str);
	return add_script(*str);
}
