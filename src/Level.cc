#include <m2/Level.h>
#include <m2/Proxy.h>
#include <m2/Value.h>
#include <m2/object/Tile.h>
#include <m2/object/Ghost.h>
#include <m2/object/God.h>
#include <m2/level_editor/Ui.h>
#include <m2/object/Origin.h>
#include <m2/object/Camera.h>
#include <m2/object/Pointer.h>
#include <m2/object/Placeholder.h>
#include <m2/ui/PixelEditor.h>
#include <m2/object/Pixel.h>
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

	world = new b2World(m2g::gravity ? b2Vec2{0.0f, 10.0f} : box2d::vec2_zero());
	contact_listener = new m2::box2d::ContactListener(m2::Physique::default_begin_contact_cb, m2::Physique::default_end_contact_cb);
	world->SetContactListener(contact_listener);

	// Create background tiles
	for (int y = 0; y < _lb->background_rows_size(); ++y) {
		for (int x = 0; x < _lb->background_rows(y).items_size(); ++x) {
			auto sprite_type = _lb->background_rows(y).items(x);
			if (sprite_type) {
                LOGF_TRACE("Creating tile from %d sprite at (%d,%d)...", sprite_type, x, y);
				auto [tile_obj, tile_id] = obj::create_tile(Vec2f{x, y} + Vec2f{0.5f, 0.5f}, GAME.get_sprite(sprite_type));
				m2g::post_tile_create(tile_obj, sprite_type);
                LOG_TRACE("Created tile", tile_id);
			}
		}
	}
	// Create foreground objects
	for (const auto& fg_object : _lb->objects()) {
        LOGF_TRACE("Creating %d type object at (%d,%d)...", fg_object.type(), fg_object.position().x(), fg_object.position().y());
		auto [obj, id] = m2::create_object(m2::Vec2f{fg_object.position()} + Vec2f{0.5f, 0.5f});

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
	left_hud_ui_state = m2::ui::State(m2g::ui::left_hud());
	left_hud_ui_state->update_positions(GAME.left_hud_rect);
	left_hud_ui_state->update_contents();
	right_hud_ui_state = m2::ui::State(m2g::ui::right_hud());
	right_hud_ui_state->update_positions(GAME.right_hud_rect);
	right_hud_ui_state->update_contents();

	return {};
}

