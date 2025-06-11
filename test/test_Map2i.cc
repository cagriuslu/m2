#include <gtest/gtest.h>
#include <m2/containers/Map2i.h>

TEST(Map2i, basic) {
	using namespace m2;

	auto* map = new Map2i<int>();
	map->Allocate(VecI{}).first = 0;
	map->Allocate(VecI{1, 1}).first = 1;
	map->Allocate(VecI{-1, 1}).first = 2;
	map->Allocate(VecI{-1, -1}).first = 3;
	map->Allocate(VecI{1, -1}).first = 4;

	auto ids = map->FindIds(VecI{}, 0);
	EXPECT_EQ(ids.size(), 1);
	EXPECT_EQ(map->Get(ids[0])->obj, 0);

	auto ids2 = map->FindIds(VecI{-2, -2}, 3);
	EXPECT_EQ(ids2.size(), 5);
	EXPECT_TRUE(std::any_of(ids2.begin(), ids2.end(), [=](Map2iID id) {
		if (map->Get(id)->obj == 0) {
			return true;
		}
		return false;
	}));
	EXPECT_TRUE(std::any_of(ids2.begin(), ids2.end(), [=](Map2iID id) {
		if (map->Get(id)->obj == 1) {
			return true;
		}
		return false;
	}));
	EXPECT_TRUE(std::any_of(ids2.begin(), ids2.end(), [=](Map2iID id) {
		if (map->Get(id)->obj == 2) {
			return true;
		}
		return false;
	}));
	EXPECT_TRUE(std::any_of(ids2.begin(), ids2.end(), [=](Map2iID id) {
		if (map->Get(id)->obj == 3) {
			return true;
		}
		return false;
	}));
	EXPECT_TRUE(std::any_of(ids2.begin(), ids2.end(), [=](Map2iID id) {
		if (map->Get(id)->obj == 4) {
			return true;
		}
		return false;
	}));

	delete map;
}

TEST(Map2i, basic2) {
	using namespace m2;

	auto* map = new Map2i<int>();
	for (int y = -50; y < 50; y++) {
		for (int x = -50; x < 50; x++) {
			map->Allocate(VecI{x, y}).first = x * y;
		}
	}

	auto needle = VecI{17, -35};
	auto radius = 3;
	auto ids = map->FindIds(needle, radius);
	EXPECT_EQ(ids.size(), 49);
	for (auto id : ids) {
		EXPECT_TRUE(map->Get(id)->pos.IsNear(needle, radius));
	}

	delete map;
}
