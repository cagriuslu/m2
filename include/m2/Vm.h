#pragma once
#include "Meta.h"
#include <array>
#include <string>
#include <variant>
#include <list>

namespace m2 {
	namespace vm {
		enum InstructionType {
			WHITESPACE,
			SIMPLE,
			WITH_STRING_ARG,
			WITH_INTEGER_ARG,
			NUMBER = '`',
			DECIMAL_SEPARATOR = '.',
			STRING = '"',
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

			PUSH_EMPTY = '{', // Array of object
			GET_INDIRECT = '[',
			SET_INDIRECT = ']',
			POP = '}',
		};
		enum StrArgInstruction : char {
			DEFINE_FUNC = ';',
			DEFINE_LABEL = ':',
			CALL_FUNC = ',',
			JUMP_POSITIVE = '>',
			JUMP_NEGATIVE = '<',
			JUMP_ZERO  = '?',
			JUMP = '!',

			GET_MEMBER = '@',
			SET_MEMBER = '$',
		};
		enum IntArgInstruction : char {
			STACK_GET = '(',
			STACK_SET = ')',
		};
		// Unused: '
	}

	class Vm {
	public:
		using InstructionIndex = size_t;
		using Labels = std::unordered_map<std::string, InstructionIndex>;

	private:
		struct Command {
			char inst{};
			std::variant<std::monostate, int, float, std::string> arg{};
			inline explicit Command(char inst) : inst(inst) {}
			inline Command(char inst, int i) : inst(inst), arg(i) {}
			inline Command(char inst, float f) : inst(inst), arg(f) {}
			inline Command(char inst, std::string s) : inst(inst), arg(std::move(s)) {}
		};
		using Commands = std::vector<Command>;
		using CommandsAndLabels = std::pair<Commands, Labels>;
		std::unordered_map<std::string, CommandsAndLabels> _functions;

	public:
		void_expected add_script(const std::string& script);
		void_expected add_script_file(const std::string& path);

		struct Object;
		using ObjectKey = std::string;
		using ObjectValue = std::variant<int, float, std::string, Object>;
		struct Object { std::unordered_map<ObjectKey, ObjectValue> map; };
		using ReturnValue = std::variant<std::monostate, int, float, std::string, Object>;
		[[nodiscard]] m2::expected<ReturnValue> execute(const std::string& func) const;

		using StackValue = ObjectValue;
		using Stack = std::list<StackValue>;
	private:
		void execute_recursively(Stack& stack, const std::string& func) const;
	};

	std::string to_string(const Vm::StackValue& value);
}