void m2::Level::LevelEditorState::PaintMode::select_sprite_type(m2g::pb::SpriteType sprite_type) {
	if (selected_sprite_ghost_id) {
		GAME.add_deferred_action(m2::create_object_deleter(selected_sprite_ghost_id));
	}
	if (sprite_type) {
		selected_sprite_type = sprite_type;
		selected_sprite_ghost_id = obj::create_ghost(GAME.get_sprite(sprite_type));
	}
}
void m2::Level::LevelEditorState::PaintMode::paint_sprite(const Vec2i& position) {
	if (!position.is_negative()) {
		// Delete previous placeholder
		EraseMode::erase_position(position);
		// Create/Replace placeholder
		LEVEL.level_editor_state->bg_placeholders[position] = std::make_pair(obj::create_placeholder(Vec2f{position}, GAME.get_sprite(selected_sprite_type), false), selected_sprite_type);
	}
}
m2::Level::LevelEditorState::PaintMode::~PaintMode() {
	if (selected_sprite_ghost_id) {
		LEVEL.deferred_actions.push_back(create_object_deleter(selected_sprite_ghost_id));
	}
}
void m2::Level::LevelEditorState::EraseMode::erase_position(const Vec2i &position) {
	auto placeholders_it = LEVEL.level_editor_state->bg_placeholders.find(position);
	if (placeholders_it != LEVEL.level_editor_state->bg_placeholders.end()) {
		LEVEL.deferred_actions.push_back(create_object_deleter(placeholders_it->second.first));
		LEVEL.level_editor_state->bg_placeholders.erase(placeholders_it);
	}
}
void m2::Level::LevelEditorState::PlaceMode::select_object_type(m2g::pb::ObjectType object_type) {
	if (selected_sprite_ghost_id) {
		GAME.add_deferred_action(create_object_deleter(selected_sprite_ghost_id));
	}
	if (object_type) {
		selected_object_type = object_type;
		selected_sprite_ghost_id = obj::create_ghost(GAME.get_sprite(GAME.level_editor_object_sprites[object_type]));
	}
}
void m2::Level::LevelEditorState::PlaceMode::select_group_type(m2g::pb::GroupType group_type) { selected_group_type = group_type; }
void m2::Level::LevelEditorState::PlaceMode::select_group_instance(unsigned group_instance) { selected_group_instance = group_instance; }
void m2::Level::LevelEditorState::PlaceMode::place_object(const Vec2i& position) const {
	if (!position.is_negative()) {
		// Delete previous placeholder
		RemoveMode::remove_object(position);
		// Create/Replace placeholder
		pb::LevelObject level_object;
		level_object.mutable_position()->set_x(position.x);
		level_object.mutable_position()->set_y(position.y);
		level_object.set_type(selected_object_type);
		level_object.mutable_group()->set_type(selected_group_type);
		level_object.mutable_group()->set_instance(selected_group_instance);
		LEVEL.level_editor_state->fg_placeholders[position] = std::make_pair(obj::create_placeholder(Vec2f{position}, GAME.get_sprite(GAME.level_editor_object_sprites[selected_object_type]), true), level_object);
	}
}
m2::Level::LevelEditorState::PlaceMode::~PlaceMode() {
	if (selected_sprite_ghost_id) {
		LEVEL.deferred_actions.push_back(create_object_deleter(selected_sprite_ghost_id));
	}
}
void m2::Level::LevelEditorState::RemoveMode::remove_object(const Vec2i &position) {
	auto placeholders_it = LEVEL.level_editor_state->fg_placeholders.find(position);
	if (placeholders_it != LEVEL.level_editor_state->fg_placeholders.end()) {
		LEVEL.deferred_actions.push_back(create_object_deleter(placeholders_it->second.first));
		LEVEL.level_editor_state->fg_placeholders.erase(placeholders_it);
	}
}
void m2::Level::LevelEditorState::SelectMode::shift_right() const {
	if (selection_position_1 && selection_position_2) {
		auto min_x = std::min(selection_position_1->x, selection_position_2->x);
		auto min_y = std::min(selection_position_1->y, selection_position_2->y);
		auto max_y = std::max(selection_position_1->y, selection_position_2->y);
		auto shift_count = abs(selection_position_1->x - selection_position_2->x) + 1;
		level_editor::shift_placeholders(LEVEL.level_editor_state->bg_placeholders, LEVEL.objects, min_x, INT32_MAX, min_y, max_y, shift_count, 0);
		level_editor::shift_placeholders(LEVEL.level_editor_state->fg_placeholders, LEVEL.objects, min_x, INT32_MAX, min_y, max_y, shift_count, 0);
	}
}
void m2::Level::LevelEditorState::SelectMode::shift_down() const {
	if (selection_position_1 && selection_position_2) {
		auto min_x = std::min(selection_position_1->x, selection_position_2->x);
		auto max_x = std::max(selection_position_1->x, selection_position_2->x);
		auto min_y = std::min(selection_position_1->y, selection_position_2->y);
		auto shift_count = abs(selection_position_1->y - selection_position_2->y) + 1;
		level_editor::shift_placeholders(LEVEL.level_editor_state->bg_placeholders, LEVEL.objects, min_x, max_x, min_y, INT32_MAX, 0, shift_count);
		level_editor::shift_placeholders(LEVEL.level_editor_state->fg_placeholders, LEVEL.objects, min_x, max_x, min_y, INT32_MAX, 0, shift_count);
	}
}
void m2::Level::LevelEditorState::SelectMode::copy() const {}
void m2::Level::LevelEditorState::SelectMode::paste_bg() const {}
void m2::Level::LevelEditorState::SelectMode::paste_fg() const {}
void m2::Level::LevelEditorState::ShiftMode::shift(const Vec2i& position) const {
	if (shift_type == ShiftType::RIGHT) {
		level_editor::shift_placeholders(LEVEL.level_editor_state->bg_placeholders, LEVEL.objects, position.x, INT32_MAX, INT32_MIN, INT32_MAX, 1, 0);
		level_editor::shift_placeholders(LEVEL.level_editor_state->fg_placeholders, LEVEL.objects, position.x, INT32_MAX, INT32_MIN, INT32_MAX, 1, 0);
	} else if (shift_type == ShiftType::DOWN) {
		level_editor::shift_placeholders(LEVEL.level_editor_state->bg_placeholders, LEVEL.objects, INT32_MIN, INT32_MAX, position.y, INT32_MAX, 0, 1);
		level_editor::shift_placeholders(LEVEL.level_editor_state->fg_placeholders, LEVEL.objects, INT32_MIN, INT32_MAX, position.y, INT32_MAX, 0, 1);
	} else if (shift_type == ShiftType::RIGHT_N_DOWN) {
		level_editor::shift_placeholders(LEVEL.level_editor_state->bg_placeholders, LEVEL.objects, position.x, INT32_MAX, position.y, INT32_MAX, 1, 1);
		level_editor::shift_placeholders(LEVEL.level_editor_state->fg_placeholders, LEVEL.objects, position.x, INT32_MAX, position.y, INT32_MAX, 1, 1);
	}
}
void m2::Level::LevelEditorState::deactivate_mode() {
	mode = std::monostate{};
}
void m2::Level::LevelEditorState::activate_paint_mode() {
	mode = PaintMode{};
	std::get<PaintMode>(mode).select_sprite_type(GAME.level_editor_background_sprites[0]);
}
void m2::Level::LevelEditorState::activate_erase_mode() {
	mode = EraseMode{};
}
void m2::Level::LevelEditorState::activate_place_mode() {
	mode = PlaceMode{};
	std::get<PlaceMode>(mode).select_object_type(GAME.level_editor_object_sprites.begin()->first);
	std::get<PlaceMode>(mode).select_group_type(m2g::pb::GroupType::NO_GROUP);
	std::get<PlaceMode>(mode).select_group_instance(0);
}
void m2::Level::LevelEditorState::activate_remove_mode() {
	mode = RemoveMode{};
}
void m2::Level::LevelEditorState::activate_select_mode() {
	mode = SelectMode{};
}
void m2::Level::LevelEditorState::activate_shift_mode() {
	mode = ShiftMode{};
}
void m2::Level::LevelEditorState::save() {
	pb::Level level;
	for (const auto& [position, pair] : LEVEL.level_editor_state->bg_placeholders) {
		auto* row = protobuf::mutable_get_or_create(level.mutable_background_rows(), position.y); // Get or create row
		*(protobuf::mutable_get_or_create(row->mutable_items(), position.x)) = pair.second; // Set sprite type
	}
	for (const auto& [position, pair] : LEVEL.level_editor_state->fg_placeholders) {
		level.add_objects()->CopyFrom(pair.second);
	}
	protobuf::message_to_json_file(level, *LEVEL._lb_path);
}
m2::VoidValue m2::Level::init_level_editor(const std::filesystem::path& lb_path) {
	_type = Type::LEVEL_EDITOR;
	_lb_path = lb_path;
	level_editor_state = LevelEditorState{};

	if (std::filesystem::exists(lb_path)) {
		auto lb = protobuf::json_file_to_message<pb::Level>(*_lb_path);
		m2_reflect_failure(lb);
		// Create background tiles
		for (int y = 0; y < lb->background_rows_size(); ++y) {
			for (int x = 0; x < lb->background_rows(y).items_size(); ++x) {
				auto sprite_type = lb->background_rows(y).items(x);
				if (sprite_type) {
					auto position = Vec2f{x, y};
					level_editor_state->bg_placeholders[position.iround()] = std::make_pair(obj::create_placeholder(position, GAME.get_sprite(sprite_type), false), sprite_type);
				}
			}
		}
		// Create foreground objects
		for (const auto& fg_object : lb->objects()) {
			auto position = m2::Vec2f{fg_object.position()};
			level_editor_state->fg_placeholders[position.iround()] = std::make_pair(obj::create_placeholder(position, GAME.get_sprite(GAME.level_editor_object_sprites[fg_object.type()]), true), fg_object);
		}
	}

	// Create default objects
	player_id = m2::obj::create_god();
	m2::obj::create_camera();
	m2::obj::create_origin();

	// UI Hud
	left_hud_ui_state = m2::ui::State(&level_editor::ui::left_hud);
	left_hud_ui_state->update_positions(GAME.left_hud_rect);
	left_hud_ui_state->update_contents();
	right_hud_ui_state = m2::ui::State(&level_editor::ui::right_hud);
	right_hud_ui_state->update_positions(GAME.right_hud_rect);
	right_hud_ui_state->update_contents();

	return {};
}

