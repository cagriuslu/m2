#include <gtest/gtest.h>
#include <m2/common/String.h>

using namespace m2;

TEST(String, SplitString) {
	EXPECT_EQ(SplitString("a,b,c", ','), (std::vector<std::string>{"a", "b", "c"}));
	EXPECT_EQ(SplitString("abc", ','), (std::vector<std::string>{"abc"}));
	EXPECT_EQ(SplitString("a,,b", ','), (std::vector<std::string>{"a", "", "b"}));
	EXPECT_EQ(SplitString("a,", ','), (std::vector<std::string>{"a", ""}));
	EXPECT_EQ(SplitString("", ','), (std::vector<std::string>{""}));
}

TEST(String, Trim) {
	EXPECT_EQ(TrimLeft("  hi"), "hi");
	EXPECT_EQ(TrimLeft("hi  "), "hi  ");
	EXPECT_EQ(TrimRight("hi  "), "hi");
	EXPECT_EQ(TrimRight("  hi"), "  hi");
	EXPECT_EQ(Trim("  hi  "), "hi");
	EXPECT_EQ(Trim("hi"), "hi");
	EXPECT_EQ(Trim("   "), "");
}

TEST(String, GetTrimmedView) {
	EXPECT_EQ(GetTrimmedView(std::string{"  hello  "}), "hello");
	EXPECT_EQ(GetTrimmedView(std::string{"world"}), "world");
	EXPECT_TRUE(GetTrimmedView(std::string{"   "}).empty());
}

TEST(String, CompileTimeString) {
	constexpr CompileTimeString cts{"hello"};
	static_assert(cts.GetSize() == 5);
	static_assert(cts.Find('h') == 0);
	static_assert(cts.Find('l') == 2);
	static_assert(cts.Find('z') == -1);

	EXPECT_EQ(cts.GetSize(), 5);
	EXPECT_EQ(cts[0], 'h');
	EXPECT_EQ(cts.Find('o'), 4);
}
