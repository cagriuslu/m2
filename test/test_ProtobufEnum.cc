#include <gtest/gtest.h>
#include <m2/protobuf/Detail.h>
#include <Enums.pb.h>

#include <format>
#include <vector>

using namespace m2;

// m2::pb::LogLevel is a simple, sequential proto enum: NO_LOG_LEVEL=0, TRC=1, ..., FTL=9.

TEST(ProtobufEnum, ValueCount) {
	// There are 10 declared values (NO_LOG_LEVEL through FTL).
	EXPECT_EQ(pb::enum_value_count<pb::LogLevel>(), 10);
}

TEST(ProtobufEnum, IndexValueNameRoundTrip) {
	// value -> index -> value
	EXPECT_EQ(pb::enum_value<pb::LogLevel>(pb::enum_index(pb::TRC)), pb::TRC);
	EXPECT_EQ(pb::enum_value<pb::LogLevel>(pb::enum_index(pb::FTL)), pb::FTL);

	// index -> value (the values are declared in increasing order starting at index 0)
	EXPECT_EQ(pb::enum_value<pb::LogLevel>(0), pb::NO_LOG_LEVEL);
	EXPECT_EQ(pb::enum_value<pb::LogLevel>(1), pb::TRC);

	// enum_name via value and via index should agree.
	EXPECT_EQ(pb::enum_name(pb::TRC), "TRC");
	EXPECT_EQ(pb::enum_name(pb::NO_LOG_LEVEL), "NO_LOG_LEVEL");
	EXPECT_EQ(pb::enum_name<pb::LogLevel>(pb::enum_index(pb::FTL)), "FTL");
}

TEST(ProtobufEnum, ForEachEnumValue) {
	std::vector<pb::LogLevel> collected;
	pb::for_each_enum_value<pb::LogLevel>([&collected](const pb::LogLevel value) { collected.push_back(value); });

	ASSERT_EQ(collected.size(), 10u);
	// The i-th visited value must equal enum_value<LogLevel>(i).
	for (int i = 0; i < static_cast<int>(collected.size()); ++i) {
		EXPECT_EQ(collected[i], pb::enum_value<pb::LogLevel>(i));
	}
	EXPECT_EQ(collected.front(), pb::NO_LOG_LEVEL);
	EXPECT_EQ(collected.back(), pb::FTL);
}

TEST(ProtobufEnum, StdFormatterYieldsEnumName) {
	// std::formatter<ProtoEnum> formats a proto enum value to its declared name.
	EXPECT_EQ(std::format("{}", pb::TRC), "TRC");
	EXPECT_EQ(std::format("{}", pb::FTL), "FTL");
	EXPECT_EQ(std::format("level={}", pb::NO_LOG_LEVEL), "level=NO_LOG_LEVEL");
}
