#include <m2/Vm.h>
#include <sstream>

using namespace m2;
using namespace m2::vm;

namespace {
	void assert_stack_size(const Vm::Stack& stack, size_t sz) {
		if (stack.size() < sz) {
			throw M2_ERROR("Stack doesn't have enough items");
		}
	}
	void assert_is_number(const Vm::StackValue& value) {
		if (not std::holds_alternative<int>(value) && not std::holds_alternative<float>(value)) {
			throw M2_ERROR("Stack value is not a number: " + to_string(value));
		}
	}
	void assert_is_int(const Vm::StackValue& value) {
		if (not std::holds_alternative<int>(value)) {
			throw M2_ERROR("Stack value is not an integer: " + to_string(value));
		}
	}
	void assert_is_index(const Vm::StackValue& value) {
		if (not std::holds_alternative<int>(value) && not std::holds_alternative<std::string>(value)) {
			throw M2_ERROR("Stack value is not an index: " + to_string(value));
		}
	}
	void assert_is_obj(const Vm::StackValue& value) {
		if (not std::holds_alternative<Vm::Object>(value)) {
			throw M2_ERROR("Stack value is not an object: " + to_string(value));
		}
	}
	Vm::InstructionIndex assert_label(const Vm::Labels& labels, const std::string& label_name) {
		auto it = labels.find(label_name);
		if (it == labels.end()) {
			throw M2_ERROR("Label not found in function " + label_name);
		}
		return it->second;
	}

	const Vm::StackValue& peek_top(const Vm::Stack& stack) {
		return stack.back();
	}
	const Vm::StackValue& peek_second(const Vm::Stack& stack) {
		return *(++stack.rbegin());
	}
	const Vm::StackValue& peek_third(const Vm::Stack& stack) {
		return *(++++stack.rbegin());
	}
	Vm::StackValue& top(Vm::Stack& stack) {
		return stack.back();
	}
	Vm::StackValue& second(Vm::Stack& stack) {
		return *(++stack.rbegin());
	}
	Vm::StackValue& third(Vm::Stack& stack) {
		return *(++++stack.rbegin());
	}

	float as_float(const Vm::StackValue& value) {
		// Assumes value is number
		if (std::holds_alternative<float>(value)) {
			return std::get<float>(value);
		} else {
			return static_cast<float>(std::get<int>(value));
		}
	}
	std::string as_index(const Vm::StackValue& value) {
		// Assumes value is index
		if (std::holds_alternative<int>(value)) {
			return to_string(std::get<int>(value));
		} else {
			return std::get<std::string>(value);
		}
	}

