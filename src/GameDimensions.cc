#include <m2/Game.h>

m2::Game::Dimensions::Dimensions(const Rational& game_height_m, int window_width, int window_height) : height_m(game_height_m) {
	window = SDL_Rect{0, 0, window_width, window_height};

	auto ideal_width = window_height * GAME_AND_HUD_ASPECT_RATIO_MUL / GAME_AND_HUD_ASPECT_RATIO_DIV;
	if (window_width < ideal_width) {
		// Screen is taller than expected
		auto provisional_game_height = window_width * GAME_AND_HUD_ASPECT_RATIO_DIV / GAME_AND_HUD_ASPECT_RATIO_MUL;
		ppm = provisional_game_height * height_m.d() / height_m.n();
	} else {
		// Screen is exact or wider
		ppm = window_height * height_m.d() / height_m.n();
	}

	int game_height = ppm * height_m.n() / height_m.d();
	int game_width = game_height * GAME_ASPECT_RATIO_MUL / GAME_ASPECT_RATIO_DIV;
	int hud_height = game_height;
	int hud_width = game_height * HUD_ASPECT_RATIO_MUL / HUD_ASPECT_RATIO_DIV;

	int top_envelope_size = (window_height - game_height) / 2;
	int bottom_envelope_size = (window_height - game_height) - top_envelope_size;
	int left_envelope_size = (window_width - game_width - 2 * hud_width) / 2;
	int right_envelope_size = (window_width - game_width - 2 * hud_width) - left_envelope_size;

	top_envelope = SDL_Rect{0, 0, window_width, top_envelope_size};
	bottom_envelope = SDL_Rect{0, top_envelope_size + game_height, window_width, bottom_envelope_size};
	left_envelope = SDL_Rect{0, top_envelope_size, left_envelope_size, game_height};
	right_envelope = SDL_Rect{left_envelope_size + 2 * hud_width + game_width, top_envelope_size, right_envelope_size, game_height};

	game_and_hud = SDL_Rect{left_envelope_size, top_envelope_size, 2 * hud_width + game_width, game_height};
	left_hud = SDL_Rect{left_envelope_size, top_envelope_size, hud_width, hud_height};
	right_hud = SDL_Rect{left_envelope_size + hud_width + game_width, top_envelope_size, hud_width, hud_height};
	game = SDL_Rect{left_envelope_size + hud_width, top_envelope_size, game_width, game_height};

	console = SDL_Rect{game.x, game.y + game.h * 22 / 24, game.w, game.h * 2 / 24};
}
