#include <pinball/Simulation.h>
#include <m2/Error.h>
#include <m2/Math.h>
#include <m2/Meta.h>
#include <m2/Log.h>

using namespace pinball;

namespace {
	float CalculatePrimitiveTemperatureDamage(const float currentMass, const float maxTemperature,
			const float currentTemperature, const float damagePerUnitPerSecond) {
		if (maxTemperature < currentTemperature) {
			const auto temperatureDiff = currentTemperature - maxTemperature;
			const auto damageAmount = temperatureDiff * SIMULATION_TICK_PERIOD_S * damagePerUnitPerSecond;
			return std::clamp(currentMass - damageAmount, 0.0f, currentMass);
		}
		return currentMass;
	}

	float CalculateAnimalTemperatureDamage(const float currentHealth, const float minTemperature,
			const float maxTemperature, const float currentTemperature, const float damagePerUnitPerSecond) {
		if (currentTemperature < minTemperature || maxTemperature < currentTemperature) {
			const auto temperatureDiff = currentTemperature < minTemperature
					? minTemperature - currentTemperature : currentTemperature - maxTemperature;
			const auto damageAmount = temperatureDiff * SIMULATION_TICK_PERIOD_S * damagePerUnitPerSecond;
			return std::clamp(currentHealth - damageAmount, 0.0f, currentHealth);
		}
		return currentHealth;
	}

	float CalculateAnimalHumidityDamage(const float currentHealth, const float currentWaterMass,
			const float damagePerSecond) {
		if (m2::is_zero(currentWaterMass, 0.001f)) {
			const auto damageAmount = damagePerSecond * SIMULATION_TICK_PERIOD_S;
			return std::clamp(currentHealth - damageAmount, 0.0f, currentHealth);
		}
		return currentHealth;
	}

	/// Returns new healthy mass, and new diseased mass
	std::pair<float,float> CalculatePrimitiveDiseaseDeath(const float healthyMass, const float diseasedMass,
			const float diseaseSpreadRatePerSecond, const float diseasedDiePercentagePerSecond) {
		// First, calculate the new infections
		const auto newHealthyMass = std::clamp(
				healthyMass - diseasedMass * diseaseSpreadRatePerSecond * SIMULATION_TICK_PERIOD_S, 0.0f, healthyMass);
		// Then, calculate death due to the disease
		const auto newDiseasedMass = std::clamp(
				diseasedMass * diseasedDiePercentagePerSecond * SIMULATION_TICK_PERIOD_S, 0.0f, diseasedMass);
		return std::make_pair(newHealthyMass, newDiseasedMass);
	}

	/// Returns [0,1] rate
	float CalculateEfficiencyFromMinIdealMax(const float currentValue, const float minValue, const float idealValue,
			const float maxValue) {
		if (currentValue < minValue || maxValue < currentValue) {
			return 0.0f;
		}
		if (currentValue < idealValue) {
			return (currentValue - minValue) / (idealValue - minValue);
		} else {
			return (currentValue - idealValue) / (maxValue - idealValue);
		}
	}

	float CalculateProductionAmount(const float productionRatePerProducerPerSecond, const float producerAmount,
			const float efficiency) {
		return productionRatePerProducerPerSecond * producerAmount * efficiency * SIMULATION_TICK_PERIOD_S;
	}

	float CalculateHungerAfterIncrease(const float currentHunger, const float hungerIncreasePerSecond) {
		return std::clamp(currentHunger + hungerIncreasePerSecond * SIMULATION_TICK_PERIOD_S, currentHunger, 1.0f);
	}

	pb::SimulationState AdvanceTickCount(const pb::SimulationState& currentState) {
		auto nextState = currentState;
		nextState.set_passed_tick_count(currentState.passed_tick_count() + 1);
		return nextState;
	}

	pb::SimulationState AdvanceEnvironment(const pb::SimulationState& currentState,
			const pb::SimulationInputs& inputs) {
		auto nextState = currentState;
		// Add water and heat to the environment
		nextState.set_water_mass(currentState.water_mass() + inputs.extra_water());
		nextState.set_temperature(currentState.temperature() + HEATING_RATE_PER_SECOND * SIMULATION_TICK_PERIOD_S);
		return nextState;
	}

