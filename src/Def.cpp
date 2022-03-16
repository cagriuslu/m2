#include "m2/Def.h"
#include <random>
#include <memory>

std::unique_ptr<std::mt19937> random_number_engine;
std::unique_ptr<std::uniform_real_distribution<float>> random_number_distribution;

float randf() {
	if (!random_number_distribution) {
		// Seed with std::random_device
		std::random_device rd;
		random_number_engine.reset(new std::mt19937(rd()));
		random_number_distribution.reset(new std::uniform_real_distribution<float>(0.0f, 1.0f));
	}
	return (*random_number_distribution)(*random_number_engine);
}
