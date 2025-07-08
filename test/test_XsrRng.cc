#include <gtest/gtest.h>
#include <m2/game/rng/XsrRng.h>
#include <m2/Log.h>
#include <m2/Meta.h>
#include <algorithm>

#include "m2/Math.h"
#include "m2/m3/VecF.h"

TEST(XsrRng, GenerateNextNumber64) {
	m2::XsrRng rng{0x6c9af76788f04b6e,
		0xf800f06302bddbfb,
		0x99c2b4630a2767c0,
		0x21350e644517af9c};

	std::array<int, 10> buckets{};
	std::ranges::fill(buckets, 0);

	m2Repeat(1000) {
		const auto bucketIdx = rng.GenerateNextNumber64() % 10;
		++buckets[bucketIdx];
	}

	for (auto i = 0; i < 10; ++i) {
		const auto msg = "Bucket Idx: " + m2::ToString(i) + " Count: " + m2::ToString(buckets[i]);
		LOG_INFO(msg.c_str());
	}
}
