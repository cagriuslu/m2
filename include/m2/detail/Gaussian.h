#pragma once
#include <vector>
#include <tuple>

namespace m2 {
	/// Returns a 2D Gaussian kernel (commonly used in image processing) generated with the given parameters. The
	/// elements of the kernel is returned as a one dimensional array. The stride of the array is the same as
	/// odd_radius. For example, if odd_radius is 3, the vector will have 9 elements, and stride will be 3.
	std::vector<float> create_gaussian_kernel(int radius, float standard_deviation);
}
