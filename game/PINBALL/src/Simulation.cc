#include <pinball/Simulation.h>
#include <m2/Error.h>
#include <m2/Math.h>
#include <m2/Meta.h>
#include <m2/Log.h>

using namespace pinball;

namespace {
	float CalculatePrimitiveTemperatureDeath(const float currentMass, const float maxTemperature,
			const float currentTemperature, const float deathExponent, const float deathPerUnitPerSecond) {
		if (maxTemperature < currentTemperature) {
			const auto temperatureDiff = currentTemperature - maxTemperature;
			const auto deathFactor = powf(temperatureDiff, deathExponent);
			const auto killAmount = deathFactor * SIMULATION_TICK_PERIOD_S * deathPerUnitPerSecond;
			return std::clamp(currentMass - killAmount, 0.0f, currentMass);
		}
		return currentMass;
	}

	float CalculateAnimalTemperatureDeath(const float currentHealth, const float minTemperature,
			const float maxTemperature, const float currentTemperature, const float deathExponent,
			const float deathPerUnitPerSecond) {
		if (currentTemperature < minTemperature || maxTemperature < currentTemperature) {
			const auto temperatureDiff = currentTemperature < minTemperature ? minTemperature - currentTemperature : currentTemperature - maxTemperature;
			const auto deathFactor = powf(temperatureDiff, deathExponent);
			const auto killAmount = deathFactor * SIMULATION_TICK_PERIOD_S * deathPerUnitPerSecond;
			return std::clamp(currentHealth - killAmount, 0.0f, currentHealth);
		}
		return currentHealth;
	}

	float CalculateAnimalHumidityDeath(const float currentHealth, const float currentWaterMass, const float deathPerSecond) {
		if (m2::is_zero(currentWaterMass, 0.001f)) {
			const auto killAmount = deathPerSecond * SIMULATION_TICK_PERIOD_S;
			return std::clamp(currentHealth - killAmount, 0.0f, currentHealth);
		}
		return currentHealth;
	}

	/// Returns new healthy mass, and new diseased mass
	std::pair<float,float> CalculatePrimitiveDiseaseDeath(const float healthyMass, const float diseasedMass,
			const float diseaseSpreadRatePerSecond, const float diseasedDiePercentagePerSecond) {
		// First, calculate the new infections
		const auto newHealthyMass = std::clamp(healthyMass - diseasedMass * diseaseSpreadRatePerSecond * SIMULATION_TICK_PERIOD_S, 0.0f, healthyMass);
		// Then, calculate death due to the disease
		const auto newDiseasedMass = std::clamp(diseasedMass * diseasedDiePercentagePerSecond * SIMULATION_TICK_PERIOD_S, 0.0f, diseasedMass);
		return std::make_pair(newHealthyMass, newDiseasedMass);
	}

	/// Returns [0,1] rate
	float CalculateEfficiencyFromMinIdealMax(const float currentValue, const float minValue, const float idealValue, const float maxValue) {
		if (currentValue < minValue || maxValue < currentValue) {
			return 0.0f;
		}
		if (currentValue < idealValue) {
			return (currentValue - minValue) / (idealValue - minValue);
		} else {
			return (currentValue - idealValue) / (maxValue - idealValue);
		}
	}

	float CalculateProductionAmount(const float productionRatePerProducerPerSecond, const float producerAmount, const float efficiency) {
		return productionRatePerProducerPerSecond * producerAmount * efficiency * SIMULATION_TICK_PERIOD_S;
	}

	pb::SimulationState AdvanceTickCount(const pb::SimulationState& currentState) {
		auto nextState = currentState;
		nextState.set_passed_tick_count(currentState.passed_tick_count() + 1);
		return nextState;
	}

	pb::SimulationState AdvanceEnvironment(const pb::SimulationState& currentState, const pb::SimulationInputs& inputs) {
		auto nextState = currentState;
		// Add water and heat to the environment
		nextState.set_water_mass(currentState.water_mass() + inputs.extra_water());
		nextState.set_temperature(currentState.temperature() + SIMULATION_HEATING_RATE_PER_SECOND * SIMULATION_TICK_PERIOD_S);
		return nextState;
	}

