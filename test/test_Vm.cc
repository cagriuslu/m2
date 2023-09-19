#include <gtest/gtest.h>
#include <m2/Vm.h>
#include <m2/Options.h>

TEST(Vm, basic) {
	using namespace m2;

	current_log_level = pb::LogLevel::TRC;

	Vm vm;
	auto expect = vm.add_script(";vmMain `1 `2 + =");
	EXPECT_TRUE(expect);
}
