#include <gtest/gtest.h>
#include <m2/reflect/Composite.h>
#include <m2/reflect/Primitive.h>
#include <m2/reflect/utils/Hash.h>

using namespace m2;

namespace {
	enum class HashField { Number, Flag };
	using MyStruct = reflect::Struct<
		reflect::Field<HashField::Number, reflect::Int32>,
		reflect::Field<HashField::Flag, reflect::Bool>>;
}

TEST(ReflectHash, sameCompositeProducesSameHash) {
	MyStruct a;
	a.Mutate<HashField::Number>() = 5;
	a.Mutate<HashField::Flag>() = true;

	MyStruct b;
	b.Mutate<HashField::Number>() = 5;
	b.Mutate<HashField::Flag>() = true;

	EXPECT_EQ(reflect::util::Hash(a, 0), reflect::util::Hash(b, 0));

	// Hashing the same object twice is deterministic.
	EXPECT_EQ(reflect::util::Hash(a, 123), reflect::util::Hash(a, 123));
}

TEST(ReflectHash, changedFieldProducesDifferentHash) {
	MyStruct base;
	base.Mutate<HashField::Number>() = 5;
	base.Mutate<HashField::Flag>() = true;

	MyStruct changedNumber;
	changedNumber.Mutate<HashField::Number>() = 6;
	changedNumber.Mutate<HashField::Flag>() = true;

	EXPECT_NE(reflect::util::Hash(base, 0), reflect::util::Hash(changedNumber, 0));

	MyStruct changedFlag;
	changedFlag.Mutate<HashField::Number>() = 5;
	changedFlag.Mutate<HashField::Flag>() = false;

	EXPECT_NE(reflect::util::Hash(base, 0), reflect::util::Hash(changedFlag, 0));
}

TEST(ReflectHash, initialHashSeedInfluencesResult) {
	MyStruct s;
	s.Mutate<HashField::Number>() = 5;

	EXPECT_NE(reflect::util::Hash(s, 0), reflect::util::Hash(s, 1));
}
