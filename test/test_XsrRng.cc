#include <gtest/gtest.h>
#include <m2/common/rng/XsrRng.h>
#include <m2/Log.h>
#include <m2/common/Meta.h>
#include <algorithm>

#include "m2/common/Math.h"
#include "m2/common/m3/VecF.h"

TEST(XsrRng, GenerateNextNumber64) {
	m2::XsrRng rng{0x6c9af76788f04b6e,
		0xf800f06302bddbfb,
		0x99c2b4630a2767c0,
		0x21350e644517af9c};

	std::array<int, 10> buckets{};
	std::ranges::fill(buckets, 0);

	m2Repeat(100000) {
		uint64_t number;
		rng.GenerateNextNumber64(number);
		const auto bucketIdx = number % 10;
		++buckets[bucketIdx];
	}

	for (auto i = 0; i < 10; ++i) {
		const auto msg = std::format("Bucket Idx: {} Count: {}", i, buckets[i]);
		LOG_INFO(msg.c_str());
	}
}