	void add(const Vm::StackValue& top, const Vm::StackValue& second, Vm::StackValue& out) {
		assert_is_number(second);
		assert_is_number(top);
		if (std::holds_alternative<float>(second) || std::holds_alternative<float>(top)) {
			out.emplace<float>(as_float(second) + as_float(top));
		} else {
			out.emplace<int>(std::get<int>(second) + std::get<int>(top));
		}
	}
	void subtract(const Vm::StackValue& top, const Vm::StackValue& second, Vm::StackValue& out) {
		assert_is_number(top);
		assert_is_number(second);
		if (std::holds_alternative<float>(top) || std::holds_alternative<float>(second)) {
			out.emplace<float>(as_float(second) - as_float(top));
		} else {
			out.emplace<int>(std::get<int>(second) - std::get<int>(top));
		}
	}
	void multiply(const Vm::StackValue& top, const Vm::StackValue& second, Vm::StackValue& out) {
		assert_is_number(top);
		assert_is_number(second);
		if (std::holds_alternative<float>(top) || std::holds_alternative<float>(second)) {
			out.emplace<float>(as_float(second) * as_float(top));
		} else {
			out.emplace<int>(std::get<int>(second) * std::get<int>(top));
		}
	}
	void divide(const Vm::StackValue& top, const Vm::StackValue& second, Vm::StackValue& out) {
		assert_is_number(top);
		assert_is_number(second);
		if (std::holds_alternative<float>(top) || std::holds_alternative<float>(second)) {
			out.emplace<float>(as_float(second) / as_float(top));
		} else {
			out.emplace<int>(std::get<int>(second) / std::get<int>(top));
		}
	}
	void binary_and(const Vm::StackValue& top, const Vm::StackValue& second, Vm::StackValue& out) {
		assert_is_int(top);
		assert_is_int(second);
		out.emplace<int>(std::get<int>(second) & std::get<int>(top));
	}
	void binary_or(const Vm::StackValue& top, const Vm::StackValue& second, Vm::StackValue& out) {
		assert_is_int(top);
		assert_is_int(second);
		out.emplace<int>(std::get<int>(second) | std::get<int>(top));
	}
	void binary_xor(const Vm::StackValue& top, const Vm::StackValue& second, Vm::StackValue& out) {
		assert_is_int(top);
		assert_is_int(second);
		out.emplace<int>(std::get<int>(second) ^ std::get<int>(top));
	}
	void binary_complement(const Vm::StackValue& top, Vm::StackValue& out) {
		assert_is_int(top);
		out.emplace<int>(~std::get<int>(top));
	}
	void modulo(const Vm::StackValue& top, const Vm::StackValue& second, Vm::StackValue& out) {
		assert_is_int(top);
		assert_is_int(second);
		out.emplace<int>(std::get<int>(second) % std::get<int>(top));
	}
	void get_indirect(const Vm::StackValue& obj, const Vm::StackValue& index, Vm::StackValue& out) {
		assert_is_obj(obj);
		assert_is_index(index);
		auto index_str = as_index(index);
		auto it = std::get<Vm::Object>(obj).map.find(index_str);
		if (it == std::get<Vm::Object>(obj).map.end()) {
			throw M2_ERROR("Object does not contain member " + index_str + ": " + to_string(obj));
		}
		out = it->second;
	}
	void set_indirect(const Vm::StackValue& value, const Vm::StackValue& index, Vm::StackValue& obj) {
		assert_is_obj(obj);
		assert_is_index(index);
		auto index_str = as_index(index);
		std::get<Vm::Object>(obj).map[index_str] = value;
	}
	bool is_positive(const Vm::StackValue& value) {
		assert_is_number(value);
		if (std::holds_alternative<int>(value)) {
			return 0 < std::get<int>(value);
		} else {
			return 0.0f < std::get<float>(value);
		}
	}
	bool is_negative(const Vm::StackValue& value) {
		assert_is_number(value);
		if (std::holds_alternative<int>(value)) {
			return std::get<int>(value) < 0;
		} else {
			return std::get<float>(value) < 0.0f;
		}
	}
	bool is_zero(const Vm::StackValue& value) {
		assert_is_number(value);
		if (std::holds_alternative<int>(value)) {
			return std::get<int>(value) == 0;
		} else {
			return std::get<float>(value) == 0.0f;
		}
	}
	void get_member(const Vm::StackValue& obj, Vm::StackValue& out, const std::string& index) {
		assert_is_obj(obj);
		auto it = std::get<Vm::Object>(obj).map.find(index);
		if (it == std::get<Vm::Object>(obj).map.end()) {
			throw M2_ERROR("Object does not contain member " + index + ": " + to_string(obj));
		}
		out = it->second;
	}
	void set_member(Vm::StackValue& obj, const Vm::StackValue& value, const std::string& index) {
		assert_is_obj(obj);
		std::get<Vm::Object>(obj).map[index] = value;
	}
}

m2::expected<m2::Vm::ReturnValue> m2::Vm::execute(const std::string &func) const {
	Stack stack;
	try {
		execute_recursively(stack, func);
	} catch (const Error& err) {
		return m2::make_unexpected(err.what());
	}

	return stack.empty()
			? ReturnValue{}
			: std::visit(m2::overloaded{
					[](const auto &value) { return ReturnValue{value}; }
			}, stack.back());
}

