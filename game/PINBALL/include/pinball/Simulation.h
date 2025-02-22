#pragma once
#include <Simulation.pb.h>

namespace pinball {
	constexpr auto SIMULATION_TICKS_PER_SECOND = 10.0f;
	constexpr auto SIMULATION_TICK_PERIOD_S = 1.0f / SIMULATION_TICKS_PER_SECOND;

	/// Rate of change of temperate if heating is enabled
	constexpr auto SIMULATION_HEATING_RATE_PER_SECOND = 0.2f;

	/// Amount of bacteria that can exist without developing diseases.
	constexpr auto SIMULATION_BACTERIA_LIMIT_KG = 1.0f;
	/// Maximum temperature bacteria can live without dying.
	constexpr auto SIMULATION_BACTERIA_MAX_TEMPERATURE = 42.0f;
	/// Exponent applied to temperature above the maximum temperature.
	constexpr auto SIMULATION_BACTERIA_TEMPERATURE_DEATH_EXPONENT = 0.5f;
	/// Amount of bacteria killed per second if the temperature is 1C above max.
	constexpr auto SIMULATION_BACTERIA_TEMPERATURE_DEATH_AMOUNT_PER_UNIT_PER_SECOND = 0.0075f;
	/// Percentage of bacteria killed per second naturally.
	constexpr auto SIMULATION_BACTERIA_NATURAL_DEATH_RATE_PER_SECOND = 0.005f;
	/// Amount of healthy bacteria infected with diseased per 1 unit of diseased bacteria per second.
	constexpr auto SIMULATION_BACTERIA_DISEASE_SPREAD_RATE_PER_SECOND = 1.5f;
	/// Percentage of diseased bacteria dying per second
	constexpr auto SIMULATION_BACTERIA_DISEASE_DIE_PERCENTAGE_PER_SECOND = 0.02f;

	/// Amount of plant that can exist without developing diseases.
	constexpr auto SIMULATION_PLANT_LIMIT_KG = 1000.0f;
	/// Maximum temperature plants can live without dying.
	constexpr auto SIMULATION_PLANT_MAX_TEMPERATURE = 48.0f;
	/// Exponent applied to temperature above the maximum temperature.
	constexpr auto SIMULATION_PLANT_TEMPERATURE_DEATH_EXPONENT = 0.65f;
	/// Amount of plant killed per second if the temperature is 1C above max.
	constexpr auto SIMULATION_PLANT_TEMPERATURE_DEATH_AMOUNT_PER_UNIT_PER_SECOND = 1.0f;
	/// Amount of healthy plant infected with diseased per 1 unit of diseased plant per second.
	constexpr auto SIMULATION_PLANT_DISEASE_SPREAD_RATE_PER_SECOND = 1.25f;
	/// Percentage of diseased plany dying per second
	constexpr auto SIMULATION_PLANT_DISEASE_DIE_PERCENTAGE_PER_SECOND = 0.01f;

	/// Minimum temperature herbivores can live without losing health.
	constexpr auto SIMULATION_HERBIVORE_MIN_TEMPERATURE = 8.0f;
	/// Maximum temperature herbivores can live without losing health.
	constexpr auto SIMULATION_HERBIVORE_MAX_TEMPERATURE = 51.0f;
	/// Exponent applied to temperature above the maximum temperature.
	constexpr auto SIMULATION_HERBIVORE_TEMPERATURE_DEATH_EXPONENT = 1.0f;
	/// Amount of herbivore health lost per second if the temperature is 1C above max.
	constexpr auto SIMULATION_HERBIVORE_TEMPERATURE_DEATH_AMOUNT_PER_UNIT_PER_SECOND = 0.05f;
	/// Amount of herbivore health lost per second if the humidity is zero.
	constexpr auto SIMULATION_HERBIVORE_HUMIDITY_DEATH_AMOUNT_PER_SECOND = 0.02f;
	/// Default duration after which a herbivore reproduces.
	constexpr auto SIMULATION_HERBIVORE_DEFAULT_REPRODUCTION_PERIOD_S = 25.0f;

	/// Minimum temperature carnivores can live without losing health.
	constexpr auto SIMULATION_CARNIVORE_MIN_TEMPERATURE = 12.0f;
	/// Maximum temperature carnivores can live without losing health.
	constexpr auto SIMULATION_CARNIVORE_MAX_TEMPERATURE = 55.0f;
	/// Exponent applied to temperature above the maximum temperature.
	constexpr auto SIMULATION_CARNIVORE_TEMPERATURE_DEATH_EXPONENT = 1.0f;
	/// Amount of carnivore health lost per second if the temperature is 1C above max.
	constexpr auto SIMULATION_CARNIVORE_TEMPERATURE_DEATH_AMOUNT_PER_UNIT_PER_SECOND = 0.075f;
	/// Amount of carnivore health lost per second if the humidity is zero.
	constexpr auto SIMULATION_CARNIVORE_HUMIDITY_DEATH_AMOUNT_PER_SECOND = 0.01f;
	/// Default duration after which a carnivore reproduces.
	constexpr auto SIMULATION_CARNIVORE_DEFAULT_REPRODUCTION_PERIOD_S = 55.0f;

	/// Advances the simulation by one tick.
	pb::SimulationState AdvanceSimulation(const pb::SimulationState& currentState, const pb::SimulationInputs& inputs,
			const std::function<int64_t()>& animalAllocator, const std::function<void(int64_t)>& animalReleaser);

	struct DerivedSimulationState {
		/// Ratio of current decomposition to max decomposition that could be achieved by all existing bacteria.
		float decompositionRate; // [0,1]
		/// Ratio of current photosynthesis to max photosynthesis that could be achieved by all existing plants.
		float photosynthesisRate; // [0,1]
		/// Ratio of plant mass eaten by herbivores to max plant mass that could be eaten by all existing herbivores.
		float herbivoreSatisfaction; // [0,1]
		/// Ratio of herbivore count eaten by carnivores to max herbivore count that could be eaten by all existing carnivores.
		float carnivoreSatisfaction; // [0,1]
	};
	DerivedSimulationState DeriveSimulationState(const pb::SimulationState& currentState);
}