	pb::SimulationState AdvanceBacteriaDeaths(const pb::SimulationState& currentState) {
		auto nextState = currentState;
		// Bacteria die at high temperatures. Healthy and diseased bacteria die at the same rate.
		const auto bacteriaMassAfterTemperatureDeath = CalculatePrimitiveTemperatureDamage(currentState.bacteria_mass(),
				BACTERIA_DEATH_TEMPERATURE, currentState.temperature(),
				BACTERIA_TEMPERATURE_DAMAGE_PER_CELSIUS_PER_SECOND);
		// Bacteria die naturally. Healthy and diseased bacteria die at the same rate.
		const auto bacteriaMassAfterNaturalDeath = bacteriaMassAfterTemperatureDeath
				* (1.0f - BACTERIA_NATURAL_DEATH_RATE_PER_SECOND * SIMULATION_TICK_PERIOD_S);
		// Zombie bacteria kill healthy bacteria
		const auto diseasedBacteriaMassAfterTemperatureDeath = bacteriaMassAfterNaturalDeath
				* currentState.zombie_bacteria_percentage();
		const auto healthyBacteriaMassAfterTemperatureDeath = bacteriaMassAfterNaturalDeath
				- diseasedBacteriaMassAfterTemperatureDeath;
		const auto [healthyBacteriaMassAfterInfection, diseasedBacteriaMassAfterDiseaseDeath] =
				CalculatePrimitiveDiseaseDeath(healthyBacteriaMassAfterTemperatureDeath,
					diseasedBacteriaMassAfterTemperatureDeath, BACTERIA_DISEASE_SPREAD_RATE_PER_SECOND,
					BACTERIA_DISEASE_DIE_PERCENTAGE_PER_SECOND);
		// Bacteria doesn't die any other way. Calculate next bacteria mass. Dead bacteria become waste mass.
		const auto nextBacteriaMass = healthyBacteriaMassAfterInfection + diseasedBacteriaMassAfterDiseaseDeath;
		const auto deadBacteriaMass = currentState.bacteria_mass() - nextBacteriaMass;
		nextState.set_bacteria_mass(nextBacteriaMass);
		nextState.set_zombie_bacteria_percentage(diseasedBacteriaMassAfterDiseaseDeath / nextBacteriaMass);
		nextState.set_waste_mass(currentState.water_mass() + deadBacteriaMass);
		return nextState;
	}

	pb::SimulationState AdvancePlantDeaths(const pb::SimulationState& currentState) {
		auto nextState = currentState;
		// Plants die at high temperatures. Healthy and diseased plants die at the same rate.
		const auto plantMassAfterTemperatureDeath = CalculatePrimitiveTemperatureDamage(currentState.plant_mass(),
				PLANT_MAX_TEMPERATURE, currentState.temperature(),
				PLANT_TEMPERATURE_DAMAGE_PER_CELCIUS_PER_SECOND);
		// Diseased plants kill healthy plants
		const auto diseasedPlantMassAfterTemperatureDeath = plantMassAfterTemperatureDeath
				* currentState.diseased_plant_percentage();
		const auto healthyPlantMassAfterTemperatureDeath = plantMassAfterTemperatureDeath
				- diseasedPlantMassAfterTemperatureDeath;
		const auto [healthyPlantMassAfterInfection, diseasedPlantMassAfterDiseaseDeath] =
				CalculatePrimitiveDiseaseDeath(healthyPlantMassAfterTemperatureDeath,
					diseasedPlantMassAfterTemperatureDeath, PLANT_DISEASE_SPREAD_RATE_PER_SECOND,
					PLANT_DISEASE_DIE_PERCENTAGE_PER_SECOND);
		const auto nextPlantMass = healthyPlantMassAfterInfection + diseasedPlantMassAfterDiseaseDeath;
		const auto deadBacteriaMass = currentState.plant_mass() - nextPlantMass;
		nextState.set_plant_mass(nextPlantMass);
		nextState.set_diseased_plant_percentage(diseasedPlantMassAfterDiseaseDeath / nextPlantMass);
		// Dead plants become waste mass. Eaten plants don't.
		nextState.set_waste_mass(currentState.waste_mass() + deadBacteriaMass);
		return nextState;
	}