	pb::SimulationState AdvanceBacteriaDeaths(const pb::SimulationState& currentState) {
		auto nextState = currentState;
		// Bacteria die at high temperatures. Healthy and diseased bacteria die at the same rate.
		const auto bacteriaMassAfterTemperatureDeath = CalculatePrimitiveTemperatureDeath(currentState.bacteria_mass(),
				SIMULATION_BACTERIA_DEATH_TEMPERATURE, currentState.temperature(),
				SIMULATION_BACTERIA_TEMPERATURE_DEATH_EXPONENT,
				SIMULATION_BACTERIA_TEMPERATURE_DEATH_AMOUNT_PER_UNIT_PER_SECOND);
		// Bacteria die naturally. Healthy and diseased bacteria die at the same rate.
		const auto bacteriaMassAfterNaturalDeath = bacteriaMassAfterTemperatureDeath
				* (1.0f - SIMULATION_BACTERIA_NATURAL_DEATH_RATE_PER_SECOND * SIMULATION_TICK_PERIOD_S);
		// Zombie bacteria kill healthy bacteria
		const auto diseasedBacteriaMassAfterTemperatureDeath = bacteriaMassAfterNaturalDeath * currentState.zombie_bacteria_percentage();
		const auto healthyBacteriaMassAfterTemperatureDeath = bacteriaMassAfterNaturalDeath - diseasedBacteriaMassAfterTemperatureDeath;
		const auto [healthyBacteriaMassAfterInfection, diseasedBacteriaMassAfterDiseaseDeath] = CalculatePrimitiveDiseaseDeath(
				healthyBacteriaMassAfterTemperatureDeath, diseasedBacteriaMassAfterTemperatureDeath,
				SIMULATION_BACTERIA_DISEASE_SPREAD_RATE_PER_SECOND, SIMULATION_BACTERIA_DISEASE_DIE_PERCENTAGE_PER_SECOND);
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
		const auto plantMassAfterTemperatureDeath = CalculatePrimitiveTemperatureDeath(currentState.plant_mass(),
				SIMULATION_PLANT_MAX_TEMPERATURE, currentState.temperature(),
				SIMULATION_PLANT_TEMPERATURE_DEATH_EXPONENT,
				SIMULATION_PLANT_TEMPERATURE_DEATH_AMOUNT_PER_UNIT_PER_SECOND);
		// Diseased plants kill healthy plants
		const auto diseasedPlantMassAfterTemperatureDeath = plantMassAfterTemperatureDeath * currentState.diseased_plant_percentage();
		const auto healthyPlantMassAfterTemperatureDeath = plantMassAfterTemperatureDeath - diseasedPlantMassAfterTemperatureDeath;
		const auto [healthyPlantMassAfterInfection, diseasedPlantMassAfterDiseaseDeath] = CalculatePrimitiveDiseaseDeath(
				healthyPlantMassAfterTemperatureDeath, diseasedPlantMassAfterTemperatureDeath,
				SIMULATION_PLANT_DISEASE_SPREAD_RATE_PER_SECOND, SIMULATION_PLANT_DISEASE_DIE_PERCENTAGE_PER_SECOND);
		const auto nextPlantMass = healthyPlantMassAfterInfection + diseasedPlantMassAfterDiseaseDeath;
		const auto deadBacteriaMass = currentState.plant_mass() - nextPlantMass;
		nextState.set_plant_mass(nextPlantMass);
		nextState.set_diseased_plant_percentage(diseasedPlantMassAfterDiseaseDeath / nextPlantMass);
		// Dead plants become waste mass. Eaten plants don't.
		nextState.set_waste_mass(currentState.waste_mass() + deadBacteriaMass);
		return nextState;
	}

