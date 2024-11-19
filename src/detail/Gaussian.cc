#include <m2/detail/Gaussian.h>
#include <m2/Error.h>
#include <m2/M2.h>

std::vector<float> m2::create_gaussian_kernel(int radius, float standard_deviation) {
	if (radius < 0) {
		throw M2_ERROR("Gaussian radius is negative");
	}
	if (standard_deviation < 0) {
		throw M2_ERROR("Gaussian standard deviation is negative");
	}
	if (255 < radius) {
		throw M2_ERROR("Given Gaussian radius is too large");
	}

	// Size of the one side of the matrix
	auto side_size = radius * 2 + 1;

	// Create a non-normalized kernel
	float sum_of_all_elements{};
	std::vector<float> non_normalized(side_size * side_size, 0.0f);
	// The element with coordinates (x:mid_point, y:mid_point) will be in the middle of the kernel
	int mid_point = radius;
	for (int y = 0; y < side_size; ++y) {
		for (int x = 0; x < side_size; ++x) {
			// Find distance to the midpoint
			auto x_distance = static_cast<float>(::abs(x - mid_point));
			auto y_distance = static_cast<float>(::abs(y - mid_point));
			auto standard_dev_sq = powf(standard_deviation, 2.0f);
			// Find the Gaussian value using the formula given in https://en.wikipedia.org/wiki/Gaussian_blur
			auto gaussian =
					expf(
						-(powf(x_distance, 2.0f) + powf(y_distance, 2.0f))
						/ (2.0f * standard_dev_sq)
					)
					/ ( 2.0f * m2::PI * standard_dev_sq);

			non_normalized[y * side_size + x] = gaussian;
			sum_of_all_elements += gaussian;
		}
	}

	// Normalize the kernel
	std::vector<float> normalized(side_size * side_size, 0.0f);
	for (int y = 0; y < side_size; ++y) {
		for (int x = 0; x < side_size; ++x) {
			normalized[y * side_size + x] = non_normalized[y * side_size + x] / sum_of_all_elements;
		}
	}

	return normalized;
}
