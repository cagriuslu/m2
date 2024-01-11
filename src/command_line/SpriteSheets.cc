#include <m2/command_line/SpriteSheets.h>
#include <Sprite.pb.h>
#include <m2g_SpriteType.pb.h>
#include <m2/protobuf/Detail.h>

std::string m2::generate_sprite_sheets_skeleton() {
	pb::SpriteSheets ss;
	auto* sheet = ss.add_sheets();
	sheet->set___comment__("Auto-generated");

	pb::for_each_enum_value<m2g::pb::SpriteType>([&sheet](m2g::pb::SpriteType type) {
		sheet->add_sprites()->set_type(type);
	});

	return *pb::message_to_json_string(ss);
}
