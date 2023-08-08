#pragma once
#include "protobuf/Detail.h"
#include "Exception.h"
#include "Meta.h"
#include <google/protobuf/message.h>
#include <string>
#include <optional>
#include <functional>
#include <type_traits>
#include <utility>
#include <cmath>

namespace m2 {
	enum class FsmSignalType {
		Custom,
		EnterState,
		ExitState,
		Alarm
	};

	/// Base class of all FSM signals
	class FsmSignalBase {
		FsmSignalType _signal_type;

	public:
		inline explicit FsmSignalBase(FsmSignalType signal_type) : _signal_type(signal_type) {}
		[[nodiscard]] inline FsmSignalType type() const { return _signal_type; }
	};

	/// Base class of all FSMs
	/// State should contain information that effects the behavior drastically
	/// Other data can be stored as derived class member
	template <typename State, typename Signal>
	class FsmBase {
		static_assert(std::is_base_of_v<FsmSignalBase,Signal> == true);
		State _state;
		float _alarm{NAN};

	public:
		explicit FsmBase(State initial_state) : _state(std::move(initial_state)) {}
		virtual ~FsmBase() = default;

		/// This function needs to be called after construction because handle_signal virtual function
		/// cannot be called during construction.
		void init() {
			signal(Signal{FsmSignalType::EnterState});
		}
		void arm(float duration_s) {
			if (duration_s < 0.0f) {
				throw M2ERROR("Invalid alarm duration");
			}
			_alarm = duration_s;
		}
		void disarm() {
			_alarm = NAN;
		}
		void signal(const Signal& s) {
			auto optional_next_state = handle_signal(s);
			if (optional_next_state) {
				handle_signal(Signal{FsmSignalType::ExitState});
				_state = *optional_next_state;
				signal(Signal{FsmSignalType::EnterState});
			}
		}
		void time(float delta_time) {
			if (not isnan(_alarm)) {
				_alarm -= delta_time;
				if (_alarm <= 0.0f) {
					_alarm = NAN;
					signal(Signal{FsmSignalType::Alarm});
				}
			}
		}

	protected:
		const State& state() const { return _state; }

		/// If next state is returned, ExitState and EnterState signals will be called
		/// If these signals are not necessary, {} should be returned
		virtual std::optional<State> handle_signal(const Signal& s) = 0;
	};
}
