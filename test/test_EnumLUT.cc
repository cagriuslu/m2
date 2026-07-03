#include <gtest/gtest.h>
#include <m2/common/protobuf/EnumLUT.h>
#include <Enums.pb.h>
#include <array>
#include <utility>
#include <vector>

using namespace m2;

namespace {
	// m2::pb::LogLevel is an engine-level protobuf enum (never overridden by any game), so this test compiles for
	// every game configuration. This mimics real usage of PartialEnumLUT (see game/WAR/include/m2g/LUTs.h).
	constexpr auto kLevelOptions = std::array{pb::TRC, pb::GFX, pb::PHY};
	using LevelLut = pb::PartialEnumLUT<int, kLevelOptions>;

	// A one-to-one mapping in the same order as the possible options.
	constexpr auto kLut = LevelLut::Create(
		std::pair{pb::TRC, 100},
		std::pair{pb::GFX, 200},
		std::pair{pb::PHY, 300});

	// Get() is consteval, so it can be exercised at compile time.
	static_assert(kLut.Get<pb::TRC>() == 100);
	static_assert(kLut.Get<pb::GFX>() == 200);
	static_assert(kLut.Get<pb::PHY>() == 300);
}

TEST(EnumLUT, GetReturnsMappedValues) {
	constexpr int trace = kLut.Get<pb::TRC>();
	constexpr int physics = kLut.Get<pb::PHY>();
	EXPECT_EQ(trace, 100);
	EXPECT_EQ(physics, 300);
}

TEST(EnumLUT, TryGetPresent) {
	const int* value = kLut.TryGet(pb::GFX);
	ASSERT_NE(value, nullptr);
	EXPECT_EQ(*value, 200);
}

TEST(EnumLUT, TryGetAbsentReturnsNull) {
	// NO_LOG_LEVEL and FTL are valid LogLevel values but are not part of this LUT's options.
	EXPECT_EQ(kLut.TryGet(pb::NO_LOG_LEVEL), nullptr);
	EXPECT_EQ(kLut.TryGet(pb::FTL), nullptr);
}

TEST(EnumLUT, UnsafeGetPresent) {
	EXPECT_EQ(kLut.UnsafeGet(pb::TRC), 100);
	EXPECT_EQ(kLut.UnsafeGet(pb::PHY), 300);
}

TEST(EnumLUT, ForEachVisitsEveryOptionInOrder) {
	std::vector<std::pair<pb::LogLevel, int>> visited;
	kLut.ForEach([&](const pb::LogLevel option, const int value) {
		visited.emplace_back(option, value);
	});
	ASSERT_EQ(visited.size(), 3u);
	EXPECT_EQ(visited[0], (std::pair{pb::TRC, 100}));
	EXPECT_EQ(visited[1], (std::pair{pb::GFX, 200}));
	EXPECT_EQ(visited[2], (std::pair{pb::PHY, 300}));
}

TEST(EnumLUT, CreateThrowsWhenMappingOrderIsWrong) {
	// The options are {TRC, GFX, PHY}; supplying them out of order is not a one-to-one mapping.
	EXPECT_ANY_THROW({
		auto bad = LevelLut::Create(
			std::pair{pb::GFX, 200},
			std::pair{pb::TRC, 100},
			std::pair{pb::PHY, 300});
		(void) bad;
	});
}

TEST(EnumLUT, CreateThrowsWhenMappingIsIncomplete) {
	EXPECT_ANY_THROW({
		auto bad = LevelLut::Create(
			std::pair{pb::TRC, 100},
			std::pair{pb::GFX, 200});
		(void) bad;
	});
}
