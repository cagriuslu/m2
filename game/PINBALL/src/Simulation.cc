#include <pinball/Simulation.h>
#include <m2/Error.h>
#include <m2/Math.h>
#include <m2/Meta.h>
#include <m2/Log.h>
#include <list>

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

	float CalculateAnimalTemperatureDamage(const float minTemperature,
			const float maxTemperature, const float currentTemperature, const float damagePerUnitPerSecond) {
		if (currentTemperature < minTemperature || maxTemperature < currentTemperature) {
			const auto temperatureDiff = currentTemperature < minTemperature
					? minTemperature - currentTemperature : currentTemperature - maxTemperature;
			return temperatureDiff * SIMULATION_TICK_PERIOD_S * damagePerUnitPerSecond;
		}
		return 0.0f;
	}

	float CalculateAnimalHumidityDamage(const float currentWaterMass, const float damagePerSecond) {
		if (m2::is_zero(currentWaterMass, 0.001f)) {
			return damagePerSecond * SIMULATION_TICK_PERIOD_S;
		}
		return 0.0f;
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
			return 1.0f - (currentValue - idealValue) / (maxValue - idealValue);
		}
	}

	float CalculateProductionAmount(const float productionRatePerProducerPerSecond, const float producerAmount,
			const float efficiency, const float maxProducerAmount) {
		const auto limitlessProductionAmount = productionRatePerProducerPerSecond * producerAmount * efficiency
				* SIMULATION_TICK_PERIOD_S;
		const auto remainingSpace = maxProducerAmount - producerAmount;
		return std::min(limitlessProductionAmount, remainingSpace);
	}

	float CalculateHungerAfterIncrease(const float currentHunger, const float hungerIncreasePerSecond) {
		return std::clamp(currentHunger + hungerIncreasePerSecond * SIMULATION_TICK_PERIOD_S, currentHunger, 1.0f);
	}

	pb::Animal CreateHerbivore(const AnimalAllocator& animalAllocator) {
		pb::Animal animal;
		animal.set_id(animalAllocator(pb::Animal_Type_HERBIVORE));
		animal.set_type(pb::Animal_Type_HERBIVORE);
		animal.set_mass(m2::UniformRandomF(HERBIVORE_MASS - HERBIVORE_MASS_VARIANCE,
				HERBIVORE_MASS + HERBIVORE_MASS_VARIANCE));
		animal.set_health(1.0f);
		animal.set_hunger(0.25f);
		animal.set_reproduction_count_down(m2::iround(HERBIVORE_DEFAULT_REPRODUCTION_PERIOD_S
				* SIMULATION_TICKS_PER_SECOND));
		return animal;
	}

	pb::Animal CreateCarnivore(const AnimalAllocator& animalAllocator) {
		pb::Animal animal;
		animal.set_id(animalAllocator(pb::Animal_Type_CARNIVORE));
		animal.set_type(pb::Animal_Type_CARNIVORE);
		animal.set_mass(m2::UniformRandomF(CARNIVORE_MASS - CARNIVORE_MASS_VARIANCE,
				CARNIVORE_MASS + CARNIVORE_MASS_VARIANCE));
		animal.set_health(1.0f);
		animal.set_hunger(0.25f);
		animal.set_reproduction_count_down(m2::iround(CARNIVORE_DEFAULT_REPRODUCTION_PERIOD_S
				* SIMULATION_TICKS_PER_SECOND));
		return animal;
	}

	const pb::Animal* FindRandomHerbivore(const google::protobuf::RepeatedPtrField<pb::Animal>& animals,
			const std::list<int64_t>& excludedAnimals) {
		std::vector<const pb::Animal*> availableHerbivores;
		// Gather non-excluded herbivores
		for (const auto& animal : animals) {
			if (animal.type() == pb::Animal_Type_HERBIVORE
					&& std::ranges::find(excludedAnimals, animal.id()) == excludedAnimals.end()) {
				availableHerbivores.emplace_back(&animal);
			}
		}
		if (availableHerbivores.empty()) {
			return nullptr;
		}
		// Select random
		const auto index = m2::Random64(availableHerbivores.size());
		return availableHerbivores[index];
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
		nextState.set_water_mass(std::clamp(currentState.water_mass() + inputs.extra_water(), 0.0f, MAX_WATER_MASS));
		nextState.set_temperature(std::clamp(currentState.temperature() +
				(inputs.heat() ? HEATING_RATE_PER_SECOND : COOLING_RATE_PER_SECOND) * SIMULATION_TICK_PERIOD_S,
				MIN_TEMPERATURE, MAX_TEMPERATURE));
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
		const auto nextWasteMass = std::min(currentState.water_mass() + deadBacteriaMass, MAX_WASTE_MASS);
		nextState.set_waste_mass(nextWasteMass);
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
		const auto nextWasteMass = std::min(currentState.waste_mass() + deadBacteriaMass, MAX_WASTE_MASS);
		nextState.set_waste_mass(nextWasteMass);
		return nextState;
	}

	pb::SimulationState AdvanceAnimalDeaths(const pb::SimulationState& currentState,
			const AnimalReleaser& animalReleaser) {
		auto nextState = currentState;
		// Animals lose health at cold, high temperatures, and zero humidity. Clear and recreate animals.
		nextState.clear_animals();
		for (const auto& animal : currentState.animals()) {
			// Temperature damage
			const auto temperatureDamage = [animalType = animal.type(), temperature = currentState.temperature()] {
				const auto minTemperature = animalType == pb::Animal_Type_HERBIVORE
						? HERBIVORE_MIN_TEMPERATURE : CARNIVORE_MIN_TEMPERATURE;
				const auto maxTemperature = animalType == pb::Animal_Type_HERBIVORE
						? HERBIVORE_MAX_TEMPERATURE : CARNIVORE_MAX_TEMPERATURE;
				const auto temperatureDeathPerUnitPerSecond = animalType == pb::Animal_Type_HERBIVORE
						? HERBIVORE_TEMPERATURE_DAMAGE_PER_CELSIUS_PER_SECOND
						: CARNIVORE_TEMPERATURE_DAMAGE_PER_CELSIUS_PER_SECOND;
				return CalculateAnimalTemperatureDamage(minTemperature, maxTemperature,
						temperature, temperatureDeathPerUnitPerSecond);
			}();
			// Hunger
			const auto hungerDamage = [hunger = animal.hunger(), animalType = animal.type()] {
				if (m2::is_one(hunger, 0.001f)) {
					const auto hungerDamagePerSecond = animalType == pb::Animal_Type_HERBIVORE
							? HERBIVORE_HUNGER_DAMAGE_PER_SECOND : CARNIVORE_HUNGER_DAMAGE_PER_SECOND;
					return hungerDamagePerSecond * SIMULATION_TICK_PERIOD_S;
				}
				return 0.0f;
			}();
			// Humidity
			const auto humidityDamage = [animalType = animal.type(), waterMass = currentState.water_mass()] {
				const auto humidityDeathPerSecond = animalType == pb::Animal_Type_HERBIVORE
					? HERBIVORE_HUMIDITY_DEATH_AMOUNT_PER_SECOND
					: CARNIVORE_HUMIDITY_DEATH_AMOUNT_PER_SECOND;
				return CalculateAnimalHumidityDamage(waterMass, humidityDeathPerSecond);
			}();
			// Calculate health after damages
			const auto newHealth = std::clamp(animal.health() - temperatureDamage - hungerDamage - humidityDamage, 0.0f,
					1.0f);
			// Reset reproduction count down if any health is lost
			const auto newReproductionCountDown = [currHealthy = animal.health(), newHealth,
					currReproductionCountDown = animal.reproduction_count_down(), animalType = animal.type()] {
				if (m2::is_equal(currHealthy, newHealth, 0.001f)) {
					return currReproductionCountDown;
				}
				const auto defaultReproductionPeriodS = animalType == pb::Animal_Type_HERBIVORE
						? HERBIVORE_DEFAULT_REPRODUCTION_PERIOD_S
						: CARNIVORE_DEFAULT_REPRODUCTION_PERIOD_S;
				return static_cast<int64_t>(m2::iround(defaultReproductionPeriodS * SIMULATION_TICKS_PER_SECOND));
			}();
			// Delete the animal if the health is zero
			if (m2::is_zero(newHealth, 0.001f)) {
				animalReleaser(animal.id());
				// Dead animals become waste mass. Eaten animals don't.
				const auto nextWasteMass = std::min(nextState.waste_mass() + animal.mass(), MAX_WASTE_MASS);
				nextState.set_waste_mass(nextWasteMass);
			} else {
				pb::Animal animalAfterDeath = animal;
				animalAfterDeath.set_health(newHealth);
				animalAfterDeath.set_reproduction_count_down(newReproductionCountDown);
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
			const auto idealRequiredWasteMass = CalculateProductionAmount(BACTERIA_DECOMPOSITION_WASTE_USE_PER_SECOND,
					healthyBacteriaMass, decompositionEfficiency, BACTERIA_MAX_MASS_KG);
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
			// Decomposition doesn't stop because of nutrient limit
			const auto nextNutrientMass = std::min(currentState.nutrient_mass() + producedNutrientMass, MAX_NUTRIENT_MASS);
			nextState.set_nutrient_mass(nextNutrientMass);
			nextState.set_water_mass(currentState.water_mass() - actualRequiredWaterMass);
			nextState.set_bacteria_mass(currentState.bacteria_mass() + reproductionMass);
			nextState.set_zombie_bacteria_percentage(newZombieBacteriaPercentage);
			// Calculate statistics
			const auto bestPossibleWasteUse = CalculateProductionAmount(BACTERIA_DECOMPOSITION_WASTE_USE_PER_SECOND,
					healthyBacteriaMass, 1.0f, BACTERIA_MAX_MASS_KG);
			const auto bestPossibleNutrientProduction = bestPossibleWasteUse * BACTERIA_DECOMPOSITION_NUTRIENT_PRODUCTION_RATE;
			const auto decompositionRate = producedNutrientMass / bestPossibleNutrientProduction;
			nextState.set_last_decomposition_rate(decompositionRate);
		} else {
			nextState.set_last_decomposition_rate(0.0f);
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
			const auto idealGrowthMass = CalculateProductionAmount(PLANT_GROWTH_PRODUCTION_RATE_PER_SECOND,
					healthyPlantMass, growthEfficiency, PLANT_MAX_MASS_KG);
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
			// Calculate statistics
			const auto bestPossibleGrowth = CalculateProductionAmount(PLANT_GROWTH_PRODUCTION_RATE_PER_SECOND,
					healthyPlantMass, 1.0f, PLANT_MAX_MASS_KG);
			const auto growthRate = actualGrowthMass / bestPossibleGrowth;
			nextState.set_last_photosynthesis_rate(growthRate);
		} else {
			nextState.set_last_photosynthesis_rate(0.0f);
		}
		return nextState;
	}

	pb::SimulationState AdvanceHerbivoreGrowth(const pb::SimulationState& currentState, const AnimalAllocator& animalAllocator) {
		auto nextState = currentState;

		std::list<pb::Animal> newAnimals;
		for (auto& animal : *nextState.mutable_animals()) {
			if (animal.type() != pb::Animal::HERBIVORE) {
				continue;
			}
			// First, increase hunger
			animal.set_hunger(
					CalculateHungerAfterIncrease(animal.hunger(), HERBIVORE_HUNGER_PERCENTAGE_GAINED_PER_SECOND));
			// Then, feed
			const auto idealHungerPercentageToFulfill = std::min(animal.hunger(),
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
			nextState.set_water_mass(nextState.water_mass() - actualRequiredWaterMass);
			nextState.set_plant_mass(nextState.plant_mass() - actualRequiredPlantMass);

			// Check if reproduction count down needs to be reset
			if (HERBIVORE_REPRODUCTION_HUNGER_THRESHOLD < animal.hunger()) {
				animal.set_reproduction_count_down(
						m2::iround(HERBIVORE_DEFAULT_REPRODUCTION_PERIOD_S * SIMULATION_TICKS_PER_SECOND));
			} else {
				if (const auto nextReproductionCountDown = animal.reproduction_count_down() - 1;
						nextReproductionCountDown == 0) {
					// Reproduce
					newAnimals.emplace_back(CreateHerbivore(animalAllocator));
					animal.set_reproduction_count_down(m2::iround(HERBIVORE_DEFAULT_REPRODUCTION_PERIOD_S
							* SIMULATION_TICKS_PER_SECOND));
				} else {
					animal.set_reproduction_count_down(nextReproductionCountDown);
				}
			}
		}
		// Add new animals
		for (const auto& newAnimal : newAnimals) {
			nextState.add_animals()->CopyFrom(newAnimal);
		}
		return nextState;
	}

	pb::SimulationState AdvanceCarnivoreGrowth(const pb::SimulationState& currentState, const AnimalAllocator& animalAllocator, const AnimalReleaser& animalReleaser) {
		auto nextState = currentState;

		std::list<pb::Animal> newAnimals;
		std::list<int64_t> huntedHerbivores;
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
					if (const auto* victim = FindRandomHerbivore(nextState.animals(), huntedHerbivores)) {
						// Hunt animal
						huntedHerbivores.emplace_back(victim->id());
						// Lose hunger
						const auto victimMassRequiredForFullHunger = animal.mass() * CARNIVORE_HERBIVORE_MASS_REQUIRED_FOR_FULL_HUNGER_FULFILLMENT;
						const auto victimMass = victim->mass();
						const auto hungerToLose = victimMass / victimMassRequiredForFullHunger;
						const auto newHunger = std::clamp(animal.hunger() - hungerToLose, 0.0f, 1.0f);
						animal.set_hunger(newHunger);
					}
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
				if (const auto nextReproductionCountDown = animal.reproduction_count_down() - 1;
						nextReproductionCountDown == 0) {
					newAnimals.emplace_back(CreateCarnivore(animalAllocator));
					animal.set_reproduction_count_down(m2::iround(CARNIVORE_DEFAULT_REPRODUCTION_PERIOD_S
							* SIMULATION_TICKS_PER_SECOND));
				} else {
					animal.set_reproduction_count_down(nextReproductionCountDown);
				}
			}
		}
		// Add new animals
		for (const auto& newAnimal : newAnimals) {
			nextState.add_animals()->CopyFrom(newAnimal);
		}
		// Remove hunted herbivores if necessary
		if (huntedHerbivores.empty()) {
			return nextState;
		} else {
			auto nextNextState = nextState;
			nextNextState.clear_animals();
			for (const auto& animal : nextState.animals()) {
				if (animal.type() == pb::Animal_Type_HERBIVORE && std::ranges::find(huntedHerbivores, animal.id()) != huntedHerbivores.end()) {
					// Animal is hunted
					animalReleaser(animal.id());
				} else {
					nextNextState.add_animals()->CopyFrom(animal);
				}
			}
			return nextNextState;
		}
	}
}

pb::SimulationState pinball::InitialSimulationState(const AnimalAllocator& animalAllocator) {
	pb::SimulationState state;
	state.set_bacteria_mass(BACTERIA_SAFE_MASS_LIMIT_KG / 8.0f);
	state.set_plant_mass(PLANT_SAFE_MASS_LIMIT_KG / 20.0f);
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
	state.set_water_mass(5.0f);
	return state;
}

pb::SimulationState pinball::AdvanceSimulation(const pb::SimulationState& currentState, const pb::SimulationInputs& inputs,
	const AnimalAllocator& animalAllocator, const AnimalReleaser& animalReleaser) {
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
