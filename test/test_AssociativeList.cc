#include <gtest/gtest.h>
#include <m2/containers/AssociativeList.h>

TEST(AssociativeList, basic) {
	m2::AssociativeList<int, float> myList;
	EXPECT_EQ(myList.empty(), true);
	EXPECT_EQ(myList.size(), 0);
	EXPECT_EQ(myList.cbegin(), myList.cend());
	EXPECT_EQ(myList.begin(), myList.end());

	EXPECT_EQ(myList.emplace(12, 12.5f), true);
	EXPECT_EQ(*myList.cbegin(), (std::pair{12, 12.5f}));
	EXPECT_EQ(++myList.cbegin(), myList.cend());
	EXPECT_EQ(*myList.find(12), (std::pair{12, 12.5f}));
	EXPECT_EQ(myList.contains(12), true);
	EXPECT_EQ(myList.empty(), false);
	EXPECT_EQ(myList.size(), 1);

	EXPECT_EQ(myList.emplace(12, 15.5f), false);
	EXPECT_EQ(*myList.cbegin(), (std::pair{12, 12.5f}));
	EXPECT_EQ(++myList.cbegin(), myList.cend());
	EXPECT_EQ(*myList.find(12), (std::pair{12, 12.5f}));
	EXPECT_EQ(myList.contains(12), true);
	EXPECT_EQ(myList.empty(), false);
	EXPECT_EQ(myList.size(), 1);

	EXPECT_EQ(myList.emplace(17, 17.5f), true);
	EXPECT_EQ(*myList.cbegin(), (std::pair{12, 12.5f}));
	EXPECT_EQ(*++myList.cbegin(), (std::pair{17, 17.5f}));
	EXPECT_EQ(++++myList.cbegin(), myList.cend());
	EXPECT_EQ(*myList.find(12), (std::pair{12, 12.5f}));
	EXPECT_EQ(*myList.find(17), (std::pair{17, 17.5f}));
	EXPECT_EQ(myList.contains(12), true);
	EXPECT_EQ(myList.contains(17), true);
	EXPECT_EQ(myList.empty(), false);
	EXPECT_EQ(myList.size(), 2);

	EXPECT_EQ(*myList.erase(myList.begin()), (std::pair{17, 17.5f}));
	EXPECT_EQ(*myList.cbegin(), (std::pair{17, 17.5f}));
	EXPECT_EQ(++myList.cbegin(), myList.cend());
	EXPECT_EQ(myList.find(12), myList.end());
	EXPECT_EQ(*myList.find(17), (std::pair{17, 17.5f}));
	EXPECT_EQ(myList.contains(12), false);
	EXPECT_EQ(myList.contains(17), true);
	EXPECT_EQ(myList.empty(), false);
	EXPECT_EQ(myList.size(), 1);

	myList.clear();
	EXPECT_EQ(myList.empty(), true);
	EXPECT_EQ(myList.size(), 0);
	EXPECT_EQ(myList.cbegin(), myList.cend());
	EXPECT_EQ(myList.begin(), myList.end());

	EXPECT_EQ(myList.emplace(1, 1.5f), true);
	EXPECT_EQ(myList.emplace(1, 1.5f), false);
	EXPECT_EQ(myList.emplace(2, 2.5f), true);
	EXPECT_EQ(myList.emplace(3, 3.5f), true);
	EXPECT_EQ(myList.emplace(4, 4.5f), true);
	EXPECT_EQ(myList.emplace(5, 5.5f), true);
	EXPECT_EQ(myList.emplace(6, 6.5f), true);
	EXPECT_EQ(myList.emplace(7, 7.5f), true);
	EXPECT_EQ(myList.emplace(8, 8.5f), true);
	EXPECT_EQ(myList.emplace(9, 9.5f), true);
	EXPECT_EQ(myList.emplace(10, 10.5f), true);
	EXPECT_EQ(myList.emplace(11, 11.5f), true);
	EXPECT_EQ(myList.find(12), myList.end());
	EXPECT_EQ(*myList.find(10), (std::pair{10, 10.5f}));
	EXPECT_EQ(myList.contains(12), false);
	EXPECT_EQ(myList.contains(10), true);
	EXPECT_EQ(myList.empty(), false);
	EXPECT_EQ(myList.size(), 11);

	myList.clear();
	EXPECT_EQ(myList.empty(), true);
	EXPECT_EQ(myList.size(), 0);
	EXPECT_EQ(myList.cbegin(), myList.cend());
	EXPECT_EQ(myList.begin(), myList.end());
}

TEST(AssociativeList, ranges) {
	m2::AssociativeList<int, float> myList;
	std::ranges::for_each(myList.begin(), myList.end(), [](const auto&) {});
}
