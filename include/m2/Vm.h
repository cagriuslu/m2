#pragma once
#include "Meta.h"
#include <array>
#include <string>
#include <variant>

namespace m2 {
	namespace vm {
		enum InstructionType {
			SIMPLE,
			WITH_STRING_ARG,
			WITH_INTEGER_ARG,
			INTEGER = ',',
			STRING = '"',
			CHAR = '\'',
			_ESCAPE = '\\', // Not really an instruction
			COMMENT = '#',
		};
		enum SimpleInstruction : char {
			ADD = '+',
			SUBTRACT = '-',
			MULTIPLY = '*',
			DIVIDE = '/',
			RETURN = '=',
			BINARY_AND = '&',
			BINARY_OR = '|',
			BINARY_XOR = '^',
			BINARY_COMPLEMENT = '~',
			MODULO = '%',
			ARRAY_INDIRECT = ']',
			STACK_INDIRECT = ')',
			PUSH_OBJECT = '{',
			POP = '`',
			// unused '}'
		};
		enum StrArgInstruction : char {
			DEFINE_FUNC = ';',
			DEFINE_LABEL = ':',
			CALL_FUNC = '.',
			JUMP_POSITIVE = '>',
			JUMP_NEGATIVE = '<',
			JUMP_ZERO  = '?',
			JUMP = '!',
			GET_MEMBER = '@',
			SET_MEMBER = '$',
		};
		enum IntArgInstruction : char {
			ARRAY_DIRECT = '[',
			STACK_DIRECT = '(',
		};
	}

	class Vm {
		struct Command {
			char inst{};
			using Arg = std::variant<std::monostate, int, std::string>;
			Arg arg;

			inline Command(char _inst, Arg&& _arg) : inst(_inst), arg(std::move(_arg)) {}
		};
		using Commands = std::vector<Command>;
		using Labels = std::unordered_map<std::string, size_t>;
		using CommandsAndLabels = std::pair<Commands, Labels>;

		std::unordered_map<std::string, CommandsAndLabels> _functions;

	public:
		Vm() = default;

		void_expected add_script(const std::string& script);
		void_expected add_script_file(const std::string& path);
	};
}
