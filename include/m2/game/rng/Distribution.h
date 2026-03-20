#pragma once
#include "Rng.h"

namespace m2 {
	class Distribution {
	public:
		virtual ~Distribution() = default;

		virtual Exact GenerateNextExact(Rng&) = 0;
	};

	class CustomOptionDistribution : public Distribution {
		std::vector<Exact> _options;

	public:
		explicit CustomOptionDistribution(std::vector<Exact>&& options) : _options(std::move(options)) {}

		Exact GenerateNextExact(Rng&) override;
	};
}