	pb::SimulationState AdvanceAnimalDeaths(const pb::SimulationState& currentState, const std::function<void(int64_t)>& animalReleaser) {
		auto nextState = currentState;
		// Animals lose health at cold, high temperatures, and zero humidity. Clear and recreate animals.
		nextState.clear_animals();
		for (const auto& animal : currentState.animals()) {
			// Temperature
			const auto minTemperature = animal.type() == pb::Animal_Type_HERBIVORE
					? SIMULATION_HERBIVORE_MIN_TEMPERATURE : SIMULATION_CARNIVORE_MIN_TEMPERATURE;
			const auto maxTemperature = animal.type() == pb::Animal_Type_HERBIVORE
					? SIMULATION_HERBIVORE_MAX_TEMPERATURE : SIMULATION_CARNIVORE_MAX_TEMPERATURE;
			const auto temperatureDeathExponent = animal.type() == pb::Animal_Type_HERBIVORE
					? SIMULATION_HERBIVORE_TEMPERATURE_DEATH_EXPONENT : SIMULATION_CARNIVORE_TEMPERATURE_DEATH_EXPONENT;
			const auto temperatureDeathPerUnitPerSecond = animal.type() == pb::Animal_Type_HERBIVORE
					? SIMULATION_HERBIVORE_TEMPERATURE_DEATH_AMOUNT_PER_UNIT_PER_SECOND
					: SIMULATION_CARNIVORE_TEMPERATURE_DEATH_AMOUNT_PER_UNIT_PER_SECOND;
			const auto healthAfterTemperatureDeath = CalculateAnimalTemperatureDeath(animal.health(), minTemperature,
				maxTemperature, currentState.temperature(), temperatureDeathExponent, temperatureDeathPerUnitPerSecond);
			// Humidity
			const auto humidityDeathPerSecond = animal.type() == pb::Animal_Type_HERBIVORE
					? SIMULATION_HERBIVORE_HUMIDITY_DEATH_AMOUNT_PER_SECOND
					: SIMULATION_CARNIVORE_HUMIDITY_DEATH_AMOUNT_PER_SECOND;
			const auto healthAfterTemperatureAndHumidityDeath = CalculateAnimalHumidityDeath(healthAfterTemperatureDeath,
					currentState.water_mass(), humidityDeathPerSecond);
			// Reset reproduction count down if any health is lost
			int64_t reproductionCountDownAfterTemperatureDeath;
			if (m2::is_equal(animal.health(), healthAfterTemperatureAndHumidityDeath, 0.001f)) {
				reproductionCountDownAfterTemperatureDeath = animal.reproduction_count_down();
			} else {
				const auto defaultReproductionPeriodS = animal.type() == pb::Animal_Type_HERBIVORE
						? SIMULATION_HERBIVORE_DEFAULT_REPRODUCTION_PERIOD_S
						: SIMULATION_CARNIVORE_DEFAULT_REPRODUCTION_PERIOD_S;
				reproductionCountDownAfterTemperatureDeath = m2::iround(defaultReproductionPeriodS * SIMULATION_TICKS_PER_SECOND);
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
		if (not inputs.light() && SIMULATION_BACTERIA_DECOMPOSITION_MIN_TEMPERATURE <= currentState.temperature()
				&& currentState.temperature() <= SIMULATION_BACTERIA_DECOMPOSITION_MAX_TEMPERATURE) {
			// Decomposition speed of a single bacteria
			const float decompositionEfficiency = CalculateEfficiencyFromMinIdealMax(currentState.temperature(),
					SIMULATION_BACTERIA_DECOMPOSITION_MIN_TEMPERATURE,
					SIMULATION_BACTERIA_DECOMPOSITION_FASTEST_TEMPERATURE,
					SIMULATION_BACTERIA_DECOMPOSITION_MAX_TEMPERATURE);
			// Only healthy bacteria do decomposition.
			const auto healthyBacteriaMass = currentState.bacteria_mass() * (1.0f - currentState.zombie_bacteria_percentage());
			// Find rate limit due to waste availability
			const auto idealRequiredWasteMass = CalculateProductionAmount(SIMULATION_BACTERIA_DECOMPOSITION_WASTE_USE_PER_SECOND, healthyBacteriaMass, decompositionEfficiency);
			const auto availableWasteMass = std::min(currentState.waste_mass(), idealRequiredWasteMass);
			const auto rateLimitDueToWaste = availableWasteMass / idealRequiredWasteMass;
			// Find rate limit due to water availability
			const auto idealRequiredWaterMass = idealRequiredWasteMass * SIMULATION_BACTERIA_DECOMPOSITION_WATER_USE_RATE;
			const auto availableWaterMass = std::min(currentState.water_mass(), idealRequiredWaterMass);
			const auto rateLimitDueToWater = availableWaterMass / idealRequiredWaterMass;
			// Find actual rate limit
			const auto rateLimit = std::min(rateLimitDueToWaste, rateLimitDueToWater);
			const auto actualRequiredWasteMass = idealRequiredWasteMass * rateLimit;
			const auto actualRequiredWaterMass = idealRequiredWaterMass * rateLimit;
			// Decompose
			const auto producedNutrientMass = actualRequiredWasteMass * SIMULATION_BACTERIA_DECOMPOSITION_NUTRIENT_PRODUCTION_RATE;
			const auto reproductionMass = actualRequiredWasteMass * SIMULATION_BACTERIA_REPRODUCTION_RATE;
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
		if (inputs.light() && SIMULATION_PLANT_GROWTH_MIN_TEMPERATURE <= currentState.temperature()
				&& currentState.temperature() <= SIMULATION_PLANT_GROWTH_MAX_TEMPERATURE) {
			const float growthEfficiency = CalculateEfficiencyFromMinIdealMax(currentState.temperature(),
					SIMULATION_PLANT_GROWTH_MIN_TEMPERATURE,
					SIMULATION_PLANT_GROWTH_FASTEST_TEMPERATURE,
					SIMULATION_PLANT_GROWTH_MAX_TEMPERATURE);
			// Only healthy plants grow
			const auto healthyPlantMass = currentState.plant_mass() * (1.0f - currentState.diseased_plant_percentage());
			// Find rate limit due to nutrient availability
			const auto idealGrowthMass = CalculateProductionAmount(SIMULATION_PLANT_GROWTH_PRODUCTION_RATE_PER_SECOND, healthyPlantMass, growthEfficiency);
			const auto idealRequiredNutrientMass = idealGrowthMass * SIMULATION_PLANT_GROWTH_NUTRIENT_USE_RATE;
			const auto availableNutrientMass = std::min(currentState.nutrient_mass(), idealRequiredNutrientMass);
			const auto rateLimitDueToNutrient = availableNutrientMass / idealRequiredNutrientMass;
			// Find rate limit due to water availability
			const auto idealRequiredWaterMass = idealGrowthMass * SIMULATION_PLANT_GROWTH_WATER_USE_RATE;
			const auto availableWaterMass = std::min(currentState.water_mass(), idealRequiredWaterMass);
			const auto rateLimitDueToWater = availableWaterMass / idealRequiredWaterMass;
			// Find actual rate limit
			const auto rateLimit = std::min(rateLimitDueToNutrient, rateLimitDueToWater);
			const auto actualRequiredNutrientMass = idealRequiredNutrientMass * rateLimit;
			const auto actualRequiredWaterMass = idealRequiredWaterMass * rateLimit;
			const auto actualGrowthMass = actualRequiredWaterMass / SIMULATION_PLANT_GROWTH_WATER_USE_RATE;
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
}

pb::SimulationState pinball::InitialSimulationState(const std::function<int64_t(pb::Animal_Type)>& animalAllocator) {
	pb::SimulationState state;
	state.set_bacteria_mass(SIMULATION_BACTERIA_LIMIT_KG / 8.0f);
	state.set_plant_mass(SIMULATION_PLANT_LIMIT_KG / 50.0f);
	for (int i = 0; i < 6; ++i) {
		auto* animal = state.add_animals();
		animal->set_id(animalAllocator(pb::Animal_Type_HERBIVORE));
		animal->set_type(pb::Animal_Type_HERBIVORE);
		animal->set_mass(0.35f);
		animal->set_health(1.0f);
		animal->set_hunger(0.5f);
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
	// TODO Herbivores consume plants and water, and reproduce
	// TODO Carnivores consume herbivores, and reproduce
	return stateAfterPlantGrowth;
}
