#include <gtest/gtest.h>
#include <m2/containers/MapF.h>
#include <m2/math/RectF.h>

TEST(MapF, size) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};

	int value = 0;
	for (auto y = -6.0f; y < 6.0f; y += 0.1f) {
		for (auto x = -6.0f; x < 6.0f; x += 0.1f) {
			auto [obj, id] = m.emplace({x, y, 0.05f, 0.05f}, value);
			EXPECT_EQ(obj, value);
			EXPECT_NE(id, 0);
			value++;
		}
	}
	EXPECT_EQ(m.size(), 14641);
}

TEST(MapF, contains) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};
	auto [obj, id] = m.emplace({}, 5);
	EXPECT_TRUE(m.contains(id));

	auto [obj2, id2] = m.emplace({1.0f, 1.0f, 0.0f, 0.0f}, 10);
	EXPECT_TRUE(m.contains(id2));
}

TEST(MapF, get) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};
	auto [obj, id] = m.emplace({}, 5);
	EXPECT_EQ(*m.get(id), 5);

	auto [obj2, id2] = m.emplace({1.0f, 1.0f, 0.0f, 0.0f}, 10);
	EXPECT_EQ(*m.get(id2), 10);
}

TEST(MapF, access) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};
	auto [obj, id] = m.emplace({}, 5);
	EXPECT_EQ(m[id], 5);

	auto [obj2, id2] = m.emplace({1.0f, 1.0f, 0.0f, 0.0f}, 10);
	EXPECT_EQ(m[id2], 10);
}

TEST(MapF, area) {
	auto area = m2::RectF{-5.0f, -5.0f, 10.0f, 10.0f};
	m2::MapF<int> m(area);
	EXPECT_TRUE(m.area().equals(area));
}

TEST(MapF, for_each) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};

	m2::RectF expect_rect{1.2f, 2.4f, 0.05f, 0.05f};
	int expect_value = 0;
	m2::Id expect_id = 0;

	int value = 0;
	for (auto y = -6.0f; y < 6.0f; y += 0.1f) {
		for (auto x = -6.0f; x < 6.0f; x += 0.1f) {
			auto rect = m2::RectF{x, y, 0.05f, 0.05f};
			auto [_, id] = m.emplace(rect, value);

			if (rect.equals(expect_rect)) {
				expect_value = value;
				expect_id = id;
			}

			value++;
		}
	}

	bool item_found = false;
	m.for_each([&item_found, expect_rect, expect_value, expect_id](const m2::RectF& area, m2::Id id, int i) {
		if (area.equals(expect_rect)) {
			EXPECT_FALSE(item_found);
			item_found = true;
			EXPECT_EQ(id, expect_id);
			EXPECT_EQ(i, expect_value);
		}
		return true;
	});
	EXPECT_TRUE(item_found);
}

TEST(MapF, for_each_containing) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};
	auto [value1, id1] = m.emplace({-6.0f, -6.0f, 0.5f, 0.5f}, 1);
	auto [value2, id2] = m.emplace({-5.25f, -5.25f, 0.5f, 0.5f}, 2);
	auto [value3, id3] = m.emplace({-4.5f, -4.5f, 0.5f, 0.5f}, 3);
	m.emplace({-0.5f, -0.5f, 0.5f, 0.5f}, 4);

	// Full world search
	bool test_failed1 = false;
	auto count1 = m.for_each_containing({-6.5f, -6.5f, 3.0f, 3.0f}, [=, &test_failed1]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
		if (id == id1) {
			if (value != value1) {
				test_failed1 = true;
			}
		} else if (id == id2) {
			if (value != value2) {
				test_failed1 = true;
			}
		} else if (id == id3) {
			if (value != value3) {
				test_failed1 = true;
			}
		} else {
			test_failed1 = true;
		}
		return true;
	});
	EXPECT_FALSE(test_failed1);
	EXPECT_EQ(count1, 3);

	// Full world search
	bool test_failed2 = false;
	auto count2 = m.for_each_containing({-6.5f, -6.5f, 3.0f, 3.0f}, [=, &test_failed2]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
		if (id == id1) {
			if (value != value1) {
				test_failed2 = true;
			}
		} else if (id == id2) {
			return false;
		} else {
			test_failed2 = true;
		}
		return true;
	});
	EXPECT_FALSE(test_failed2);
	EXPECT_EQ(count2, 1);

	// In-map search
	bool test_failed3 = false;
	auto count3 = m.for_each_containing({-5.0f, -5.0f, 4.0f, 4.0f}, [=, &test_failed3]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
		if (id == id3) {
			if (value != value3) {
				test_failed3 = true;
			}
		} else {
			test_failed3 = true;
		}
		return true;
	});
	EXPECT_FALSE(test_failed3);
	EXPECT_EQ(count3, 1);

	// In-object search
	bool test_failed4 = false;
	auto count4 = m.for_each_containing({-0.25f, -0.25f, 0.2f, 0.2f}, [=, &test_failed4]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
		test_failed4 = true;
		return true;
	});
	EXPECT_FALSE(test_failed4);
	EXPECT_EQ(count4, 0);
}

