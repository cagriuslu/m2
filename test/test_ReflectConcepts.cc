#include <gtest/gtest.h>
#include <m2/common/reflect/Detail.h>
#include <m2/reflect/Primitive.h>

using namespace m2::reflect;

namespace {
	enum class ScopedField { Alpha, Beta };
	enum PlainEnum { PlainA, PlainB };

	using AlphaField = Field<ScopedField::Alpha, Int32>;

	// IsValuePackUnique: true only when every element is pairwise distinct.
	static_assert(IsValuePackUnique(1));
	static_assert(IsValuePackUnique(1, 2, 3));
	static_assert(IsValuePackUnique('a', 'b', 'c'));
	static_assert(not IsValuePackUnique(1, 2, 1));
	static_assert(not IsValuePackUnique(7, 7));

	// IsDerivedFromField: only true for Field specializations.
	static_assert(IsDerivedFromField<AlphaField>::Value);
	static_assert(not IsDerivedFromField<int>::Value);
	static_assert(not IsDerivedFromField<Int32>::Value); // A reflective type, but not a Field.

	// Enum classification concepts.
	static_assert(IsScopedEnum<ScopedField>);
	static_assert(not IsScopedEnum<PlainEnum>);
	static_assert(not IsScopedEnum<int>);
	static_assert(IsPlainEnum<PlainEnum>);
	static_assert(not IsPlainEnum<ScopedField>);
	static_assert(IsEnum<ScopedField>);
	static_assert(IsEnum<PlainEnum>);
	static_assert(not IsEnum<int>);

	// Reflective concepts.
	static_assert(IsPrimitiveReflective<Int32>);
	static_assert(IsPrimitiveReflective<Bool>);
	static_assert(not IsPrimitiveReflective<int>);
	static_assert(IsReflective<Int32>);
	static_assert(IsReflective<Bool>);
	static_assert(not IsReflective<int>);
}

TEST(ReflectConcepts, CompileTimeChecks) {
	// The definitive assertions are the file-scope static_asserts above; mirror the load-bearing ones here so the
	// suite is non-empty and the results are visible at runtime.
	EXPECT_TRUE((IsDerivedFromField<AlphaField>::Value));
	EXPECT_FALSE((IsDerivedFromField<int>::Value));
	EXPECT_TRUE(IsValuePackUnique(1, 2, 3));
	EXPECT_FALSE(IsValuePackUnique(1, 2, 1));
}