	pb::SimulationState AdvanceAnimalDeaths(const pb::SimulationState& currentState,
			const std::function<void(int64_t)>& animalReleaser) {
		auto nextState = currentState;
		// Animals lose health at cold, high temperatures, and zero humidity. Clear and recreate animals.
		nextState.clear_animals();
		for (const auto& animal : currentState.animals()) {
			// Temperature
			const auto minTemperature = animal.type() == pb::Animal_Type_HERBIVORE
					? HERBIVORE_MIN_TEMPERATURE : CARNIVORE_MIN_TEMPERATURE;
			const auto maxTemperature = animal.type() == pb::Animal_Type_HERBIVORE
					? HERBIVORE_MAX_TEMPERATURE : CARNIVORE_MAX_TEMPERATURE;
			const auto temperatureDeathPerUnitPerSecond = animal.type() == pb::Animal_Type_HERBIVORE
					? HERBIVORE_TEMPERATURE_DAMAGE_PER_CELSIUS_PER_SECOND
					: CARNIVORE_TEMPERATURE_DAMAGE_PER_CELSIUS_PER_SECOND;
			const auto healthAfterTemperatureDeath = CalculateAnimalTemperatureDamage(animal.health(), minTemperature,
					maxTemperature, nextState.temperature(), temperatureDeathPerUnitPerSecond);
			// TODO lose health due to hunger
			// Humidity
			const auto humidityDeathPerSecond = animal.type() == pb::Animal_Type_HERBIVORE
					? HERBIVORE_HUMIDITY_DEATH_AMOUNT_PER_SECOND
					: CARNIVORE_HUMIDITY_DEATH_AMOUNT_PER_SECOND;
			const auto healthAfterTemperatureAndHumidityDeath = CalculateAnimalHumidityDamage(
					healthAfterTemperatureDeath, nextState.water_mass(), humidityDeathPerSecond);
			// Reset reproduction count down if any health is lost
			int64_t reproductionCountDownAfterTemperatureDeath;
			if (m2::is_equal(animal.health(), healthAfterTemperatureAndHumidityDeath, 0.001f)) {
				reproductionCountDownAfterTemperatureDeath = animal.reproduction_count_down();
			} else {
				const auto defaultReproductionPeriodS = animal.type() == pb::Animal_Type_HERBIVORE
						? HERBIVORE_DEFAULT_REPRODUCTION_PERIOD_S
						: CARNIVORE_DEFAULT_REPRODUCTION_PERIOD_S;
				reproductionCountDownAfterTemperatureDeath = m2::iround(defaultReproductionPeriodS
						* SIMULATION_TICKS_PER_SECOND);
			}
			// Delete the animal if the health is zero
			if (healthAfterTemperatureAndHumidityDeath < 0.001f) {
				animalReleaser(animal.id());
				// Dead animals become waste mass. Eaten animals don't.
				nextState.set_waste_mass(nextState.waste_mass() + animal.mass());
			} else {
				pb::Animal animalAfterDeath = animal;
				animalAfterDeath.set_health(healthAfterTemperatureAndHumidityDeath);
				animalAfterDeath.set_reproduction_count_down(reproductionCountDownAfterTemperatureDeath);
				nextState.add_animals()->CopyFrom(animal);
			}
		}
		return nextState;
	}

