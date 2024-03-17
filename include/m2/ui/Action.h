#pragma once

#include <m2/M2.h>
#include <memory>

namespace m2::ui {
	class ActionBase {
	protected:
		ActionBase() = default;

	public:
		virtual ~ActionBase() = default;
	};

	struct Continue : public ActionBase {
		/// If set and true, focus is requested.
		/// If set and false, focus is yielded.
		/// If not set, focus doesn't need to change.
		std::optional<bool> focus;

		Continue() = default;

		explicit Continue(bool focus) : ActionBase(), focus(focus) {}
	};

	class ReturnBase : public ActionBase {
		/// Base class of all Return<T> classes.
		/// Enables checking whether an Action is of any Return<T> type.
	protected:
		ReturnBase() = default;
	};

	template <typename T = m2::Void>
	struct Return : public ReturnBase {
		T value;

		explicit Return(T&& value) : ReturnBase(), value(std::move(value)) {}
	};

	struct ClearStack : public ActionBase {};

	struct Quit : public ActionBase {};

	class Action {
		std::unique_ptr<ActionBase> _ptr;

	public:
		explicit Action(std::unique_ptr<ActionBase> ptr) : _ptr(std::move(ptr)) {}

		[[nodiscard]] bool is_continue() const { return dynamic_cast<const Continue*>(_ptr.get()); }

		const Action& if_continue(const std::function<void()>& handler) const {
			if (is_continue()) { handler(); }
			return *this;
		}

		const Action& if_continue_with_focus_state(const std::function<void(bool)>& handler) const {
			if (const auto* continue_action = dynamic_cast<const Continue*>(_ptr.get())) {
				if (continue_action->focus) {
					handler(*continue_action->focus);
				}
			}
			return *this;
		}

		[[nodiscard]] bool is_return() const { return dynamic_cast<const ReturnBase*>(_ptr.get()); }

		template <typename T>
		[[nodiscard]] bool is_return() const { return dynamic_cast<const Return<T>*>(_ptr.get()); }

		const Action& if_any_return(const std::function<void()>& handler) const {
			if (is_return()) { handler(); }
			return *this;
		}

		const Action& if_void_return(const std::function<void()>& handler) const {
			if (const auto* return_action = dynamic_cast<const Return<m2::Void>*>(_ptr.get())) { handler(); }
			return *this;
		}

		template <typename T>
		const Action& if_return(const std::function<void(const T&)>& handler) const {
			if (const auto* return_action = dynamic_cast<const Return<T>*>(_ptr.get())) { handler(return_action->value); }
			return *this;
		}

		[[nodiscard]] bool is_clear_stack() const { return dynamic_cast<const ClearStack*>(_ptr.get()); }

		const Action& if_clear_stack(const std::function<void()>& handler) const {
			if (is_clear_stack()) { handler(); }
			return *this;
		}

		[[nodiscard]] bool is_quit() const { return dynamic_cast<const Quit*>(_ptr.get()); }

		const Action& if_quit(const std::function<void()>& handler) const {
			if (is_quit()) { handler(); }
			return *this;
		}
	};

	inline Action make_continue_action() { return Action{std::make_unique<Continue>()}; }

	inline Action make_continue_action(bool focus) { return Action{std::make_unique<Continue>(focus)}; }

	inline Action make_return_action() { return Action{std::make_unique<Return<Void>>(Void{})}; }

	template <typename T>
	Action make_return_action(T value) { return Action{std::make_unique<Return<T>>(std::forward<T>(value))}; }

	inline Action make_clear_stack_action() { return Action{std::make_unique<ClearStack>()}; }

	inline Action make_quit_action() { return Action{std::make_unique<Quit>()}; }
}  // namespace m2::ui
