#include <m2/Level.h>
#include <m2/Proxy.h>
#include <m2/Value.h>
#include <m2/object/Tile.h>
#include <m2/object/God.h>
#include <m2/level_editor/Ui.h>
#include <m2/object/Origin.h>
#include <m2/object/Camera.h>
#include <m2/object/Pointer.h>
#include <m2/object/Placeholder.h>
#include <m2/pixel_editor/Ui.h>
#include <m2/protobuf/Detail.h>
#include <m2/box2d/Detail.h>
#include <SDL2/SDL_image.h>
#include <m2/Game.h>
#include <filesystem>
#include <iterator>

m2::Level::~Level() {
	// Custom destructor is provided because the order is important
	characters.clear();
	lights.clear();
	terrain_graphics.clear();
	graphics.clear();
	physics.clear();
	objects.clear();
	groups.clear();

	delete contact_listener;
	contact_listener = nullptr;
	delete world;
	world = nullptr;
}

m2::VoidValue m2::Level::init_single_player(const std::variant<std::filesystem::path,pb::Level>& level_path_or_blueprint, const std::string& name) {
	_type = Type::SINGLE_PLAYER;
	if (std::holds_alternative<std::filesystem::path>(level_path_or_blueprint)) {
		_lb_path = std::get<std::filesystem::path>(level_path_or_blueprint);
		auto lb = protobuf::json_file_to_message<pb::Level>(*_lb_path);
		m2_reflect_failure(lb);
		_lb = *lb;
	} else {
		_lb_path = {};
		_lb = std::get<pb::Level>(level_path_or_blueprint);
	}
	_name = name;

	left_hud_ui_state = m2::ui::State(m2g::ui::left_hud());
	right_hud_ui_state = m2::ui::State(m2g::ui::right_hud());
	message_box_ui_state = m2::ui::State(&ui::message_box_ui);

	world = new b2World(m2g::gravity ? b2Vec2{0.0f, 10.0f} : box2d::vec2_zero());
	contact_listener = new m2::box2d::ContactListener(m2::Physique::default_begin_contact_cb, m2::Physique::default_end_contact_cb);
	world->SetContactListener(contact_listener);

	// Create background tiles
	for (int y = 0; y < _lb->background_rows_size(); ++y) {
		for (int x = 0; x < _lb->background_rows(y).items_size(); ++x) {
			auto sprite_type = _lb->background_rows(y).items(x);
			if (sprite_type) {
                LOGF_TRACE("Creating tile from %d sprite at (%d,%d)...", sprite_type, x, y);
				auto [tile_obj, tile_id] = obj::create_tile(VecF{x, y} + VecF{0.5f, 0.5f}, GAME.get_sprite(sprite_type));
				m2g::post_tile_create(tile_obj, sprite_type);
                LOG_TRACE("Created tile", tile_id);
			}
		}
	}
	// Create foreground objects
	for (const auto& fg_object : _lb->objects()) {
        LOGF_TRACE("Creating %d type object at (%d,%d)...", fg_object.type(), fg_object.position().x(), fg_object.position().y());
		auto [obj, id] = m2::create_object(m2::VecF{fg_object.position()} + VecF{0.5f, 0.5f});

		// Assign to group
		if (fg_object.has_group() && fg_object.group().type() != m2g::pb::GroupType::NO_GROUP) {
			GroupId group_id{fg_object.group()};

			Group *group;
			auto group_it = groups.find(group_id);
			if (group_it != groups.end()) {
				group = group_it->second.get();
			} else {
				group = m2g::create_group(group_id.type);
				groups[group_id] = std::unique_ptr<Group>(group);
			}
			obj.set_group(group_id, group->add_member(id));
		}

		auto load_result = m2g::fg_object_loader(obj, fg_object.type());
		m2_reflect_failure(load_result);
        LOG_TRACE("Created object", id);
	}
	// Init pathfinder map
	pathfinder = Pathfinder{*_lb};

	// Create default objects
	m2::obj::create_camera();
	m2::obj::create_pointer();

	// Init HUD
	left_hud_ui_state->update_positions(GAME.dimensions().left_hud);
	left_hud_ui_state->update_contents();
	right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
	right_hud_ui_state->update_contents();
	message_box_ui_state->update_positions(GAME.dimensions().message_box);
	message_box_ui_state->update_contents();

	return {};
}

