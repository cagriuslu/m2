#pragma once
#include <Simulation.pb.h>
#include <m2/Meta.h>

namespace pinball {
	constexpr auto SIMULATION_TICKS_PER_SECOND = 10.0f;
	constexpr auto SIMULATION_TICK_PERIOD_S = 1.0f / SIMULATION_TICKS_PER_SECOND;
	const auto SIMULATION_TICK_PERIOD_TICKS = m2::iround(SIMULATION_TICK_PERIOD_S * 1000.0f);

	/// Rate of change of temperate if heating is enabled
	constexpr auto SIMULATION_HEATING_RATE_PER_SECOND = 0.2f;

	/// Amount of bacteria that can exist without developing diseases.
	constexpr auto SIMULATION_BACTERIA_LIMIT_KG = 1.0f;
	/// Maximum temperature bacteria can live without dying.
	constexpr auto SIMULATION_BACTERIA_DEATH_TEMPERATURE = 42.0f;
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
	/// Minimum decomposition temperature
	constexpr auto SIMULATION_BACTERIA_DECOMPOSITION_MIN_TEMPERATURE = 20.0f;
	/// Fastest decomposition temperature
	constexpr auto SIMULATION_BACTERIA_DECOMPOSITION_FASTEST_TEMPERATURE = 32.0f;
	/// Maximum decomposition temperature
	constexpr auto SIMULATION_BACTERIA_DECOMPOSITION_MAX_TEMPERATURE = 38.0f;
	/// Amount of waste decomposited per 1KG of bacteria per second at the fastest decomposition temperature
	constexpr auto SIMULATION_BACTERIA_DECOMPOSITION_WASTE_USE_PER_SECOND = 0.25f;
	/// Amount of nutrient produced per 1KG of waste decomposited
	constexpr auto SIMULATION_BACTERIA_DECOMPOSITION_NUTRIENT_PRODUCTION_RATE = 0.05f;
	/// Amount of water used per 1KG of waste decomposited
	constexpr auto SIMULATION_BACTERIA_DECOMPOSITION_WATER_USE_RATE = 0.5f;
	/// Amount of bacteria reproduction per 1KG of waste used
	constexpr auto SIMULATION_BACTERIA_REPRODUCTION_RATE = 0.05f;

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
	/// Percentage of diseased plant dying per second
	constexpr auto SIMULATION_PLANT_DISEASE_DIE_PERCENTAGE_PER_SECOND = 0.01f;
	/// Minimum growth temperature
	constexpr auto SIMULATION_PLANT_GROWTH_MIN_TEMPERATURE = 14.0f;
	/// Fastest growth temperature
	constexpr auto SIMULATION_PLANT_GROWTH_FASTEST_TEMPERATURE = 28.0f;
	/// Maximum growth temperature
	constexpr auto SIMULATION_PLANT_GROWTH_MAX_TEMPERATURE = 36.0f;
	/// Amount of plant growth per 1KG of plant per second at the fastest growth temperature
	constexpr auto SIMULATION_PLANT_GROWTH_PRODUCTION_RATE_PER_SECOND = 0.1f;
	/// Amount of nutrient used per 1KG of plant growth
	constexpr auto SIMULATION_PLANT_GROWTH_NUTRIENT_USE_RATE = 0.1f;
	/// Amount of water used per 1KG of plant growth
	constexpr auto SIMULATION_PLANT_GROWTH_WATER_USE_RATE = 0.5f;

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
	/// Percentage of hunger increase per second
	constexpr auto SIMULATION_HERBIVORE_HUNGER_PERCENTAGE_GAINED_PER_SECOND = 0.04f;
	/// Percentage of hunger fulfilled every second if there's enough food in the environment
	constexpr auto SIMULATION_HERBIVORE_HUNGER_PERCENTAGE_FULFILLED_PER_SECOND = 0.1f;
	/// Ratio of required plant amount to animal's mass to fulfill a full hunger
	constexpr auto SIMULATION_HERBIVORE_PLANT_REQUIRED_FOR_FULL_HUNGER_FULFILLMENT = 1.2f;
	/// Ratio of required water amount to animal's mass to fulfill a full hunger
	constexpr auto SIMULATION_HERBIVORE_WATER_REQUIRED_FOR_FULL_HUNGER_FULFILLMENT = 0.5f;
	/// Maximum hunger allowed for enabling reproduction timer
	constexpr auto SIMULATION_HERBIVORE_REPRODUCTION_HUNGER_THRESHOLD = 0.5f;
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
	/// Percentage of hunger increase per second
	constexpr auto SIMULATION_CARNIVORE_HUNGER_PERCENTAGE_GAINED_PER_SECOND = 0.04f;
	/// Minimum hunger for carnivore to hunt
	constexpr auto SIMULATION_CARNIVORE_HUNTING_HUNGER_THRESHOLD = 0.65f;
	/// Hunting period for carnivore
	constexpr auto SIMULATION_CARNIVORE_HUNTING_PERIOD_S = 9.0f;
	/// Hunting period variance for carnivore
	constexpr auto SIMULATION_CARNIVORE_HUNTING_PERIOD_VARIANCE_S = 2.0f;
	/// Ratio of required herbivore mass to animal's mass to fulfill a full hunger
	constexpr auto SIMULATION_CARNIVORE_HERBIVORE_MASS_REQUIRED_FOR_FULL_HUNGER_FULFILLMENT = 0.8f;
	/// Ratio of required water amount to animal's mass to fulfill a full hunger
	constexpr auto SIMULATION_CARNIVORE_WATER_REQUIRED_FOR_FULL_HUNGER_FULFILLMENT = 0.7f;
	/// Maximum hunger allowed for enabling reproduction timer
	constexpr auto SIMULATION_CARNIVORE_REPRODUCTION_HUNGER_THRESHOLD = 0.5f;
	/// Default duration after which a carnivore reproduces.
	constexpr auto SIMULATION_CARNIVORE_DEFAULT_REPRODUCTION_PERIOD_S = 55.0f;

	pb::SimulationState InitialSimulationState(const std::function<int64_t(pb::Animal_Type)>& animalAllocator);

	/// Advances the simulation by one tick.
	pb::SimulationState AdvanceSimulation(const pb::SimulationState& currentState, const pb::SimulationInputs& inputs,
			const std::function<int64_t(pb::Animal_Type)>& animalAllocator, const std::function<void(int64_t)>& animalReleaser);

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
