#include <gtest/gtest.h>
#include <m2/reflect/Container.h>
#include <m2/reflect/Primitive.h>

using namespace m2;

TEST(ReflectContainer, vectorAddIterateAndSize) {
	reflect::Vector<reflect::Int32> vec;
	EXPECT_TRUE(vec.Get().empty());

	vec.Mutate().push_back(reflect::Int32{10});
	vec.Mutate().push_back(reflect::Int32{20});
	vec.Mutate().push_back(reflect::Int32{30});

	ASSERT_EQ(vec.Get().size(), 3u);
	EXPECT_EQ(vec.Get()[0].Get(), 10);
	EXPECT_EQ(vec.Get()[1].Get(), 20);
	EXPECT_EQ(vec.Get()[2].Get(), 30);

	int32_t sum = 0;
	for (const auto& element : vec.Get()) {
		sum += element.Get();
	}
	EXPECT_EQ(sum, 60);
}

TEST(ReflectContainer, dequeAddAndSize) {
	reflect::Deque<reflect::Int32> deque;
	deque.Mutate().push_back(reflect::Int32{1});
	deque.Mutate().push_back(reflect::Int32{2});

	ASSERT_EQ(deque.Get().size(), 2u);
	EXPECT_EQ(deque.Get()[0].Get(), 1);
	EXPECT_EQ(deque.Get()[1].Get(), 2);
}

TEST(ReflectContainer, listAddAndIterate) {
	reflect::List<reflect::Int32> list;
	list.Mutate().push_back(reflect::Int32{7});
	list.Mutate().push_back(reflect::Int32{8});

	ASSERT_EQ(list.Get().size(), 2u);
	EXPECT_EQ(list.Get().front().Get(), 7);
	EXPECT_EQ(list.Get().back().Get(), 8);

	int32_t sum = 0;
	for (const auto& element : list.Get()) {
		sum += element.Get();
	}
	EXPECT_EQ(sum, 15);
}
