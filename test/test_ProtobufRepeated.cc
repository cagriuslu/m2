#include <gtest/gtest.h>
#include <m2/protobuf/Detail.h>
#include <Key.pb.h>
#include <Level.pb.h>

using namespace m2;

// --- mutable_get_or_create on a scalar RepeatedField (Key.sdl_scancodes is repeated int32) ---

TEST(ProtobufRepeated, GetOrCreateScalarGrowsToIndex) {
	pb::Key key;
	ASSERT_EQ(key.sdl_scancodes_size(), 0);

	int* element = pb::mutable_get_or_create(key.mutable_sdl_scancodes(), 3);
	// The field grows so that the requested index becomes valid.
	ASSERT_EQ(key.sdl_scancodes_size(), 4);
	*element = 99;
	EXPECT_EQ(key.sdl_scancodes(3), 99);
	// Elements before the requested index are default-filled.
	EXPECT_EQ(key.sdl_scancodes(0), 0);
	EXPECT_EQ(key.sdl_scancodes(1), 0);
	EXPECT_EQ(key.sdl_scancodes(2), 0);
}

TEST(ProtobufRepeated, GetOrCreateScalarUsesFillValue) {
	pb::Key key;
	pb::mutable_get_or_create(key.mutable_sdl_scancodes(), 5, 7);
	ASSERT_EQ(key.sdl_scancodes_size(), 6);
	for (int i = 0; i < key.sdl_scancodes_size(); ++i) {
		EXPECT_EQ(key.sdl_scancodes(i), 7);
	}
}

TEST(ProtobufRepeated, GetOrCreateScalarReturnsExistingWithoutGrowing) {
	pb::Key key;
	key.add_sdl_scancodes(10);
	key.add_sdl_scancodes(20);
	key.add_sdl_scancodes(30);

	int* element = pb::mutable_get_or_create(key.mutable_sdl_scancodes(), 1);
	// Index already exists: size is unchanged and the existing element is returned.
	EXPECT_EQ(key.sdl_scancodes_size(), 3);
	EXPECT_EQ(*element, 20);
	*element = 25;
	EXPECT_EQ(key.sdl_scancodes(1), 25);
}

// --- mutable_get_or_create / mutable_insert on a message RepeatedPtrField (Level.objects) ---

TEST(ProtobufRepeated, GetOrCreateMessageGrowsToIndex) {
	pb::Level level;
	ASSERT_EQ(level.objects_size(), 0);

	pb::Object* object = pb::mutable_get_or_create(level.mutable_objects(), 2);
	ASSERT_EQ(level.objects_size(), 3);
	object->set_orientation(1.5f);
	EXPECT_FLOAT_EQ(level.objects(2).orientation(), 1.5f);
}

TEST(ProtobufRepeated, InsertShiftsSubsequentElements) {
	pb::Level level;
	level.add_objects()->set_orientation(1.0f);
	level.add_objects()->set_orientation(2.0f);
	level.add_objects()->set_orientation(3.0f);

	// Insert right before index 1.
	pb::Object* inserted = pb::mutable_insert(level.mutable_objects(), 1);
	ASSERT_NE(inserted, nullptr);
	inserted->set_orientation(1.5f);

	ASSERT_EQ(level.objects_size(), 4);
	EXPECT_FLOAT_EQ(level.objects(0).orientation(), 1.0f);
	EXPECT_FLOAT_EQ(level.objects(1).orientation(), 1.5f);  // newly inserted
	EXPECT_FLOAT_EQ(level.objects(2).orientation(), 2.0f);  // shifted
	EXPECT_FLOAT_EQ(level.objects(3).orientation(), 3.0f);  // shifted
}

TEST(ProtobufRepeated, InsertAtEndAppends) {
	pb::Level level;
	level.add_objects()->set_orientation(1.0f);
	level.add_objects()->set_orientation(2.0f);

	// index == size is allowed and appends at the end.
	pb::Object* inserted = pb::mutable_insert(level.mutable_objects(), 2);
	ASSERT_NE(inserted, nullptr);
	inserted->set_orientation(3.0f);

	ASSERT_EQ(level.objects_size(), 3);
	EXPECT_FLOAT_EQ(level.objects(0).orientation(), 1.0f);
	EXPECT_FLOAT_EQ(level.objects(1).orientation(), 2.0f);
	EXPECT_FLOAT_EQ(level.objects(2).orientation(), 3.0f);
}

TEST(ProtobufRepeated, InsertOutOfBoundsReturnsNull) {
	pb::Level level;
	level.add_objects()->set_orientation(1.0f);

	// index > size is out-of-bounds.
	pb::Object* inserted = pb::mutable_insert(level.mutable_objects(), 5);
	EXPECT_EQ(inserted, nullptr);
	EXPECT_EQ(level.objects_size(), 1);
}