	pb::SimulationState AdvanceDecomposition(const pb::SimulationState& currentState, const pb::SimulationInputs& inputs) {
		auto nextState = currentState;
		// Decomposition requires darkness, and appropriate temperature
		if (not inputs.light() && BACTERIA_DECOMPOSITION_MIN_TEMPERATURE <= currentState.temperature()
				&& currentState.temperature() <= BACTERIA_DECOMPOSITION_MAX_TEMPERATURE) {
			// Decomposition speed of a single bacteria
			const float decompositionEfficiency = CalculateEfficiencyFromMinIdealMax(currentState.temperature(),
					BACTERIA_DECOMPOSITION_MIN_TEMPERATURE,
					BACTERIA_DECOMPOSITION_FASTEST_TEMPERATURE,
					BACTERIA_DECOMPOSITION_MAX_TEMPERATURE);
			// Only healthy bacteria do decomposition.
			const auto healthyBacteriaMass = currentState.bacteria_mass() * (1.0f - currentState.zombie_bacteria_percentage());
			// Find rate limit due to waste availability
			const auto idealRequiredWasteMass = CalculateProductionAmount(BACTERIA_DECOMPOSITION_WASTE_USE_PER_SECOND, healthyBacteriaMass, decompositionEfficiency);
			const auto availableWasteMass = std::min(currentState.waste_mass(), idealRequiredWasteMass);
			const auto rateLimitDueToWaste = availableWasteMass / idealRequiredWasteMass;
			// Find rate limit due to water availability
			const auto idealRequiredWaterMass = idealRequiredWasteMass * BACTERIA_DECOMPOSITION_WATER_USE_RATE;
			const auto availableWaterMass = std::min(currentState.water_mass(), idealRequiredWaterMass);
			const auto rateLimitDueToWater = availableWaterMass / idealRequiredWaterMass;
			// Find actual rate limit
			const auto rateLimit = std::min(rateLimitDueToWaste, rateLimitDueToWater);
			const auto actualRequiredWasteMass = idealRequiredWasteMass * rateLimit;
			const auto actualRequiredWaterMass = idealRequiredWaterMass * rateLimit;
			// Decompose
			const auto producedNutrientMass = actualRequiredWasteMass * BACTERIA_DECOMPOSITION_NUTRIENT_PRODUCTION_RATE;
			const auto reproductionMass = actualRequiredWasteMass * BACTERIA_REPRODUCTION_RATE;
			const auto newZombieBacteriaPercentage = currentState.bacteria_mass() * currentState.zombie_bacteria_percentage()
					/ (currentState.bacteria_mass() + reproductionMass);
			nextState.set_waste_mass(currentState.waste_mass() - actualRequiredWasteMass);
			nextState.set_nutrient_mass(currentState.nutrient_mass() + producedNutrientMass);
			nextState.set_water_mass(currentState.water_mass() - actualRequiredWaterMass);
			nextState.set_bacteria_mass(currentState.bacteria_mass() + reproductionMass);
			nextState.set_zombie_bacteria_percentage(newZombieBacteriaPercentage);
		}
		return nextState;
	}

	pb::SimulationState AdvancePlantGrowth(const pb::SimulationState& currentState, const pb::SimulationInputs& inputs) {
		auto nextState = currentState;
		// Plant growth requires light, and appropriate temperature
		if (inputs.light() && PLANT_GROWTH_MIN_TEMPERATURE <= currentState.temperature()
				&& currentState.temperature() <= PLANT_GROWTH_MAX_TEMPERATURE) {
			const float growthEfficiency = CalculateEfficiencyFromMinIdealMax(currentState.temperature(),
					PLANT_GROWTH_MIN_TEMPERATURE,
					PLANT_GROWTH_FASTEST_TEMPERATURE,
					PLANT_GROWTH_MAX_TEMPERATURE);
			// Only healthy plants grow
			const auto healthyPlantMass = currentState.plant_mass() * (1.0f - currentState.diseased_plant_percentage());
			// Find rate limit due to nutrient availability
			const auto idealGrowthMass = CalculateProductionAmount(PLANT_GROWTH_PRODUCTION_RATE_PER_SECOND, healthyPlantMass, growthEfficiency);
			const auto idealRequiredNutrientMass = idealGrowthMass * PLANT_GROWTH_NUTRIENT_USE_RATE;
			const auto availableNutrientMass = std::min(currentState.nutrient_mass(), idealRequiredNutrientMass);
			const auto rateLimitDueToNutrient = availableNutrientMass / idealRequiredNutrientMass;
			// Find rate limit due to water availability
			const auto idealRequiredWaterMass = idealGrowthMass * PLANT_GROWTH_WATER_USE_RATE;
			const auto availableWaterMass = std::min(currentState.water_mass(), idealRequiredWaterMass);
			const auto rateLimitDueToWater = availableWaterMass / idealRequiredWaterMass;
			// Find actual rate limit
			const auto rateLimit = std::min(rateLimitDueToNutrient, rateLimitDueToWater);
			const auto actualRequiredNutrientMass = idealRequiredNutrientMass * rateLimit;
			const auto actualRequiredWaterMass = idealRequiredWaterMass * rateLimit;
			const auto actualGrowthMass = actualRequiredWaterMass / PLANT_GROWTH_WATER_USE_RATE;
			// Grow
			const auto newDiseasedPlantPercentage = currentState.plant_mass() * currentState.diseased_plant_percentage()
					/ (currentState.plant_mass() + actualGrowthMass);
			nextState.set_plant_mass(currentState.plant_mass() + actualGrowthMass);
			nextState.set_diseased_plant_percentage(newDiseasedPlantPercentage);
			nextState.set_nutrient_mass(currentState.nutrient_mass() - actualRequiredNutrientMass);
			nextState.set_water_mass(currentState.water_mass() - actualRequiredWaterMass);
		}
		return nextState;
	}