void m2::Vm::execute_recursively(Stack& stack, const std::string& func) const {
	auto f = _functions.find(func);
	if (f == _functions.end()) {
		throw M2_ERROR("Function not found: " + func);
	}
	const auto& [commands, labels] = f->second;

	for (InstructionIndex i = 0; i < commands.size(); ++i) {
		try {
			auto inst = commands[i].inst;
			const auto &arg = commands[i].arg;
			switch (inst) {
				case NUMBER:
					if (std::holds_alternative<int>(arg)) {
						stack.emplace_back(std::get<int>(arg));
					} else if (std::holds_alternative<float>(arg)) {
						stack.emplace_back(std::get<float>(arg));
					}
					break;
				case STRING:
					stack.emplace_back(std::get<std::string>(arg));
					break;
				case ADD:
					assert_stack_size(stack, 2);
					add(peek_top(stack), peek_second(stack), second(stack));
					stack.pop_back();
					break;
				case SUBTRACT:
					assert_stack_size(stack, 2);
					subtract(peek_top(stack), peek_second(stack), second(stack));
					stack.pop_back();
					break;
				case MULTIPLY:
					assert_stack_size(stack, 2);
					multiply(peek_top(stack), peek_second(stack), second(stack));
					stack.pop_back();
					break;
				case DIVIDE:
					assert_stack_size(stack, 2);
					divide(peek_top(stack), peek_second(stack), second(stack));
					stack.pop_back();
					break;
				case RETURN:
					return;
				case BINARY_AND:
					assert_stack_size(stack, 2);
					binary_and(peek_top(stack), peek_second(stack), second(stack));
					stack.pop_back();
					break;
				case BINARY_OR:
					assert_stack_size(stack, 2);
					binary_or(peek_top(stack), peek_second(stack), second(stack));
					stack.pop_back();
					break;
				case BINARY_XOR:
					assert_stack_size(stack, 2);
					binary_xor(peek_top(stack), peek_second(stack), second(stack));
					stack.pop_back();
					break;
				case BINARY_COMPLEMENT:
					assert_stack_size(stack, 1);
					binary_complement(peek_top(stack), top(stack));
					break;
				case MODULO:
					assert_stack_size(stack, 2);
					modulo(peek_top(stack), peek_second(stack), second(stack));
					stack.pop_back();
					break;
				case PUSH_EMPTY:
					stack.emplace_back(Object{});
					break;
				case GET_INDIRECT:
					assert_stack_size(stack, 2);
					stack.emplace_back();
					get_indirect(peek_third(stack), peek_second(stack), top(stack));
					break;
				case SET_INDIRECT:
					assert_stack_size(stack, 3);
					set_indirect(peek_second(stack), peek_top(stack), third(stack));
					stack.pop_back();
					break;
				case POP:
					stack.pop_back();
					break;
				case CALL_FUNC:
					execute_recursively(stack, std::get<std::string>(arg));
					break;
				case JUMP_POSITIVE:
					assert_stack_size(stack, 1);
					if (is_positive(peek_top(stack))) {
						i = assert_label(labels, std::get<std::string>(arg)) - 1;
					}
					break;
				case JUMP_NEGATIVE:
					assert_stack_size(stack, 1);
					if (is_negative(peek_top(stack))) {
						i = assert_label(labels, std::get<std::string>(arg)) - 1;
					}
					break;
				case JUMP_ZERO:
					assert_stack_size(stack, 1);
					if (is_zero(peek_top(stack))) {
						i = assert_label(labels, std::get<std::string>(arg)) - 1;
					}
					break;
				case JUMP:
					i = assert_label(labels, std::get<std::string>(arg)) - 1;
					break;
				case GET_MEMBER:
					assert_stack_size(stack, 1);
					stack.emplace_back();
					get_member(peek_second(stack), top(stack), std::get<std::string>(arg));
					break;
				case SET_MEMBER:
					assert_stack_size(stack, 2);
					set_member(second(stack), peek_top(stack), std::get<std::string>(arg));
					break;
				case STACK_GET: {
					assert_stack_size(stack, std::get<int>(arg) + 1);
					auto it = stack.rbegin();
					std::advance(it, std::get<int>(arg));
					stack.emplace_back(*it);
					break;
				}
				case STACK_SET: {
					assert_stack_size(stack, std::get<int>(arg) + 1);
					auto it = stack.rbegin();
					std::advance(it, std::get<int>(arg));
					*it = stack.back();
					stack.pop_back();
					break;
				}
				default:
					throw M2_ERROR("Unknown instruction");
			}
		} catch (const Error& err) {
			throw M2_ERROR(S(err.what()) +  "\n   in function " + func + " instruction " + to_string(i));
		}
	}
}

std::string m2::to_string(const Vm::StackValue& value) {
	return std::visit(m2::overloaded{
			[](const int& i) -> std::string { return to_string(i); },
			[](const float& f) -> std::string { return to_string(f); },
			[](const std::string& s) -> std::string { return s; },
			[](const Vm::Object& o) -> std::string {
				std::stringstream ss;
				ss << "{";
				for (const auto& kv_pair : o.map) {
					ss << kv_pair.first << "=";
					ss << to_string(kv_pair.second) << ", ";
				}
				ss << "}";
				return ss.str();
			},
	}, value);
}
