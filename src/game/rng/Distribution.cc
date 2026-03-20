#include <m2/game/rng/Distribution.h>

using namespace m2;

Exact CustomOptionDistribution::GenerateNextExact(Rng& rng) {
	return _options[rng.GenerateNextNumber64() % _options.size()];
}
