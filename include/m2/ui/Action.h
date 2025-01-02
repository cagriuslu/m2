#pragma once

#include <m2/M2.h>
#include <memory>

namespace m2::ui {
	// Action types

	class ActionBase {
	protected:
		ActionBase() = default;
	public:
		virtual ~ActionBase() = default;
	};

	class Continue final : public ActionBase {
		/// If set and true, focus is requested.
		/// If set and false, focus is yielded.
		/// If not set, focus doesn't need to change.
		std::optional<bool> _focus;
	public:
		Continue() = default;
		explicit Continue(bool focus) : ActionBase(), _focus(focus) {}
		const std::optional<bool>& Focus() const { return _focus; }
	};

	class ReturnBase : public ActionBase {
		/// Base class of all Return<T> classes.
	/// Enables checking whether an Action is of any Return<T> type.
	protected:
		ReturnBase() = default;
	};

	template <typename T = Void>
	class Return : public ReturnBase {
		T _value;
	public:
		explicit Return(T&& value) : ReturnBase(), _value(std::move(value)) {}
		explicit Return(const T& value) : ReturnBase(), _value(value) {}
		const T& Value() const { return _value; }
	};

	class AnyReturnContainer final {
		std::unique_ptr<ReturnBase> _ptr;
	public:
		explicit AnyReturnContainer(std::unique_ptr<ReturnBase> ptr) : _ptr(std::move(ptr)) {}
		const ReturnBase* Get() const { return _ptr.get(); }
	};

	class ClearStack final : public ActionBase {};

	class Quit final : public ActionBase {};

	// Action container

	class Action {
		std::variant<Continue, AnyReturnContainer, ClearStack, Quit> _variant;

	public:
		explicit Action(Continue&& c) : _variant(std::move(c)) {}
		explicit Action(AnyReturnContainer&& r) : _variant(std::move(r)) {}
		explicit Action(ClearStack&& c) : _variant(std::move(c)) {}
		explicit Action(Quit&& q) : _variant(std::move(q)) {}

		// Accessors

		bool IsContinue() const { return std::holds_alternative<Continue>(_variant); }
		bool IsReturn() const { return std::holds_alternative<AnyReturnContainer>(_variant); }
		template <typename T> bool IsReturn() const { return IsReturn() && dynamic_cast<const Return<T>*>(std::get<AnyReturnContainer>(_variant).Get()); }
		bool IsClearStack() const { return std::holds_alternative<ClearStack>(_variant); }
		bool IsQuit() const { return std::holds_alternative<Quit>(_variant); }

		const Action& IfContinue(const std::function<void()>& handler) const {
			if (IsContinue()) { handler(); }
			return *this;
		}
		const Action& IfContinueWithFocusState(const std::function<void(bool)>& handler) const {
			if (IsContinue()) {
				if (std::get<Continue>(_variant).Focus()) {
					handler(*std::get<Continue>(_variant).Focus());
				}
			}
			return *this;
		}
		const Action& IfAnyReturn(const std::function<void()>& handler) const {
			if (IsReturn()) { handler(); }
			return *this;
		}
		const Action& IfVoidReturn(const std::function<void()>& handler) const {
			if (IsReturn<Void>()) { handler(); }
			return *this;
		}
		template <typename T>
		const Action& IfReturn(const std::function<void(const T&)>& handler) const {
			if (IsReturn<T>()) { handler(dynamic_cast<const Return<T>*>(std::get<AnyReturnContainer>(_variant).Get())->Value()); }
			return *this;
		}
		const Action& IfClearStack(const std::function<void()>& handler) const {
			if (IsClearStack()) { handler(); }
			return *this;
		}
		const Action& IfQuit(const std::function<void()>& handler) const {
			if (IsQuit()) { handler(); }
			return *this;
		}
	};

	inline Action MakeContinueAction() { return Action{Continue{}}; }

	inline Action MakeContinueAction(bool focus) { return Action{Continue{focus}}; }

	inline Action MakeReturnAction() { return Action{AnyReturnContainer{std::make_unique<Return<>>(Void{})}}; }

	template <typename T>
	Action MakeReturnAction(T&& value) { return Action{AnyReturnContainer{std::make_unique<Return<T>>(std::move(value))}}; }
	template <typename T>
	Action MakeReturnAction(const T& value) { return Action{AnyReturnContainer{std::make_unique<Return<T>>(value)}}; }

	inline Action MakeClearStackAction() { return Action{ClearStack{}}; }

	inline Action MakeQuitAction() { return Action{Quit{}}; }
}  // namespace m2::ui
