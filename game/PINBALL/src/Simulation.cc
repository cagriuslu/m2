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
}

pb::SimulationState pinball::AdvanceSimulation(const pb::SimulationState& currentState, const pb::SimulationInputs& inputs,
	const std::function<int64_t()>& animalAllocator, const std::function<void(int64_t)>& animalReleaser) {
	pb::SimulationState nextState;
	nextState.set_passed_tick_count(currentState.passed_tick_count() + 1);

	// Add water and heat to the environment
	const auto waterMassAfterAddition = currentState.water_mass() + inputs.extra_water();
	const auto temperatureAfterHeating = currentState.temperature() + (SIMULATION_HEATING_RATE_PER_SECOND * SIMULATION_TICK_PERIOD_S);
	// Temperature doesn't change any other way, we can record it.
	nextState.set_temperature(temperatureAfterHeating);

	///////////////////////////////////////////////// Do the killings /////////////////////////////////////////////////
	float wasteMassFromKillings = 0.0f;

	// Bacteria die at high temperatures. Healthy and diseased bacteria die at the same rate.
	const auto bacteriaMassAfterTemperatureDeath = CalculatePrimitiveTemperatureDeath(currentState.bacteria_mass(),
			SIMULATION_BACTERIA_MAX_TEMPERATURE, temperatureAfterHeating,
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
	wasteMassFromKillings += deadBacteriaMass;
	nextState.set_bacteria_mass(nextBacteriaMass);
	nextState.set_zombie_bacteria_percentage(diseasedBacteriaMassAfterDiseaseDeath / nextBacteriaMass);

	// Plants die at high temperatures. Healthy and diseased plants die at the same rate.
	const auto plantMassAfterTemperatureDeath = CalculatePrimitiveTemperatureDeath(currentState.plant_mass(),
			SIMULATION_PLANT_MAX_TEMPERATURE, temperatureAfterHeating,
			SIMULATION_PLANT_TEMPERATURE_DEATH_EXPONENT,
			SIMULATION_PLANT_TEMPERATURE_DEATH_AMOUNT_PER_UNIT_PER_SECOND);
	// Diseased plants kill healthy plants
	const auto diseasedPlantMassAfterTemperatureDeath = plantMassAfterTemperatureDeath * currentState.diseased_plant_percentage();
	const auto healthyPlantMassAfterTemperatureDeath = plantMassAfterTemperatureDeath - diseasedPlantMassAfterTemperatureDeath;
	const auto [healthyPlantMassAfterInfection, diseasedPlantMassAfterDiseaseDeath] = CalculatePrimitiveDiseaseDeath(
			healthyPlantMassAfterTemperatureDeath, diseasedPlantMassAfterTemperatureDeath,
			SIMULATION_PLANT_DISEASE_SPREAD_RATE_PER_SECOND, SIMULATION_PLANT_DISEASE_DIE_PERCENTAGE_PER_SECOND);
	// Dead plants become waste mass. Eaten plants don't.
	wasteMassFromKillings += currentState.plant_mass() - (healthyPlantMassAfterInfection + diseasedPlantMassAfterDiseaseDeath);

	// Animals lose health at cold and high temperatures
	// Animals lose health at zero humidity
	std::vector<pb::Animal> animalsAfterTemperatureAndHumidityDeath;
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
			maxTemperature, temperatureAfterHeating, temperatureDeathExponent, temperatureDeathPerUnitPerSecond);
		// Humidity
		const auto humidityDeathPerSecond = animal.type() == pb::Animal_Type_HERBIVORE
				? SIMULATION_HERBIVORE_HUMIDITY_DEATH_AMOUNT_PER_SECOND
				: SIMULATION_CARNIVORE_HUMIDITY_DEATH_AMOUNT_PER_SECOND;
		const auto healthAfterTemperatureAndHumidityDeath = CalculateAnimalHumidityDeath(healthAfterTemperatureDeath,
				waterMassAfterAddition, humidityDeathPerSecond);
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
			wasteMassFromKillings += animal.mass();
		} else {
			pb::Animal animalAfterTemperatureDeath = animal;
			animalAfterTemperatureDeath.set_health(healthAfterTemperatureAndHumidityDeath);
			animalAfterTemperatureDeath.set_reproduction_count_down(reproductionCountDownAfterTemperatureDeath);
			animalsAfterTemperatureAndHumidityDeath.emplace_back(animalAfterTemperatureDeath);
		}
	}
	// Waste doesn't occur any other way, calculate the total.
	const auto wasteAfterKillings = currentState.waste_mass() + wasteMassFromKillings;

	// Do consumptions and productions

	// If the conditions are right, bacteria consumes waste and produces nutrients.
	// If the conditions are right, plants consume nutrients and water, and produce mass
	// Herbivores consume plants and water
	// Carnivores consume herbivores

	// Do reproductions
}
