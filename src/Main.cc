#include <m2g/Proxy.h>
#include "m2/Events.h"
#include "m2/Game.h"
#include "m2/sdl/Utils.hh"
#include <m2g/Ui.h>
#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <m2/ThreadPool.h>
#include <m2/Log.h>
#include <m2/Map2i.h>
#include <cstdlib>

using namespace m2;

int main(int argc, char **argv) {
	DEBUG_FN();

	if (b2_version.major != 2 || b2_version.minor != 4 || b2_version.revision != 0) {
		throw M2FATAL("Box2D version mismatch");
	}

	// Process command line arguments
	for (int i = 1; i < argc; i++) {
		const char* loglevel = "--log-lvl=";
		const size_t loglevelStrlen = strlen(loglevel);
		if (strncmp(argv[i], loglevel, loglevelStrlen) == 0) {
			if (strcmp(argv[i] + loglevelStrlen, "trace") == 0) {
				m2::current_log_level = m2::LogLevel::Trace;
			} else if (strcmp(argv[i] + loglevelStrlen, "debug") == 0) {
				m2::current_log_level = m2::LogLevel::Debug;
			} else if (strcmp(argv[i] + loglevelStrlen, "info") == 0) {
				m2::current_log_level = m2::LogLevel::Info;
			} else if (strcmp(argv[i] + loglevelStrlen, "warning") == 0) {
				m2::current_log_level = m2::LogLevel::Warn;
			} else if (strcmp(argv[i] + loglevelStrlen, "error") == 0) {
				m2::current_log_level = m2::LogLevel::Error;
			} else if (strcmp(argv[i] + loglevelStrlen, "fatal") == 0) {
				m2::current_log_level = m2::LogLevel::Fatal;
			} else {
				LOG_WARN("Invalid log level");
			}
			LOG_INFO("Current log level", m2::current_log_level);
		} else {
			LOG_WARN("Invalid command line argument");
		}
	}

	ThreadPool tpool;

	// Global initialization
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
		LOG_FATAL("SDL error", SDL_GetError());
		return -1;
	}
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		LOG_FATAL("SDL error", IMG_GetError());
		return -1;
	}
	if (TTF_Init() != 0) {
		LOG_FATAL("SDL error", TTF_GetError());
		return -1;
	}

	Game::create_instance();

	if (m2::ui::execute_blocking(&m2g::ui::entry) == m2::ui::Action::QUIT) {
		return 0;
	}

	auto pause_ticks = sdl::get_ticks();
	float time_since_last_phy = 0.0f;
	unsigned prev_phy_ticks = 0, prev_gfx_ticks = 0;
	unsigned prev_phy_step_count = UINT_MAX;

	unsigned frame_count_time = 0;
	unsigned phy_step_count = 0;
	unsigned gfx_draw_count = 0;
	auto frame_start_ticks = pause_ticks;
	while (true) {
		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		if (prev_phy_step_count) {
			// Clear the events only if the physics step has executed
			// Otherwise some keys/buttons may not have been handled
			GAME.events.clear();
		}
		if (GAME.events.gather()) {
			// Handle quit event
			if (GAME.events.pop_quit()) {
				break;
			}
			// Handle window resize event
			auto window_resize = GAME.events.pop_window_resize();
			if (window_resize) {
				GAME.update_window_dims(window_resize->x, window_resize->y);
				if (GAME.leftHudUIState) {
					GAME.leftHudUIState->update_positions(GAME.leftHudRect);
				}
                if (GAME.rightHudUIState) {
	                GAME.rightHudUIState->update_positions(GAME.rightHudRect);
				}
			}
            // Handle key events
            if (GAME.events.pop_key_press(Key::MENU)) {
				auto pause_start = sdl::get_ticks();
                if (m2::ui::execute_blocking(&m2g::ui::pause) == m2::ui::Action::QUIT) {
                    return 0;
                }
				pause_ticks += sdl::get_ticks() - pause_start;
            }
            if (GAME.events.pop_key_press(Key::CONSOLE)) {
				auto pause_start = sdl::get_ticks();
                if (m2::ui::execute_blocking(&m2::ui::console_ui) == m2::ui::Action::QUIT) {
                    return 0;
                }
				pause_ticks += sdl::get_ticks() - pause_start;
            }
            // Handle HUD events (mouse and key)
			IF(GAME.leftHudUIState)->handle_events(GAME.events);
			IF(GAME.rightHudUIState)->handle_events(GAME.events);
		}
		GAME.update_mouse_position();
		//////////////////////// END OF EVENT HANDLING /////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// PHYSICS ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		for (prev_phy_step_count = 0; prev_phy_step_count < 4; prev_phy_step_count++) {
			auto ticks_since_prev_phy = sdl::get_ticks(prev_phy_ticks, pause_ticks, 1) - prev_phy_ticks;
			prev_phy_ticks += ticks_since_prev_phy;
			time_since_last_phy += (float)ticks_since_prev_phy / 1000.0f;
			if (time_since_last_phy <= GAME.phy_period) {
				break;
			}

			// Advance time by GAME.phy_period
			GAME.deltaTime_s = GAME.phy_period;
			/////////////////////////////// PRE-PHY ////////////////////////////////
			for (auto physique_it: GAME.physics) {
				IF(physique_it.first->pre_step)(*physique_it.first);
			}
			GAME.execute_deferred_actions();
			////////////////////////////// CHARACTER ///////////////////////////////
			for (auto character_it: GAME.characters) {
				auto &chr = get_character_base(*character_it.first);
				chr.automatic_update();
			}
			for (auto character_it: GAME.characters) {
				auto &chr = get_character_base(*character_it.first);
				IF(chr.update)(chr);
			}
			GAME.execute_deferred_actions();
			/////////////////////////////// PHYSICS ////////////////////////////////
			if (GAME.world) {
				GAME.world->Step(GAME.phy_period, GAME.velocityIterations, GAME.positionIterations);
				// Update positions
				for (auto physique_it: GAME.physics) {
					auto &phy = *physique_it.first;
					if (phy.body) {
						auto &object = phy.parent();
						auto old_pos = object.position;
						object.position = m2::Vec2f{phy.body->GetPosition()};
						if (old_pos != object.position) {
							GAME.draw_list.queue_update(phy.object_id, object.position);
						}
					}
				}
			}
			GAME.draw_list.update();
			/////////////////////////////// POST-PHY ///////////////////////////////
			for (auto physique_it: GAME.physics) {
				IF(physique_it.first->post_step)(*physique_it.first);
			}
			GAME.execute_deferred_actions();
			++phy_step_count;
			//////////////////////////// END OF PHYSICS ////////////////////////////
			////////////////////////////////////////////////////////////////////////

			time_since_last_phy -= GAME.phy_period;
		}
		if (prev_phy_step_count == 4) {
			time_since_last_phy = 0.0f;
		}

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// PRE-GFX ///////////////////////////////
		GAME.deltaTicks_ms = sdl::get_ticks(prev_gfx_ticks, pause_ticks, 1) - prev_gfx_ticks;
		GAME.deltaTime_s = (float)GAME.deltaTicks_ms / 1000.0f;
		prev_gfx_ticks += GAME.deltaTicks_ms;
		for (auto graphic_if : GAME.graphics) {
			IF(graphic_if.first->pre_draw)(*graphic_if.first);
		}
		////////////////////////////////// HUD /////////////////////////////////
		IF(GAME.leftHudUIState)->update_contents();
		IF(GAME.rightHudUIState)->update_contents();
		///////////////////////////////// CLEAR ////////////////////////////////
		SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 0, 0, 255);
		SDL_RenderClear(GAME.sdlRenderer);
		//////////////////////////////// TERRAIN ///////////////////////////////
        for (auto graphic_it : GAME.terrainGraphics) {
			IF(graphic_it.first->on_draw)(*graphic_it.first);
        }
		//////////////////////////////// OBJECTS ///////////////////////////////
		for (const auto& gfx_id : GAME.draw_list) {
			auto& gfx = GAME.graphics[gfx_id];
			IF(gfx.on_draw)(gfx);
		}
		//////////////////////////////// LIGHTS ////////////////////////////////
        for (auto light_it : GAME.lights) {
			IF(light_it.first->on_draw)(*light_it.first);
        }
		//////////////////////////////// EFFECTS ///////////////////////////////
		for (auto gfx_it : GAME.terrainGraphics) {
			IF(gfx_it.first->on_effect)(*gfx_it.first);
		}
		for (auto gfx_it : GAME.graphics) {
			IF(gfx_it.first->on_effect)(*gfx_it.first);
		}