	pb::SimulationState AdvanceHerbivoreGrowth(const pb::SimulationState& currentState, const std::function<int64_t(pb::Animal_Type)>& animalAllocator) {
		auto nextState = currentState;
		for (auto& animal : *nextState.mutable_animals()) {
			if (animal.type() != pb::Animal::HERBIVORE) {
				continue;
			}
			// First, increase hunger
			animal.set_hunger(
					CalculateHungerAfterIncrease(animal.hunger(), HERBIVORE_HUNGER_PERCENTAGE_GAINED_PER_SECOND));
			// Then, feed
			const auto idealHungerPercentageToFulfill = std::max(animal.hunger(),
					HERBIVORE_HUNGER_PERCENTAGE_FULFILLED_PER_SECOND * SIMULATION_TICK_PERIOD_S);

			const auto idealRequiredPlantMass = idealHungerPercentageToFulfill * animal.mass() * HERBIVORE_PLANT_REQUIRED_FOR_FULL_HUNGER_FULFILLMENT;
			const auto availablePlantMassForFeeding = std::min(nextState.plant_mass(), idealRequiredPlantMass);
			const auto feedRateLimitDueToPlant = availablePlantMassForFeeding / idealRequiredPlantMass;

			const auto idealRequiredWaterMass = idealHungerPercentageToFulfill * animal.mass() * HERBIVORE_WATER_REQUIRED_FOR_FULL_HUNGER_FULFILLMENT;
			const auto availableWaterMassForFeeding = std::min(nextState.water_mass(), idealRequiredWaterMass);
			const auto feedRateLimitDueToWater = availableWaterMassForFeeding / idealRequiredWaterMass;

			const auto feedRateLimit = std::min(feedRateLimitDueToPlant, feedRateLimitDueToWater);
			const auto actualRequiredPlantMass = idealRequiredPlantMass * feedRateLimit;
			const auto actualRequiredWaterMass = idealRequiredWaterMass * feedRateLimit;
			const auto actualHungerPercentageToFulfill = idealHungerPercentageToFulfill * feedRateLimit;

			animal.set_hunger(animal.hunger() - actualHungerPercentageToFulfill);
			nextState.set_plant_mass(nextState.water_mass() - actualRequiredPlantMass);
			nextState.set_plant_mass(nextState.plant_mass() - actualRequiredWaterMass);

			// Check if reproduction count down needs to be reset
			if (HERBIVORE_REPRODUCTION_HUNGER_THRESHOLD < animal.hunger()) {
				animal.set_reproduction_count_down(
						m2::iround(HERBIVORE_DEFAULT_REPRODUCTION_PERIOD_S * SIMULATION_TICKS_PER_SECOND));
			} else {
				const auto nextReproductionCountDown = animal.reproduction_count_down() - 1;
				if (nextReproductionCountDown == 0) {
					// TODO Reproduce
					animal.set_reproduction_count_down(
							m2::iround(HERBIVORE_DEFAULT_REPRODUCTION_PERIOD_S * SIMULATION_TICKS_PER_SECOND));
				} else {
					animal.set_reproduction_count_down(nextReproductionCountDown);
				}
			}
		}
		return nextState;
	}

