#include <pinball/Simulation.h>
#include <m2/Error.h>
#include <m2/Math.h>
#include <m2/Meta.h>

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

	pb::SimulationState AdvanceTickCount(const pb::SimulationState& currentState) {
		auto nextState = currentState;
		nextState.set_passed_tick_count(currentState.passed_tick_count() + 1);
		return nextState;
	}

	pb::SimulationState AdvanceEnvironment(const pb::SimulationState& currentState, const pb::SimulationInputs& inputs) {
		auto nextState = currentState;
		// Add water and heat to the environment
		nextState.set_water_mass(currentState.water_mass() + inputs.extra_water());
		nextState.set_temperature(currentState.temperature() + (SIMULATION_HEATING_RATE_PER_SECOND * SIMULATION_TICK_PERIOD_S));
		return nextState;
	}

	pb::SimulationState AdvanceBacteriaDeaths(const pb::SimulationState& currentState) {
		auto nextState = currentState;
		// Bacteria die at high temperatures. Healthy and diseased bacteria die at the same rate.
		const auto bacteriaMassAfterTemperatureDeath = CalculatePrimitiveTemperatureDeath(currentState.bacteria_mass(),
				SIMULATION_BACTERIA_MAX_TEMPERATURE, currentState.temperature(),
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
}

pb::SimulationState pinball::AdvanceSimulation(const pb::SimulationState& currentState, const pb::SimulationInputs& inputs,
	const std::function<int64_t()>& animalAllocator, const std::function<void(int64_t)>& animalReleaser) {
	const auto stateAfterTickCount = AdvanceTickCount(currentState);
	const auto stateAfterEnvironment = AdvanceEnvironment(stateAfterTickCount, inputs);
	const auto stateAfterBacteriaDeaths = AdvanceBacteriaDeaths(stateAfterEnvironment);
	const auto stateAfterPlantDeaths = AdvancePlantDeaths(stateAfterBacteriaDeaths);
	const auto stateAfterAnimalDeaths = AdvanceAnimalDeaths(stateAfterPlantDeaths, animalReleaser);

	// Do consumptions and productions

	// If the conditions are right, bacteria consumes waste and produces nutrients.
	// If the conditions are right, plants consume nutrients and water, and produce mass
	// Herbivores consume plants and water
	// Carnivores consume herbivores

	// Do reproductions
}