m2::VoidValue m2::Level::init_level_editor(const std::filesystem::path& lb_path) {
	_type = Type::LEVEL_EDITOR;
	_lb_path = lb_path;
	level_editor_state = ledit::State{};

	if (std::filesystem::exists(lb_path)) {
		auto lb = protobuf::json_file_to_message<pb::Level>(*_lb_path);
		m2_reflect_failure(lb);
		// Create background tiles
		for (int y = 0; y < lb->background_rows_size(); ++y) {
			for (int x = 0; x < lb->background_rows(y).items_size(); ++x) {
				auto sprite_type = lb->background_rows(y).items(x);
				if (sprite_type) {
					auto position = VecF{x, y};
					level_editor_state->bg_placeholders[position.iround()] = std::make_pair(obj::create_placeholder(position, GAME.get_sprite(sprite_type), false), sprite_type);
				}
			}
		}
		// Create foreground objects
		for (const auto& fg_object : lb->objects()) {
			auto position = m2::VecF{fg_object.position()};
			level_editor_state->fg_placeholders[position.iround()] = std::make_pair(obj::create_placeholder(position, GAME.get_sprite(GAME.level_editor_object_sprites[fg_object.type()]), true), fg_object);
		}
	}

	// Create default objects
	player_id = m2::obj::create_god();
	m2::obj::create_camera();
	m2::obj::create_origin();

	// UI Hud
	left_hud_ui_state = m2::ui::State(&level_editor::ui::left_hud);
	left_hud_ui_state->update_positions(GAME.dimensions().left_hud);
	left_hud_ui_state->update_contents();
	right_hud_ui_state = m2::ui::State(&level_editor::ui::right_hud);
	right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
	right_hud_ui_state->update_contents();

	return {};
}

m2::VoidValue m2::Level::init_pixel_editor(const std::filesystem::path &path, int x_offset, int y_offset) {
	_type = Type::PIXEL_EDITOR;
	_lb_path = path;
	pixel_editor_state = pedit::State{};
	pixel_editor_state->image_offset = VecI{x_offset, y_offset};

	if (std::filesystem::exists(path)) {
		// Load image
		sdl::SurfaceUniquePtr tmp_surface(IMG_Load(path.string().c_str()));
		if (not tmp_surface) {
			return failure("Unable to load image " + path.string() + ": " + IMG_GetError());
		}
		// Convert to a more conventional format
		pixel_editor_state->image_surface.reset(SDL_ConvertSurfaceFormat(tmp_surface.get(), SDL_PIXELFORMAT_BGRA32, 0));
		if (not pixel_editor_state->image_surface) {
			return failure("Unable to convert image format: " + std::string(SDL_GetError()));
		}
		// Iterate over pixels
		SDL_LockSurface(pixel_editor_state->image_surface.get());
		// Activate paint mode
		pixel_editor_state->activate_paint_mode();

		const auto& off = pixel_editor_state->image_offset;
		const auto& sur = *pixel_editor_state->image_surface;
		auto y_max = std::min(128, sur.h - off.y);
		auto x_max = std::min(128, sur.w - off.x);
		for (int y = off.y; y < y_max; ++y) {
			for (int x = off.x; x < x_max; ++x) {
				// Get pixel
				auto pixel = sdl::get_pixel(&sur, x, y);
				// Map pixel to color
				SDL_Color color;
				SDL_GetRGBA(pixel, sur.format, &color.r, &color.g, &color.b, &color.a);
				// Select color
				pixel_editor_state->selected_color = color;
				// Paint pixel
				pedit::State::PaintMode::paint_color(VecI{x, y});
			}
		}

		pixel_editor_state->deactivate_mode();
		SDL_UnlockSurface(pixel_editor_state->image_surface.get());
	}

	// Create default objects
	player_id = m2::obj::create_god();
	m2::obj::create_camera();
	m2::obj::create_origin();

	// UI Hud
	left_hud_ui_state = m2::ui::State(&ui::pixel_editor_left_hud);
	left_hud_ui_state->update_positions(GAME.dimensions().left_hud);
	left_hud_ui_state->update_contents();
	right_hud_ui_state = m2::ui::State(&ui::pixel_editor_right_hud);
	right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
	right_hud_ui_state->update_contents();

	return {};
}

m2::sdl::ticks_t m2::Level::get_level_duration() const {
	return sdl::get_ticks_since(*level_start_ticks, GAME.pause_ticks - *level_start_pause_ticks);
}

void m2::Level::begin_game_loop() {
	if (!level_start_ticks || !level_start_pause_ticks) {
		// This means this is the first time the game loop is executing
		// Initialize start_ticks counters
		level_start_ticks = sdl::get_ticks();
		level_start_pause_ticks = GAME.pause_ticks;
	}
}

void m2::Level::toggle_grid() {
	auto& camera_data = dynamic_cast<m2::obj::Camera&>(*camera()->impl);
	camera_data.draw_grid_lines = !camera_data.draw_grid_lines;
}
void m2::Level::display_message(const std::string& msg, float timeout) {
	message = msg;
	message_box_ui_state->widgets[0]->disable_after = timeout;
	message_box_ui_state->widgets[0]->enabled = true;
}
