#include <gtest/gtest.h>
#include <m2/containers/MapF.h>
#include <m2/math/RectF.h>

TEST(MapF, size) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};

	int value = 0;
	for (auto y = -6.0f; y < 6.0f; y += 0.1f) {
		for (auto x = -6.0f; x < 6.0f; x += 0.1f) {
			auto [obj, id] = m.Emplace({x, y, 0.05f, 0.05f}, value);
			EXPECT_EQ(obj, value);
			EXPECT_NE(id, 0);
			value++;
		}
	}
	EXPECT_EQ(m.Size(), 14641);
}

TEST(MapF, contains) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};
	auto [obj, id] = m.Emplace({}, 5);
	EXPECT_TRUE(m.Contains(id));

	auto [obj2, id2] = m.Emplace({1.0f, 1.0f, 0.0f, 0.0f}, 10);
	EXPECT_TRUE(m.Contains(id2));
}

TEST(MapF, get) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};
	auto [obj, id] = m.Emplace({}, 5);
	EXPECT_EQ(*m.Get(id), 5);

	auto [obj2, id2] = m.Emplace({1.0f, 1.0f, 0.0f, 0.0f}, 10);
	EXPECT_EQ(*m.Get(id2), 10);
}

TEST(MapF, access) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};
	auto [obj, id] = m.Emplace({}, 5);
	EXPECT_EQ(m[id], 5);

	auto [obj2, id2] = m.Emplace({1.0f, 1.0f, 0.0f, 0.0f}, 10);
	EXPECT_EQ(m[id2], 10);
}

