#pragma once
#include <vector>

namespace m2 {
	/// Returns a 2D Gaussian kernel (commonly used in image processing) generated with the given parameters. The
	/// elements of the kernel is returned as a one dimensional array. The stride of the array is (2 * radius + 1).
	/// For example, if radius is 3, the vector will have 49 elements, and stride will be 7.
	std::vector<float> CreateGaussianKernel(int radius, float standardDeviation);
}
