#include <m2/Vm.h>
#include <m2/FileSystem.h>
#include <m2/Exception.h>
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
			throw M2ERROR("Unexpected identifier character: " + std::to_string(c));
		}
	}

	std::string fetch_until_except_character(std::stringstream& ss, int limiter) {
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
						throw M2ERROR(std::string("Unexpected escape character") + (char) c);
				}
			}
			out << (char)c;
			escaping = false;
		}

		return out.str();
	}

	int fetch_integer(std::stringstream& ss) {
		std::stringstream i_str;

		// Parse until non-digit character is fetched
		int c, n = 0;
		for (c = ss.get(); isdigit(c) || (c == '-' && n == 0); c = ss.get(), ++n) {
			i_str << (char)c;
		}

		if (is_whitespace(c)) {
			// Success
			auto str = i_str.str();
			return (int) strtol(str.c_str(), nullptr, 10);
		} else {
			throw M2ERROR("Unexpected digit character: " + std::to_string(c));
		}
	}

	int fetch_character(std::stringstream& ss) {
		int c = ss.get();
		if (c == m2::vm::_ESCAPE) {
			c = ss.get();
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
					throw M2ERROR(std::string("Unexpected escape character") + (char) c);
			}
		}

		return c;
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
			case ARRAY_INDIRECT:
			case STACK_INDIRECT:
			case PUSH_OBJECT:
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

			case ARRAY_DIRECT:
			case STACK_DIRECT:
				return WITH_INTEGER_ARG;

			case INTEGER:
				return INTEGER;

			case STRING:
				return STRING;

			case CHAR:
				return CHAR;

			case COMMENT:
				return COMMENT;

			default:
				throw M2ERROR(std::string("Unexpected character: ") + c);
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
				throw M2ERROR(std::string("Multiple definition of function: ") + *current_function);
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
				fetch_until_except_character(ss, '\n');
			} else if (c == DEFINE_FUNC) {
				// Begin new function
				auto func_name = fetch_identifier(ss);
				if (!func_name.empty()) {
					save_function();
					// Start current function
					current_function = func_name;
				} else {
					throw M2ERROR("Expected function name");
				}
			} else if (c == DEFINE_LABEL) {
				// Record label
				auto label = fetch_identifier(ss);
				if (current_labels.contains(label)) {
					// Check for multiple definitions
					throw M2ERROR(std::string("Multiple definition of label: ") + label);
				}
				current_labels[label] = current_commands.size();
			} else if (current_function) {
				// Save command
				auto inst_type = determine_instruction_type((char) c);
				switch (inst_type) {
					case SIMPLE:
						current_commands.emplace_back(c, std::monostate{});
						break;
					case WITH_STRING_ARG:
						current_commands.emplace_back(c, fetch_identifier(ss));
						break;
					case WITH_INTEGER_ARG:
					case INTEGER:
						current_commands.emplace_back(c, fetch_integer(ss));
						break;
					case STRING:
						current_commands.emplace_back(c, fetch_until_except_character(ss, '"'));
						break;
					case CHAR:
						current_commands.emplace_back(c, fetch_character(ss));
						break;
					default:
						throw M2ERROR(std::string("Unexpected instruction type: ") + std::to_string(inst_type));
				}
			} else {
				throw M2ERROR("Expected function before instruction");
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
	m2_reflect_failure(str);
	return add_script(*str);
}