TEST(MapF, area) {
	auto area = m2::RectF{-5.0f, -5.0f, 10.0f, 10.0f};
	m2::MapF<int> m(area);
	EXPECT_TRUE(m.Area().equals(area));
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
			auto [_, id] = m.Emplace(rect, value);

			if (rect.equals(expect_rect)) {
				expect_value = value;
				expect_id = id;
			}

			value++;
		}
	}

	bool item_found = false;
	m.ForEach([&item_found, expect_rect, expect_value, expect_id](const m2::RectF& area, m2::Id id, int i) {
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
	auto [value1, id1] = m.Emplace({-6.0f, -6.0f, 0.5f, 0.5f}, 1);
	auto [value2, id2] = m.Emplace({-5.25f, -5.25f, 0.5f, 0.5f}, 2);
	auto [value3, id3] = m.Emplace({-4.5f, -4.5f, 0.5f, 0.5f}, 3);
	m.Emplace({-0.5f, -0.5f, 0.5f, 0.5f}, 4);

	// Full world search
	bool test_failed1 = false;
	auto count1 = m.ForEachContaining({-6.5f, -6.5f, 3.0f, 3.0f}, [=, &test_failed1]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
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
	auto count2 = m.ForEachContaining({-6.5f, -6.5f, 3.0f, 3.0f}, [=, &test_failed2]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
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
	auto count3 = m.ForEachContaining({-5.0f, -5.0f, 4.0f, 4.0f}, [=, &test_failed3]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
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
	auto count4 = m.ForEachContaining({-0.25f, -0.25f, 0.2f, 0.2f}, [=, &test_failed4]([[maybe_unused]] const m2::RectF& r, m2::Id, int&) -> bool {
		test_failed4 = true;
		return true;
	});
	EXPECT_FALSE(test_failed4);
	EXPECT_EQ(count4, 0);
}

TEST(MapF, for_each_intersecting) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};
	auto [value1, id1] = m.Emplace({-6.0f, -6.0f, 0.5f, 0.5f}, 1);
	auto [value2, id2] = m.Emplace({-5.25f, -5.25f, 0.5f, 0.5f}, 2);
	auto [value3, id3] = m.Emplace({-4.5f, -4.5f, 0.5f, 0.5f}, 3);
	m.Emplace({-0.5f, -0.5f, 0.5f, 0.5f}, 4);

	// Full world search
	bool test_failed1 = false;
	auto count1 = m.ForEachIntersecting({-6.5f, -6.5f, 3.0f, 3.0f}, [=, &test_failed1]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
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
	auto count2 = m.ForEachIntersecting({-6.5f, -6.5f, 3.0f, 3.0f}, [=, &test_failed2]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
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
	auto count3 = m.ForEachIntersecting({-5.0f, -5.0f, 3.0f, 3.0f}, [=, &test_failed3]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
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
	auto count4 = m.ForEachIntersecting({-4.25f, -4.25f, 0.1f, 0.1f}, [=, &test_failed4]([[maybe_unused]] const m2::RectF& r, m2::Id id, int& value) -> bool {
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
			auto [obj, id] = m.Emplace({x, y, 0.05f, 0.05f}, value);
			EXPECT_EQ(obj, value);
			EXPECT_NE(id, 0);
			value++;
		}
	}
}

TEST(MapF, erase) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};
	auto [obj1, id1] = m.Emplace({-6.0f, -6.0f, 0.05f, 0.05f}, 1);
	auto [obj2, id2] = m.Emplace({-5.0f, -5.0f, 0.05f, 0.05f}, 2);
	auto [obj3, id3] = m.Emplace({-4.0f, -4.0f, 0.05f, 0.05f}, 3);
	auto [obj4, id4] = m.Emplace({-3.0f, -3.0f, 0.05f, 0.05f}, 4);
	auto [obj5, id5] = m.Emplace({-2.0f, -2.0f, 0.05f, 0.05f}, 5);
	auto [obj6, id6] = m.Emplace({-1.0f, -1.0f, 0.05f, 0.05f}, 6);
	auto [obj7, id7] = m.Emplace({0.0f, 0.0f, 0.05f, 0.05f}, 7);
	auto [obj8, id8] = m.Emplace({1.0f, 1.0f, 0.05f, 0.05f}, 8);
	auto [obj9, id9] = m.Emplace({2.0f, 2.0f, 0.05f, 0.05f}, 9);
	auto [obj10, id10] = m.Emplace({3.0f, 3.0f, 0.05f, 0.05f}, 10);
	auto [obj11, id11] = m.Emplace({4.0f, 4.0f, 0.05f, 0.05f}, 11);
	auto [obj12, id12] = m.Emplace({5.0f, 5.0f, 0.05f, 0.05f}, 12);
	auto [obj13, id13] = m.Emplace({6.0f, 6.0f, 0.05f, 0.05f}, 13);

	EXPECT_EQ(m.Size(), 13);
	m.Erase(id1);
	EXPECT_EQ(m.Size(), 12);
	m.Erase(id2);
	EXPECT_EQ(m.Size(), 11);
	m.Erase(id3);
	EXPECT_EQ(m.Size(), 10);
	m.Erase(id4);
	EXPECT_EQ(m.Size(), 9);
	m.Erase(id5);
	EXPECT_EQ(m.Size(), 8);
	m.Erase(id6);
	EXPECT_EQ(m.Size(), 7);
	m.Erase(id7);
	EXPECT_EQ(m.Size(), 6);
	m.Erase(id8);
	EXPECT_EQ(m.Size(), 5);
	m.Erase(id9);
	EXPECT_EQ(m.Size(), 4);
	m.Erase(id10);
	EXPECT_EQ(m.Size(), 3);
	m.Erase(id11);
	EXPECT_EQ(m.Size(), 2);
	m.Erase(id12);
	EXPECT_EQ(m.Size(), 1);
	m.Erase(id13);
	EXPECT_EQ(m.Size(), 0);
}

TEST(MapF, move) {
	m2::MapF<int> m{-5.0f, -5.0f, 10.0f, 10.0f};

	int value = 0;
	for (auto y = -6.0f; y < 6.0f; y += 0.1f) {
		for (auto x = -6.0f; x < 6.0f; x += 0.1f) {
			auto [obj, id] = m.Emplace({x, y, 0.05f, 0.05f}, value);
			m.Move(id, {x - 1.0f, y + 1.0f, 0.5f, 0.5f});
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
			m.Emplace({x, y, 0.05f, 0.05f}, value);
			value++;
		}
	}

	m.Clear();
	EXPECT_EQ(m.Size(), 0);
}