TEST(MapF, for_each_intersecting) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};
	auto [value1, id1] = m.emplace({-6.0f, -6.0f, 0.5f, 0.5f}, 1);
	auto [value2, id2] = m.emplace({-5.25f, -5.25f, 0.5f, 0.5f}, 2);
	auto [value3, id3] = m.emplace({-4.5f, -4.5f, 0.5f, 0.5f}, 3);
	m.emplace({-0.5f, -0.5f, 0.5f, 0.5f}, 4);

	// Full world search
	bool test_failed1 = false;
	auto count1 = m.for_each_intersecting({-6.5f, -6.5f, 3.0f, 3.0f}, [=, &test_failed1]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
		if (id == id1) {
			if (value != value1) {
				test_failed1 = true;
			}
		} else if (id == id2) {
			if (value != value2) {
				test_failed1 = true;
			}
		} else if (id == id3) {
			if (value != value3) {
				test_failed1 = true;
			}
		} else {
			test_failed1 = true;
		}
		return true;
	});
	EXPECT_FALSE(test_failed1);
	EXPECT_EQ(count1, 3);

	// Full world search
	bool test_failed2 = false;
	auto count2 = m.for_each_intersecting({-6.5f, -6.5f, 3.0f, 3.0f}, [=, &test_failed2]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
		if (id == id1) {
			if (value != value1) {
				test_failed2 = true;
			}
		} else if (id == id2) {
			return false;
		} else {
			test_failed2 = true;
		}
		return true;
	});
	EXPECT_FALSE(test_failed2);
	EXPECT_EQ(count2, 1);

	// In-map search
	bool test_failed3 = false;
	auto count3 = m.for_each_intersecting({-5.0f, -5.0f, 3.0f, 3.0f}, [=, &test_failed3]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
		if (id == id2) {
			if (value != value2) {
				test_failed3 = true;
			}
		} else if (id == id3) {
			if (value != value3) {
				test_failed3 = true;
			}
		} else {
			test_failed3 = true;
		}
		return true;
	});
	EXPECT_FALSE(test_failed3);
	EXPECT_EQ(count3, 2);

	// In-object search
	bool test_failed4 = false;
	auto count4 = m.for_each_intersecting({-4.25f, -4.25f, 0.1f, 0.1f}, [=, &test_failed4]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
		if (id == id3) {
			if (value != value3) {
				test_failed4 = true;
			}
		} else {
			test_failed4 = true;
		}
		return true;
	});
	EXPECT_FALSE(test_failed4);
	EXPECT_EQ(count4, 1);
}

TEST(MapF, emplace) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};

	int value = 0;
	for (auto y = -6.0f; y < 6.0f; y += 0.1f) {
		for (auto x = -6.0f; x < 6.0f; x += 0.1f) {
			auto [obj, id] = m.emplace({x, y, 0.05f, 0.05f}, value);
			EXPECT_EQ(obj, value);
			EXPECT_NE(id, 0);
			value++;
		}
	}
}

TEST(MapF, erase) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};
	auto [obj1, id1] = m.emplace({-6.0f, -6.0f, 0.05f, 0.05f}, 1);
	auto [obj2, id2] = m.emplace({-5.0f, -5.0f, 0.05f, 0.05f}, 2);
	auto [obj3, id3] = m.emplace({-4.0f, -4.0f, 0.05f, 0.05f}, 3);
	auto [obj4, id4] = m.emplace({-3.0f, -3.0f, 0.05f, 0.05f}, 4);
	auto [obj5, id5] = m.emplace({-2.0f, -2.0f, 0.05f, 0.05f}, 5);
	auto [obj6, id6] = m.emplace({-1.0f, -1.0f, 0.05f, 0.05f}, 6);
	auto [obj7, id7] = m.emplace({0.0f, 0.0f, 0.05f, 0.05f}, 7);
	auto [obj8, id8] = m.emplace({1.0f, 1.0f, 0.05f, 0.05f}, 8);
	auto [obj9, id9] = m.emplace({2.0f, 2.0f, 0.05f, 0.05f}, 9);
	auto [obj10, id10] = m.emplace({3.0f, 3.0f, 0.05f, 0.05f}, 10);
	auto [obj11, id11] = m.emplace({4.0f, 4.0f, 0.05f, 0.05f}, 11);
	auto [obj12, id12] = m.emplace({5.0f, 5.0f, 0.05f, 0.05f}, 12);
	auto [obj13, id13] = m.emplace({6.0f, 6.0f, 0.05f, 0.05f}, 13);

	EXPECT_EQ(m.size(), 13);
	m.erase(id1);
	EXPECT_EQ(m.size(), 12);
	m.erase(id2);
	EXPECT_EQ(m.size(), 11);
	m.erase(id3);
	EXPECT_EQ(m.size(), 10);
	m.erase(id4);
	EXPECT_EQ(m.size(), 9);
	m.erase(id5);
	EXPECT_EQ(m.size(), 8);
	m.erase(id6);
	EXPECT_EQ(m.size(), 7);
	m.erase(id7);
	EXPECT_EQ(m.size(), 6);
	m.erase(id8);
	EXPECT_EQ(m.size(), 5);
	m.erase(id9);
	EXPECT_EQ(m.size(), 4);
	m.erase(id10);
	EXPECT_EQ(m.size(), 3);
	m.erase(id11);
	EXPECT_EQ(m.size(), 2);
	m.erase(id12);
	EXPECT_EQ(m.size(), 1);
	m.erase(id13);
	EXPECT_EQ(m.size(), 0);
}

TEST(MapF, move) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};

	int value = 0;
	for (auto y = -6.0f; y < 6.0f; y += 0.1f) {
		for (auto x = -6.0f; x < 6.0f; x += 0.1f) {
			auto [obj, id] = m.emplace({x, y, 0.05f, 0.05f}, value);
			m.move(id, {x - 1.0f, y + 1.0f, 0.5f, 0.5f});
			// TODO test move
			value++;
		}
	}
}

TEST(MapF, clear) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};

	int value = 0;
	for (auto y = -6.0f; y < 6.0f; y += 0.1f) {
		for (auto x = -6.0f; x < 6.0f; x += 0.1f) {
			m.emplace({x, y, 0.05f, 0.05f}, value);
			value++;
		}
	}

	m.clear();
	EXPECT_EQ(m.size(), 0);
}
