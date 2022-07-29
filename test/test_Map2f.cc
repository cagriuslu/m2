#include <gtest/gtest.h>
#include <m2/Map2f.h>

TEST(Map2f, basic) {
	fprintf(stderr, "A\n");
	auto* map = new m2::Map2f<int>();

	fprintf(stderr, "B\n");
	map->alloc(m2::Vec2f{});

	fprintf(stderr, "C\n");
}
