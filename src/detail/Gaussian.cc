#include <m2/detail/Gaussian.h>
#include <m2/Error.h>
#include <m2/M2.h>
#include <m2/Math.h>

std::vector<float> m2::CreateGaussianKernel(const int radius, const float standardDeviation) {
	if (radius < 0) {
		throw M2_ERROR("Gaussian radius is negative");
	}
	if (standardDeviation < 0) {
		throw M2_ERROR("Gaussian standard deviation is negative");
	}
	if (255 < radius) {
		throw M2_ERROR("Given Gaussian radius is too large");
	}

	// Size of the one side of the matrix
	const auto sideSize = radius * 2 + 1;

	// Create a non-normalized kernel
	float sumOfAllElements{};
	std::vector nonNormalized(sideSize * sideSize, 0.0f);
	// The element with coordinates (x:midPoint, y:midPoint) will be in the middle of the kernel
	const int midPoint = radius;
	for (int y = 0; y < sideSize; ++y) {
		for (int x = 0; x < sideSize; ++x) {
			// Find distance to the midpoint
			const auto xDistance = static_cast<float>(abs(x - midPoint));
			const auto yDistance = static_cast<float>(abs(y - midPoint));
			const auto standardDevSq = powf(standardDeviation, 2.0f);
			// Find the Gaussian value using the formula given in https://en.wikipedia.org/wiki/Gaussian_blur
			const auto gaussian = expf(-(powf(xDistance, 2.0f) + powf(yDistance, 2.0f)) / (2.0f * standardDevSq))
				/ ( 2.0f * PI * standardDevSq);

			nonNormalized[y * sideSize + x] = gaussian;
			sumOfAllElements += gaussian;
		}
	}

	// Normalize the kernel
	std::vector normalized(sideSize * sideSize, 0.0f);
	for (int y = 0; y < sideSize; ++y) {
		for (int x = 0; x < sideSize; ++x) {
			normalized[y * sideSize + x] = nonNormalized[y * sideSize + x] / sumOfAllElements;
		}
	}
	return normalized;
}
