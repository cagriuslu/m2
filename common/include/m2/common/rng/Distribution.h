#pragma once
#include "Rng.h"

namespace m2 {
	class Distribution {
	public:
		virtual ~Distribution() = default;

		/// Following functions return void by design. This ensures that they aren't called from function parameters
		/// (ex. Func(GenerateNextNumber(), GenerateNextNumber()) because different platforms may call such these
		/// functions in different order, which could break the determinism of the RNG.

		virtual void GenerateNextExact(Rng&, Exact& out) = 0;
	};

	class CustomOptionDistribution : public Distribution {
		std::vector<Exact> _options;

	public:
		explicit CustomOptionDistribution(std::vector<Exact>&& options) : _options(std::move(options)) {}

		void GenerateNextExact(Rng&, Exact& out) override;
	};
}
