#include <m2/Def.hh>
#include <memory>
#include <random>
#include <memory>

float NORMALIZE_2PI(float angle) {
	float n = fmodf(angle, 2 * M2_PI);
	if (n < 0.0f) {
		return n + 2 * M2_PI;
	}
	return n;
}

std::unique_ptr<std::mt19937> random_number_engine;
std::unique_ptr<std::uniform_real_distribution<float>> random_number_distribution;

float randf() {
    if (!random_number_distribution) {
        // Seed with std::random_device
        std::random_device rd;
        random_number_engine = std::make_unique<std::mt19937>(rd());
        random_number_distribution = std::make_unique<std::uniform_real_distribution<float>>(0.0f, 1.0f);
    }
    return (*random_number_distribution)(*random_number_engine);
}