void m2::Level::PixelEditorState::PaintMode::paint_color(const Vec2i& position) {
	// Delete existing pixel, if there is one
	auto it = LEVEL.pixel_editor_state->pixels.find(position);
	if (it != LEVEL.pixel_editor_state->pixels.end()) {
		GAME.add_deferred_action(create_object_deleter(it->second.first));
		LEVEL.pixel_editor_state->pixels.erase(it);
	}
	auto pixel_id = obj::create_pixel(static_cast<Vec2f>(position), LEVEL.pixel_editor_state->selected_color);
	LEVEL.pixel_editor_state->pixels[position] = std::make_pair(pixel_id, LEVEL.pixel_editor_state->selected_color);
}
void m2::Level::PixelEditorState::EraseMode::erase_color(const Vec2i &position) {
	auto it = LEVEL.pixel_editor_state->pixels.find(position);
	if (it != LEVEL.pixel_editor_state->pixels.end()) {
		GAME.add_deferred_action(create_object_deleter(it->second.first));
		LEVEL.pixel_editor_state->pixels.erase(it);
	}
}
void m2::Level::PixelEditorState::ColorPickerMode::pick_color(const Vec2i &position) {
	auto it = LEVEL.pixel_editor_state->pixels.find(position);
	if (it != LEVEL.pixel_editor_state->pixels.end()) {
		LOG_DEBUG("Selected color");
		LEVEL.pixel_editor_state->select_color(it->second.second);
	}
}
void m2::Level::PixelEditorState::deactivate_mode() {
	mode = std::monostate{};
}
void m2::Level::PixelEditorState::activate_paint_mode() {
	mode = PaintMode{};
}
void m2::Level::PixelEditorState::activate_erase_mode() {
	mode = EraseMode{};
}
void m2::Level::PixelEditorState::activate_color_picker_mode() {
	mode = ColorPickerMode{};
}
void m2::Level::PixelEditorState::save() {
	// TODO
}
m2::VoidValue m2::Level::init_pixel_editor(const std::filesystem::path &path, int x_offset, int y_offset) {
	_type = Type::PIXEL_EDITOR;
	_lb_path = path;
	pixel_editor_state = PixelEditorState{};
	pixel_editor_state->image_offset = Vec2i{x_offset, y_offset};

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
				std::get<Level::PixelEditorState::PaintMode>(pixel_editor_state->mode).paint_color(Vec2i{x, y});
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
	left_hud_ui_state->update_positions(GAME.left_hud_rect);
	left_hud_ui_state->update_contents();
	right_hud_ui_state = m2::ui::State(&ui::pixel_editor_right_hud);
	right_hud_ui_state->update_positions(GAME.right_hud_rect);
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
