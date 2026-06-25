#include <m2/thirdparty/video/Cursor.h>
#include <SDL3/SDL_mouse.h>
#include <format>

m2::expected<m2::thirdparty::video::Cursor> m2::thirdparty::video::Cursor::Create() {
	auto str =
		"                                "
		"                                "
		"             XXXXX              "
		"             X...X              "
		"             X.X.X              "
		"             X.X.X              "
		"             X.X.X              "
		"             X.X.X              "
		"             X.X.X              "
		"             X.X.X              "
		"             X...X              "
		"             XXXXX              "
		"                                "
		"  XXXXXXXXXX XXXXX XXXXXXXXXX   "
		"  X........X X...X X........X   "
		"  X.XXXXXX.X X.X.X X.XXXXXX.X   "
		"  X........X X...X X........X   "
		"  XXXXXXXXXX XXXXX XXXXXXXXXX   "
		"                                "
		"             XXXXX              "
		"             X...X              "
		"             X.X.X              "
		"             X.X.X              "
		"             X.X.X              "
		"             X.X.X              "
		"             X.X.X              "
		"             X.X.X              "
		"             X...X              "
		"             XXXXX              "
		"                                "
		"                                "
		"                                "
	;

	uint8_t data[128] = {};
	uint8_t mask[128] = {};
	constexpr unsigned side_size = 32;

	for (unsigned y = 0; y < side_size; y++) {
		for (unsigned x = 0; x < side_size; x++) {
			const unsigned index = y * side_size + x;
			if (const char c = str[index]; c == 'X') {
				data[index / 8] |= 0x80 >> (index % 8);
				mask[index / 8] |= 0x80 >> (index % 8);
			} else if (c == '.') {
				mask[index / 8] |= 0x80 >> (index % 8);
			}
		}
	}

	auto* cursor = SDL_CreateCursor(data, mask, side_size, side_size, side_size / 2 - 1, side_size / 2 - 1);
	if (not cursor) {
		return make_unexpected(std::format("SDL_CreateCursor failure: {}", SDL_GetError()));
	}
	return Cursor{cursor};
}

m2::thirdparty::video::Cursor::Cursor(Cursor&& other) noexcept : _cursor(other._cursor) {
	other._cursor = nullptr;
}
m2::thirdparty::video::Cursor& m2::thirdparty::video::Cursor::operator=(Cursor&& other) noexcept {
	std::swap(_cursor, other._cursor);
	return *this;
}
m2::thirdparty::video::Cursor::~Cursor() {
	if (_cursor) {
		SDL_DestroyCursor(static_cast<SDL_Cursor*>(_cursor));
		_cursor = nullptr;
	}
}

void m2::thirdparty::video::Cursor::Load() const {
	SDL_SetCursor(static_cast<SDL_Cursor*>(_cursor));
}
