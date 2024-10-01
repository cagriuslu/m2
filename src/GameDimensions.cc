#include <m2/Game.h>

constexpr int GAME_AND_HUD_ASPECT_RATIO_MUL = 16;
constexpr int GAME_AND_HUD_ASPECT_RATIO_DIV = 9;

namespace {
	constexpr int hud_aspect_ratio_mul(int game_aspect_ratio_mul, int game_aspect_ratio_div) {
		return (GAME_AND_HUD_ASPECT_RATIO_MUL * game_aspect_ratio_div - game_aspect_ratio_mul * GAME_AND_HUD_ASPECT_RATIO_DIV);
	}

	constexpr int hud_aspect_ratio_div(int game_aspect_ratio_div) {
		return (GAME_AND_HUD_ASPECT_RATIO_DIV * game_aspect_ratio_div * 2);
	}
}

m2::Game::Dimensions::Dimensions(int game_height_m, int window_width, int window_height, int game_aspect_ratio_mul, int game_aspect_ratio_div) : height_m(game_height_m), width_m(static_cast<float>(height_m * game_aspect_ratio_mul) / static_cast<float>(game_aspect_ratio_div)) {
	window = RectI{0, 0, window_width, window_height};

	auto ideal_width = window_height * GAME_AND_HUD_ASPECT_RATIO_MUL / GAME_AND_HUD_ASPECT_RATIO_DIV;
	if (window_width < ideal_width) {
		// Screen is taller than expected
		auto provisional_game_height = window_width * GAME_AND_HUD_ASPECT_RATIO_DIV / GAME_AND_HUD_ASPECT_RATIO_MUL;
		ppm = provisional_game_height / height_m;
	} else {
		// Screen is exact or wider
		ppm = window_height / height_m;
	}

	int game_height = ppm * height_m;
	int game_width = game_height * game_aspect_ratio_mul / game_aspect_ratio_div;
	int hud_height = game_height;
	int hud_width = game_height * hud_aspect_ratio_mul(game_aspect_ratio_mul, game_aspect_ratio_div) / hud_aspect_ratio_div(game_aspect_ratio_div);

	int top_envelope_size = (window_height - game_height) / 2;
	int bottom_envelope_size = (window_height - game_height) - top_envelope_size;
	int left_envelope_size = (window_width - game_width - 2 * hud_width) / 2;
	int right_envelope_size = (window_width - game_width - 2 * hud_width) - left_envelope_size;

	top_envelope = RectI{0, 0, window_width, top_envelope_size};
	bottom_envelope = RectI{0, top_envelope_size + game_height, window_width, bottom_envelope_size};
	left_envelope = RectI{0, top_envelope_size, left_envelope_size, game_height};
	right_envelope = RectI{left_envelope_size + 2 * hud_width + game_width, top_envelope_size, right_envelope_size, game_height};

	game_and_hud = RectI{left_envelope_size, top_envelope_size, 2 * hud_width + game_width, game_height};
	left_hud = RectI{left_envelope_size, top_envelope_size, hud_width, hud_height};
	right_hud = RectI{left_envelope_size + hud_width + game_width, top_envelope_size, hud_width, hud_height};
	game = RectI{left_envelope_size + hud_width, top_envelope_size, game_width, game_height};

	auto message_box_height = game.h / 25;
	message_box = RectI{game.x, game.y + game.h - message_box_height, game.w, message_box_height};
}
