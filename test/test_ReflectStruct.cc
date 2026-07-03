#include <gtest/gtest.h>
#include <m2/reflect/Composite.h>
#include <m2/reflect/Primitive.h>
#include <type_traits>

using namespace m2;

namespace {
	enum class StructField { Alpha, Beta };
	using MyStruct = reflect::Struct<
		reflect::Field<StructField::Alpha, reflect::Int32>,
		reflect::Field<StructField::Beta, reflect::Bool>>;

	enum class VariantAlt { First, Second };
	using MyVariant = reflect::Variant<
		reflect::Field<VariantAlt::First, reflect::Int32>,
		reflect::Field<VariantAlt::Second, reflect::Bool>>;
}

TEST(ReflectStruct, defaultsAreZeroInitialized) {
	MyStruct s;
	EXPECT_EQ(s.Get<StructField::Alpha>(), 0);
	EXPECT_FALSE(s.Get<StructField::Beta>());
}

TEST(ReflectStruct, getMutateRoundTrip) {
	MyStruct s;
	s.Mutate<StructField::Alpha>() = 42;
	s.Mutate<StructField::Beta>() = true;
	EXPECT_EQ(s.Get<StructField::Alpha>(), 42);
	EXPECT_TRUE(s.Get<StructField::Beta>());

	// Mutating one field does not disturb the other.
	s.Mutate<StructField::Alpha>() = -7;
	EXPECT_EQ(s.Get<StructField::Alpha>(), -7);
	EXPECT_TRUE(s.Get<StructField::Beta>());
}

TEST(ReflectVariant, defaultHoldsFirstAlternative) {
	MyVariant v;
	EXPECT_TRUE(v.HoldsAlternative<VariantAlt::First>());
	EXPECT_FALSE(v.HoldsAlternative<VariantAlt::Second>());
	ASSERT_NE(v.TryGet<VariantAlt::First>(), nullptr);
	EXPECT_EQ(*v.TryGet<VariantAlt::First>(), 0);
	EXPECT_EQ(v.TryGet<VariantAlt::Second>(), nullptr);
}

TEST(ReflectVariant, emplaceSwitchesAlternativeAndTryGetReflectsIt) {
	MyVariant v;

	v.Emplace<VariantAlt::Second>() = true;
	EXPECT_TRUE(v.HoldsAlternative<VariantAlt::Second>());
	EXPECT_FALSE(v.HoldsAlternative<VariantAlt::First>());
	ASSERT_NE(v.TryGet<VariantAlt::Second>(), nullptr);
	EXPECT_TRUE(*v.TryGet<VariantAlt::Second>());
	EXPECT_EQ(v.TryGet<VariantAlt::First>(), nullptr);

	v.Emplace<VariantAlt::First>() = 123;
	EXPECT_TRUE(v.HoldsAlternative<VariantAlt::First>());
	ASSERT_NE(v.TryGet<VariantAlt::First>(), nullptr);
	EXPECT_EQ(*v.TryGet<VariantAlt::First>(), 123);
	EXPECT_EQ(v.TryGet<VariantAlt::Second>(), nullptr);
}

TEST(ReflectVariant, visitDispatchesToActiveAlternative) {
	MyVariant v;
	v.Emplace<VariantAlt::First>() = 77;

	const int32_t result = v.Visit([](const auto& wrapper) -> int32_t {
		using WrapperType = std::decay_t<decltype(wrapper)>;
		if constexpr (std::is_same_v<WrapperType, reflect::Int32>) {
			return wrapper.Get();
		} else {
			return -1;
		}
	});
	EXPECT_EQ(result, 77);
}