#ifdef DEBUG
		// Draw debug shapes
		for (auto physique_it : GAME.physics) {
			physique_it.first->draw_debug_shapes();
		}
#endif
		////////////////////////////////// HUD /////////////////////////////////
		IF(GAME.leftHudUIState)->draw();
		IF(GAME.rightHudUIState)->draw();
		/////////////////////////////// ENVELOPER //////////////////////////////
		SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 0, 0, 255);
		SDL_RenderFillRect(GAME.sdlRenderer, &GAME.topEnvelopeRect);
		SDL_RenderFillRect(GAME.sdlRenderer, &GAME.bottomEnvelopeRect);
		SDL_RenderFillRect(GAME.sdlRenderer, &GAME.leftEnvelopeRect);
		SDL_RenderFillRect(GAME.sdlRenderer, &GAME.rightEnvelopeRect);
		//////////////////////////////// PRESENT ///////////////////////////////
		SDL_RenderPresent(GAME.sdlRenderer);
		/////////////////////////////// POST-GFX ///////////////////////////////
		for (auto graphic_if : GAME.graphics) {
			IF(graphic_if.first->post_draw)(*graphic_if.first);
		}
		++gfx_draw_count;
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////

		auto frame_end_ticks = sdl::get_ticks();
		frame_count_time += frame_end_ticks - frame_start_ticks;
		frame_start_ticks = frame_end_ticks;
		if (1000 < frame_count_time) {
			frame_count_time -= 1000;
			LOGF_TRACE("PHY count %d, GFX count %d", phy_step_count, gfx_draw_count);
			phy_step_count = 0;
			gfx_draw_count = 0;
		}
	}

	Game::destroy_instance();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}
