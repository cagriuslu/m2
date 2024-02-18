#pragma once
#include <memory>

namespace m2::ui {
	class ActionBase {
	   protected:
		ActionBase() = default;

	   public:
		virtual ~ActionBase() = default;
	};
	using Action = std::unique_ptr<ActionBase>;

	struct Continue : public ActionBase {
		/// If set and true, focus is requested.
		/// If set and false, focus is yielded.
		/// If not set, focus doesn't need to change.
		std::optional<bool> focus;

		Continue() = default;
		Continue(bool focus) : ActionBase(), focus(focus) {}
	};
	inline Action make_continue_action() { return std::make_unique<Continue>(); }
	inline Action make_continue_action(bool focus) { return std::make_unique<Continue>(focus); }
	inline bool is_continue(const Action& action) { return dynamic_cast<const Continue*>(action.get()); }
	inline const Continue& as_continue(const Action& action) { return dynamic_cast<const Continue&>(*action); }

	class ReturnBase : public ActionBase {
		/// This class is introduces as a base class of all Return<T> classes.
		/// This allows us to check whether a given Action is of Return<T> type or not.
	   protected:
		ReturnBase() = default;
	};
	template <typename T>
	struct Return : public ReturnBase {
		std::optional<T> value;

		/// Without value
		Return() = default;
		/// With value
		explicit Return(T&& value) : ReturnBase(), value(std::move(value)) {}
	};
	template <typename T>
	Action make_return_action() {
		return std::make_unique<Return<T>>();
	}
	template <typename T>
	Action make_return_action(T&& value) {
		return std::make_unique<Return<T>>(std::forward<T>(value));
	}
	inline bool is_return(const Action& action) { return dynamic_cast<const ReturnBase*>(action.get()); }
	template <typename T>
	inline const Return<T>& as_return(const Action& action) {
		return dynamic_cast<const Return<T>&>(*action);
	}

	struct ClearStack : public ActionBase {};
	inline Action make_clear_stack_action() { return std::make_unique<ClearStack>(); }
	inline bool is_clear_stack(const Action& action) { return dynamic_cast<const ClearStack*>(action.get()); }

	struct Quit : public ActionBase {};
	inline Action make_quit_action() { return std::make_unique<Quit>(); }
	inline bool is_quit(const Action& action) { return dynamic_cast<const Quit*>(action.get()); }
}  // namespace m2::ui
