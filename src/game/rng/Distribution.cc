#include <m2/game/rng/Distribution.h>

using namespace m2;

void CustomOptionDistribution::GenerateNextExact(Rng& rng, Exact& out) {
	uint64_t nextNumber;
	rng.GenerateNextNumber64(nextNumber);
	out = _options[nextNumber % _options.size()];
}
