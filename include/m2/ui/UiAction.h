#pragma once

#include <m2/M2.h>
#include <memory>
#include <variant>

namespace m2 {
	// UiAction types

	class UiActionBase {
	protected:
		UiActionBase() = default;
	public:
		virtual ~UiActionBase() = default;
	};

	class Continue final : public UiActionBase {
		/// If set and true, focus is requested.
		/// If set and false, focus is yielded.
		/// If not set, focus doesn't need to change.
		std::optional<bool> _focus;
	public:
		Continue() = default;
		explicit Continue(bool focus) : UiActionBase(), _focus(focus) {}
		[[nodiscard]] const std::optional<bool>& Focus() const { return _focus; }
	};

	class ReturnBase : public UiActionBase {
		/// Base class of all Return<T> classes.
	/// Enables checking whether an UiAction is of any Return<T> type.
	protected:
		ReturnBase() = default;
	};

	template <typename T = Void>
	class Return : public ReturnBase {
		T _value;
	public:
		explicit Return(T&& value) : ReturnBase(), _value(std::move(value)) {}
		explicit Return(const T& value) : ReturnBase(), _value(value) {}
		[[nodiscard]] const T& Value() const { return _value; }
	};

	class AnyReturnContainer final {
		std::unique_ptr<ReturnBase> _ptr;
	public:
		explicit AnyReturnContainer(std::unique_ptr<ReturnBase> ptr) : _ptr(std::move(ptr)) {}
		[[nodiscard]] const ReturnBase* Get() const { return _ptr.get(); }
	};

	class ClearStack final : public UiActionBase {};

	class Quit final : public UiActionBase {};

	// UiAction container

	class UiAction {
		std::variant<Continue, AnyReturnContainer, ClearStack, Quit> _variant;

	public:
		explicit UiAction(Continue&& c) : _variant(std::move(c)) {}
		explicit UiAction(AnyReturnContainer&& r) : _variant(std::move(r)) {}
		explicit UiAction(ClearStack&& c) : _variant(std::move(c)) {}
		explicit UiAction(Quit&& q) : _variant(std::move(q)) {}

		// Accessors

		[[nodiscard]] bool IsContinue() const { return std::holds_alternative<Continue>(_variant); }
		[[nodiscard]] bool IsReturn() const { return std::holds_alternative<AnyReturnContainer>(_variant); }
		template <typename T> [[nodiscard]] bool IsReturn() const { return IsReturn() && dynamic_cast<const Return<T>*>(std::get<AnyReturnContainer>(_variant).Get()); }
		[[nodiscard]] bool IsVoidReturn() const { return IsReturn<Void>(); }
		[[nodiscard]] bool IsClearStack() const { return std::holds_alternative<ClearStack>(_variant); }
		[[nodiscard]] bool IsQuit() const { return std::holds_alternative<Quit>(_variant); }

		const UiAction& IfContinue(const std::function<void()>& handler) const {
			if (IsContinue()) { handler(); }
			return *this;
		}
		const UiAction& IfContinueWithFocusState(const std::function<void(bool)>& handler) const {
			if (IsContinue()) {
				if (std::get<Continue>(_variant).Focus()) {
					handler(*std::get<Continue>(_variant).Focus());
				}
			}
			return *this;
		}
		const UiAction& IfAnyReturn(const std::function<void(const ReturnBase&)>& handler) const {
			if (IsReturn()) { handler(*std::get<AnyReturnContainer>(_variant).Get()); }
			return *this;
		}
		const UiAction& IfVoidReturn(const std::function<void()>& handler) const {
			if (IsReturn<Void>()) { handler(); }
			return *this;
		}
		template <typename T>
		const UiAction& IfReturn(const std::function<void(const T&)>& handler) const {
			if (IsReturn<T>()) { handler(dynamic_cast<const Return<T>*>(std::get<AnyReturnContainer>(_variant).Get())->Value()); }
			return *this;
		}
		const UiAction& IfClearStack(const std::function<void()>& handler) const {
			if (IsClearStack()) { handler(); }
			return *this;
		}
		const UiAction& IfQuit(const std::function<void()>& handler) const {
			if (IsQuit()) { handler(); }
			return *this;
		}

		// Modifiers

		std::optional<AnyReturnContainer> ExtractAnyReturnContainer() {
			if (IsReturn()) {
				return std::move(std::get<AnyReturnContainer>(_variant));
			}
			return std::nullopt;
		}
	};

	inline UiAction MakeContinueAction() { return UiAction{Continue{}}; }

	inline UiAction MakeContinueAction(bool focus) { return UiAction{Continue{focus}}; }

	inline UiAction MakeReturnAction() { return UiAction{AnyReturnContainer{std::make_unique<Return<>>(Void{})}}; }

	template <typename T>
	UiAction MakeReturnAction(T&& value) { return UiAction{AnyReturnContainer{std::make_unique<Return<T>>(std::forward<T>(value))}}; }
	template <typename T>
	UiAction MakeReturnAction(const T& value) { return UiAction{AnyReturnContainer{std::make_unique<Return<T>>(value)}}; }

	inline UiAction MakeClearStackAction() { return UiAction{ClearStack{}}; }

	inline UiAction MakeQuitAction() { return UiAction{Quit{}}; }

	/// Converts ReturnAction to ContinueAction. Other return types are passed on unmodified.
	inline UiAction ConvertReturnActionToContinue(UiAction&& value) { return value.IsReturn() ? MakeContinueAction() : std::move(value); }
}  // namespace m2
