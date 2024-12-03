#pragma once
#include "../Error.h"
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

	/// Base class of all FSMs. Partition the behavior into states in which a signal might be handled differently from
	/// the others. During or after construction, the child class must call init(initial_state) to initialize the
	/// machine. deinit() must be called before or during the destructor to deinitialize the machine. If you have to
	/// send a signal from one FSM to another, make sure the sender outlives the receiver. Otherwise, use deferral
	/// mechanism to queue the signal for later. Some FSMs (ex. user journeys) are designed to be singletons and have
	/// side effects. Make sure no unwanted temporaries are constructed/destructed while creating these FSMs. Use
	/// std::in_place to store them in std::optional, or std::in_place_type to store them in std::variant.
	template <typename StateEnum, typename SignalT>
	class FsmBase {
		static_assert(std::is_base_of_v<FsmSignalBase,SignalT> == true);
		std::optional<StateEnum> _state;
		float _alarm{NAN};

	protected:
		FsmBase() = default;
	public:
		virtual ~FsmBase() = default;

		/// Must be called by the child class constructor because handle_signal will be called with Enter signal to
		/// enter the initial state.
		void init(StateEnum initial_state) {
			if (_state) {
				throw M2_ERROR("FSM already initialized");
			}
			_state = initial_state;
			signal(SignalT{FsmSignalType::EnterState});
		}

		/// Must be called by the child class destructor because handle_signal will be called with Exit signal to exit
		/// the current state.
		void deinit() {
			if (_state) {
				signal(SignalT{FsmSignalType::ExitState});
				_state.reset();
			}
		}

		/// Arm the alarm
		void arm(float duration_s) {
			if (duration_s < 0.0f) {
				throw M2_ERROR("Invalid alarm duration");
			}
			_alarm = duration_s;
		}
		/// Disarm the alarm
		void disarm() {
			_alarm = NAN;
		}
		/// Report the time passed to the state machine. This function needs to be called periodically for alarm
		/// functionality to work.
		void time(float delta_time_s) {
			if (not std::isnan(_alarm)) {
				if (_alarm -= delta_time_s; _alarm <= 0.0f) {
					_alarm = NAN;
					signal(SignalT{FsmSignalType::Alarm});
				}
			}
		}

		const StateEnum& state() const { return *_state; }

		/// Send a signal to the state machine
		void signal(const SignalT& s) {
			if (auto optional_next_state = handle_signal(s); optional_next_state) {
				handle_signal(SignalT{FsmSignalType::ExitState});
				_state = *optional_next_state;
				signal(SignalT{FsmSignalType::EnterState});
			}
		}

	protected:
		/// If StateEnum is returned, ExitState and EnterState signals will be called.
		/// If std::nullopt is returned, the state stays the same.
		virtual std::optional<StateEnum> handle_signal(const SignalT& s) = 0;
	};

	/// Convenience function that can be called from `handle_signal` with a look up table of state, signal type, and
	/// signal handler.
	template <typename FsmT, typename StateEnum, typename SignalT>
	std::optional<StateEnum> handle_signal_using_handler_map(
			const std::initializer_list<std::tuple<
					StateEnum,
					FsmSignalType,
					// Handler that doesn't expect the signal
					std::optional<StateEnum>(FsmT::*)(),
					// Handler that expects the signal
					std::optional<StateEnum>(FsmT::*)(const SignalT&)>>& handlers,
			FsmT& instance,
			const SignalT& s) {
		// Search for an appropriate handler
		auto current_state = instance.state();
		auto signal_type = s.type();
		auto handler_it = std::ranges::find_if(handlers,
				[current_state,signal_type](const auto& tuple) {
					return (std::get<StateEnum>(tuple) == current_state)
							&& (std::get<FsmSignalType>(tuple) == signal_type);
				});
		if (handler_it != handlers.end()) {
			// Handler found
			auto method_pointer_no_signal = std::get<2>(*handler_it);
			auto method_pointer_signal = std::get<3>(*handler_it);
			if (method_pointer_no_signal) {
				return (instance.*method_pointer_no_signal)();
			} else if (method_pointer_signal) {
				return (instance.*method_pointer_signal)(s);
			}
		}
		return std::nullopt;
	}
}
