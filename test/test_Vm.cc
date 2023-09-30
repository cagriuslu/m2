#include <gtest/gtest.h>
#include <m2/Vm.h>
#include <m2/Options.h>

TEST(Vm, basic) {
	m2::current_log_level = m2::pb::LogLevel::TRC;

	m2::Vm vm;
	auto expect = vm.add_script(";main `3 `5 +");
	EXPECT_TRUE(expect);
}

TEST(Vm, addition) {
	m2::Vm vm;
	vm.add_script(";main `3 `5 +");
	auto return_value = vm.execute("main");
	EXPECT_EQ(std::get<int>(*return_value), 8);
}

TEST(Vm, subtraction) {
	m2::Vm vm;
	vm.add_script(";main `5 `3 -");
	auto return_value = vm.execute("main");
	EXPECT_EQ(std::get<int>(*return_value), 2);
}
