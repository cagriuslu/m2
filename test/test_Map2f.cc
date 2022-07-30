#include <gtest/gtest.h>
#include <m2/Map2f.h>

TEST(Map2f, basic) {
	using namespace m2;

	auto* map = new Map2f<int>();
	map->alloc(Vec2f{}).first = 0;
	map->alloc(Vec2f{1, 1}).first = 1;
	map->alloc(Vec2f{-1, 1}).first = 2;
	map->alloc(Vec2f{-1, -1}).first = 3;
	map->alloc(Vec2f{1, -1}).first = 4;

	auto ids = map->find_ids(Vec2f{}, 0.5f);
	EXPECT_EQ(ids.size(), 1);
	EXPECT_EQ(map->get(ids[0])->obj, 0);

	auto ids2 = map->find_ids(Vec2f{-2, -2}, 3.5f);
	EXPECT_EQ(ids2.size(), 5);
	EXPECT_TRUE(std::any_of(ids2.begin(), ids2.end(), [=](Map2fID id) {
		if (map->get(id)->obj == 0) {
			return true;
		}
		return false;
	}));
	EXPECT_TRUE(std::any_of(ids2.begin(), ids2.end(), [=](Map2fID id) {
		if (map->get(id)->obj == 1) {
			return true;
		}
		return false;
	}));
	EXPECT_TRUE(std::any_of(ids2.begin(), ids2.end(), [=](Map2fID id) {
		if (map->get(id)->obj == 2) {
			return true;
		}
		return false;
	}));
	EXPECT_TRUE(std::any_of(ids2.begin(), ids2.end(), [=](Map2fID id) {
		if (map->get(id)->obj == 3) {
			return true;
		}
		return false;
	}));
	EXPECT_TRUE(std::any_of(ids2.begin(), ids2.end(), [=](Map2fID id) {
		if (map->get(id)->obj == 4) {
			return true;
		}
		return false;
	}));

	delete map;
}

TEST(Map2f, basic2) {
	using namespace m2;

	auto* map = new Map2f<int>();
	for (int y = -50; y < 50; y++) {
		for (int x = -50; x < 50; x++) {
			map->alloc(Vec2f{x, y}).first = x * y;
		}
	}
	map->shuffle();

	auto needle = Vec2f{17, -35};
	auto radius = 3.5f;
	auto ids = map->find_ids(needle, radius);
	EXPECT_EQ(ids.size(), 49);
	for (auto id : ids) {
		EXPECT_TRUE(map->get(id)->pos.is_near(needle, radius));
	}

	delete map;
}
