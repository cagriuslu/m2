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
	/// Other data can be stored as derived class' member
	template <typename StateEnum, typename SignalT>
	class FsmBase {
		static_assert(std::is_base_of_v<FsmSignalBase,SignalT> == true);
		StateEnum _state;
		float _alarm{NAN};

	protected:
		explicit FsmBase(StateEnum initial_state) : _state(std::move(initial_state)) {}
	public:
		virtual ~FsmBase() = default;

		/// This function should be called after construction because handle_signal virtual function is unavailable
		/// during construction.
		void init() {
			signal(SignalT{FsmSignalType::EnterState});
		}

		/// Arm the alarm
		void arm(float duration_s) {
			if (duration_s < 0.0f) {
				throw M2ERROR("Invalid alarm duration");
			}
			_alarm = duration_s;
		}
		/// Disarm the alarm
		void disarm() {
			_alarm = NAN;
		}
		/// Report the time passed to the state machine
		void time(float delta_time) {
			if (not isnan(_alarm)) {
				if (_alarm -= delta_time; _alarm <= 0.0f) {
					_alarm = NAN;
					signal(SignalT{FsmSignalType::Alarm});
				}
			}
		}

		/// Send a signal to the state machine
		void signal(const SignalT& s) {
			if (auto optional_next_state = handle_signal(s); optional_next_state) {
				handle_signal(SignalT{FsmSignalType::ExitState});
				_state = *optional_next_state;
				signal(SignalT{FsmSignalType::EnterState});
			}
		}

	protected:
		const StateEnum& state() const { return _state; }

		/// If StateEnum is returned, ExitState and EnterState signals will be called.
		/// If std::nullopt is returned, the state stays the same.
		virtual std::optional<StateEnum> handle_signal(const SignalT& s) = 0;
	};
}