	pb::SimulationState AdvanceCarnivoreGrowth(const pb::SimulationState& currentState, const std::function<int64_t(pb::Animal_Type)>& animalAllocator, const std::function<void(int64_t)>& animalReleaser) {
		auto nextState = currentState;
		for (auto& animal : *nextState.mutable_animals()) {
			if (animal.type() != pb::Animal::CARNIVORE) {
				continue;
			}
			// First, increase hunger
			animal.set_hunger(
					CalculateHungerAfterIncrease(animal.hunger(), CARNIVORE_HUNGER_PERCENTAGE_GAINED_PER_SECOND));
			// Check if hunger threshold is passed
			if (CARNIVORE_HUNTING_HUNGER_THRESHOLD < animal.hunger()) {
				if (animal.hunting_count_down() == 0) {
					// Reinit hunting count down
					const auto minTicks = m2::iround(CARNIVORE_HUNTING_PERIOD_S * SIMULATION_TICKS_PER_SECOND
							- CARNIVORE_HUNTING_PERIOD_VARIANCE_S * SIMULATION_TICKS_PER_SECOND);
					const auto maxTicks = m2::iround(CARNIVORE_HUNTING_PERIOD_S * SIMULATION_TICKS_PER_SECOND
							+ CARNIVORE_HUNTING_PERIOD_VARIANCE_S * SIMULATION_TICKS_PER_SECOND);
					const auto ticks = m2::UniformRandom(minTicks, maxTicks);
					animal.set_hunting_count_down(ticks);
				} else if (animal.hunting_count_down() == 1) {
					// TODO hunt if there's an animal
				} else {
					// Decrement hunting count down
					animal.set_hunting_count_down(animal.hunting_count_down() - 1);
				}
			} else {
				// Clear hunting period
				animal.clear_hunting_count_down();
			}

			// Check if reproduction count down needs to be reset
			if (CARNIVORE_REPRODUCTION_HUNGER_THRESHOLD < animal.hunger()) {
				animal.set_reproduction_count_down(
						m2::iround(CARNIVORE_DEFAULT_REPRODUCTION_PERIOD_S * SIMULATION_TICKS_PER_SECOND));
			} else {
				const auto nextReproductionCountDown = animal.reproduction_count_down() - 1;
				if (nextReproductionCountDown == 0) {
					// TODO Reproduce
					animal.set_reproduction_count_down(
							m2::iround(CARNIVORE_DEFAULT_REPRODUCTION_PERIOD_S * SIMULATION_TICKS_PER_SECOND));
				} else {
					animal.set_reproduction_count_down(nextReproductionCountDown);
				}
			}
		}
		return nextState;
	}
}

pb::SimulationState pinball::InitialSimulationState(const std::function<int64_t(pb::Animal_Type)>& animalAllocator) {
	pb::SimulationState state;
	state.set_bacteria_mass(BACTERIA_LIMIT_KG / 8.0f);
	state.set_plant_mass(PLANT_LIMIT_KG / 50.0f);
	for (int i = 0; i < 6; ++i) {
		auto* animal = state.add_animals();
		animal->set_id(animalAllocator(pb::Animal_Type_HERBIVORE));
		animal->set_type(pb::Animal_Type_HERBIVORE);
		animal->set_mass(0.35f);
		animal->set_health(1.0f);
		animal->set_hunger(0.25f);
		animal->set_reproduction_count_down(150);
	}
	state.set_temperature(25.0f);
	state.set_waste_mass(4.0f);
	state.set_nutrient_mass(0.25f);
	state.set_water_mass(15.0f);
	return state;
}

pb::SimulationState pinball::AdvanceSimulation(const pb::SimulationState& currentState, const pb::SimulationInputs& inputs,
	const std::function<int64_t(pb::Animal_Type)>& animalAllocator, const std::function<void(int64_t)>& animalReleaser) {
	LOG_DEBUG("Advancing simulation");
	const auto stateAfterTickCount = AdvanceTickCount(currentState);
	const auto stateAfterEnvironment = AdvanceEnvironment(stateAfterTickCount, inputs);
	const auto stateAfterBacteriaDeaths = AdvanceBacteriaDeaths(stateAfterEnvironment);
	const auto stateAfterPlantDeaths = AdvancePlantDeaths(stateAfterBacteriaDeaths);
	const auto stateAfterAnimalDeaths = AdvanceAnimalDeaths(stateAfterPlantDeaths, animalReleaser);
	const auto stateAfterDecomposition = AdvanceDecomposition(stateAfterAnimalDeaths, inputs);
	const auto stateAfterPlantGrowth = AdvancePlantGrowth(stateAfterDecomposition, inputs);
	const auto stateAfterHerbivoreGrowth = AdvanceHerbivoreGrowth(stateAfterPlantGrowth, animalAllocator);
	const auto stateAfterCarnivoreGrowth = AdvanceCarnivoreGrowth(stateAfterHerbivoreGrowth, animalAllocator, animalReleaser);
	return stateAfterCarnivoreGrowth;
}
