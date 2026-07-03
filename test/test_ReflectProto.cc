#include <gtest/gtest.h>
#include <m2/reflect/Composite.h>
#include <m2/reflect/Primitive.h>
#include <m2/reflect/utils/Proto.h>

using namespace m2;

namespace {
	enum class ProtoField { Number, Flag };
	using MyStruct = reflect::Struct<
		reflect::Field<ProtoField::Number, reflect::Int32>,
		reflect::Field<ProtoField::Flag, reflect::Bool>>;
}

TEST(ReflectProto, structSerializesFieldsIntoProto) {
	MyStruct s;
	s.Mutate<ProtoField::Number>() = 42;
	s.Mutate<ProtoField::Flag>() = true;

	pb::Reflective proto;
	reflect::util::StoreToProto(s, proto);

	// The root must be a struct.
	ASSERT_TRUE(proto.has_struct_());
	const auto& fields = proto.struct_().fields();

	// The field ids are the underlying values of the scoped enum.
	const auto numberKey = static_cast<int32_t>(ProtoField::Number);
	const auto flagKey = static_cast<int32_t>(ProtoField::Flag);

	ASSERT_TRUE(fields.contains(numberKey));
	EXPECT_TRUE(fields.at(numberKey).has_int32());
	EXPECT_EQ(fields.at(numberKey).int32(), 42);

	ASSERT_TRUE(fields.contains(flagKey));
	EXPECT_TRUE(fields.at(flagKey).has_bool_());
	EXPECT_TRUE(fields.at(flagKey).bool_());
}

TEST(ReflectProto, changedFieldReflectsInProto) {
	MyStruct s;
	s.Mutate<ProtoField::Number>() = -13;
	s.Mutate<ProtoField::Flag>() = false;

	pb::Reflective proto;
	reflect::util::StoreToProto(s, proto);

	const auto numberKey = static_cast<int32_t>(ProtoField::Number);
	const auto flagKey = static_cast<int32_t>(ProtoField::Flag);

	ASSERT_TRUE(proto.has_struct_());
	const auto& fields = proto.struct_().fields();
	ASSERT_TRUE(fields.contains(numberKey));
	EXPECT_EQ(fields.at(numberKey).int32(), -13);
	ASSERT_TRUE(fields.contains(flagKey));
	EXPECT_FALSE(fields.at(flagKey).bool_());
}
