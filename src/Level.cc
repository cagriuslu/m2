#include <SDL2/SDL_image.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/Level.h>
#include <m2/Meta.h>
#include <m2/Proxy.h>
#include <m2/box2d/Detail.h>
#include <m2/bulk_sheet_editor/Ui.h>
#include <m2/level_editor/Ui.h>
#include <m2/game/object/Camera.h>
#include <m2/game/object/God.h>
#include <m2/game/object/Origin.h>
#include <m2/game/object/Placeholder.h>
#include <m2/game/object/Pointer.h>
#include <m2/game/object/Tile.h>
#include <m2/pixel_editor/Ui.h>
#include <m2/protobuf/Detail.h>
#include <m2/sheet_editor/Ui.h>

#include <filesystem>
#include <iterator>

m2::Level::~Level() {
	// Custom destructor is provided because the order is important
	characters.clear();
	lights.clear();
	for (auto& terrain : terrain_graphics) {
		terrain.clear();
	}
	graphics.clear();
	physics.clear();
	objects.clear();
	groups.clear();

	delete contact_listener;
	contact_listener = nullptr;
	delete world;
	world = nullptr;
}

m2::void_expected m2::Level::init_single_player(
    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name) {
	type_state.emplace<splayer::State>();
	return init_any_player(
	    level_path_or_blueprint, name, true, &m2g::Proxy::pre_single_player_level_init, &m2g::Proxy::post_single_player_level_init);
}

m2::void_expected m2::Level::init_multi_player_as_host(
    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name) {
	INFO_FN();
	type_state.emplace<mplayer::State>();
	return init_any_player(
	    level_path_or_blueprint, name, false, &m2g::Proxy::pre_multi_player_level_client_init, &m2g::Proxy::post_multi_player_level_client_init);
}

m2::void_expected m2::Level::init_multi_player_as_guest(
    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name) {
	DEBUG_FN();
	type_state.emplace<mplayer::State>();
	return init_any_player(
	    level_path_or_blueprint, name, false, &m2g::Proxy::pre_multi_player_level_client_init, &m2g::Proxy::post_multi_player_level_client_init);
}

m2::void_expected m2::Level::init_level_editor(const std::filesystem::path& lb_path) {
	_lb_path = lb_path;
	type_state.emplace<ledit::State>();
	auto& le_state = std::get<ledit::State>(type_state);

	message_box_ui_panel.emplace(&message_box_ui, M2_GAME.Dimensions().MessageBox());

	if (std::filesystem::exists(*_lb_path)) {
		auto lb = pb::json_file_to_message<pb::Level>(*_lb_path);
		m2_reflect_unexpected(lb);
		_lb.emplace(*lb);
		// Create background tiles
		for (int l = 0; l < lb->background_layers_size(); ++l) {
			const auto& layer = lb->background_layers(l);
			for (int y = 0; y < layer.background_rows_size(); ++y) {
				for (int x = 0; x < layer.background_rows(y).items_size(); ++x) {
					auto sprite_type = layer.background_rows(y).items(x);
					if (sprite_type) {
						auto position = VecF{x, y};
						le_state.bg_placeholders[l][position.iround()] = std::make_pair(
						    obj::create_background_placeholder(position, sprite_type, static_cast<BackgroundLayer>(l)),
									sprite_type);
					}
				}
			}
		}
		// Create foreground objects
		for (const auto& fg_object : lb->objects()) {
			auto position = m2::VecF{fg_object.position()};
			le_state.fg_placeholders[position.iround()] = std::make_pair(
			    obj::create_foreground_placeholder(position, M2_GAME.object_main_sprites[fg_object.type()]), fg_object);
		}
	}

	// Create default objects
	player_id = m2::obj::create_god();
	m2::obj::create_camera();
	m2::obj::create_origin();

	// UI Hud
	left_hud_ui_panel.emplace(&level_editor::left_hud, M2_GAME.Dimensions().LeftHud());
	left_hud_ui_panel->update_contents(0.0f);
	right_hud_ui_panel.emplace(&level_editor::right_hud, M2_GAME.Dimensions().RightHud());
	right_hud_ui_panel->update_contents(0.0f);
	message_box_ui_panel->update_contents(0.0f);

	return {};
}

m2::void_expected m2::Level::init_pixel_editor(const std::filesystem::path& path, int x_offset, int y_offset) {
	_lb_path = path;
	type_state.emplace<pedit::State>();
	auto& pe_state = std::get<pedit::State>(type_state);

	pe_state.image_offset = VecI{x_offset, y_offset};

	if (std::filesystem::exists(path)) {
		// Load image
		sdl::SurfaceUniquePtr tmp_surface(IMG_Load(path.string().c_str()));
		if (not tmp_surface) {
			return make_unexpected("Unable to load image " + path.string() + ": " + IMG_GetError());
		}
		// Convert to a more conventional format
		pe_state.image_surface.reset(SDL_ConvertSurfaceFormat(tmp_surface.get(), SDL_PIXELFORMAT_BGRA32, 0));
		if (not pe_state.image_surface) {
			return make_unexpected("Unable to convert image format: " + std::string(SDL_GetError()));
		}
		// Iterate over pixels
		SDL_LockSurface(pe_state.image_surface.get());
		// Activate paint mode
		pe_state.activate_paint_mode();

		const auto& off = pe_state.image_offset;
		const auto& sur = *pe_state.image_surface;
		auto y_max = std::min(128, sur.h - off.y);
		auto x_max = std::min(128, sur.w - off.x);
		for (int y = off.y; y < y_max; ++y) {
			for (int x = off.x; x < x_max; ++x) {
				// Get pixel
				auto pixel = sdl::get_pixel(&sur, x, y);
				if (!pixel) {
					throw std::runtime_error("Implementation error! Pixel should have been in bounds");
				}
				// Map pixel to color
				SDL_Color color;
				SDL_GetRGBA(*pixel, sur.format, &color.r, &color.g, &color.b, &color.a);
				// Select color
				pe_state.selected_color = color;
				// Paint pixel
				pedit::State::PaintMode::paint_color(VecI{x, y});
			}
		}

		pe_state.deactivate_mode();
		SDL_UnlockSurface(pe_state.image_surface.get());
	}

	// Create default objects
	player_id = m2::obj::create_god();
	m2::obj::create_camera();
	m2::obj::create_origin();

	// UI Hud
	left_hud_ui_panel.emplace(&pixel_editor_left_hud, M2_GAME.Dimensions().LeftHud());
	left_hud_ui_panel->update_contents(0.0f);
	right_hud_ui_panel.emplace(&pixel_editor_right_hud, M2_GAME.Dimensions().RightHud());
	right_hud_ui_panel->update_contents(0.0f);

	return {};
}

m2::void_expected m2::Level::init_sheet_editor(const std::filesystem::path& path) {
	// Create state
	auto state = sedit::State::create(path);
	m2_reflect_unexpected(state);
	type_state.emplace<sedit::State>(std::move(*state));

	message_box_ui_panel.emplace(&message_box_ui, M2_GAME.Dimensions().MessageBox());

	// Create default objects
	player_id = m2::obj::create_god();
	m2::obj::create_camera();
	m2::obj::create_origin();

	// UI Hud
	left_hud_ui_panel.emplace(&sheet_editor_left_hud, M2_GAME.Dimensions().LeftHud());
	left_hud_ui_panel->update_contents(0.0f);
	right_hud_ui_panel.emplace(&sheet_editor_right_hud, M2_GAME.Dimensions().RightHud());
	right_hud_ui_panel->update_contents(0.0f);
	message_box_ui_panel->update_contents(0.0f);

	return {};
}

m2::void_expected m2::Level::init_bulk_sheet_editor(const std::filesystem::path& path) {
	// Create state
	auto state = bsedit::State::create(path);
	m2_reflect_unexpected(state);
	type_state.emplace<bsedit::State>(std::move(*state));

	message_box_ui_panel.emplace(&message_box_ui, M2_GAME.Dimensions().MessageBox());

	// Create default objects
	player_id = m2::obj::create_god();
	m2::obj::create_camera();
	m2::obj::create_origin();

	// UI Hud
	left_hud_ui_panel.emplace(&bulk_sheet_editor_left_hud, M2_GAME.Dimensions().LeftHud());
	left_hud_ui_panel->update_contents(0.0f);
	right_hud_ui_panel.emplace(&bulk_sheet_editor_right_hud, M2_GAME.Dimensions().RightHud());
	right_hud_ui_panel->update_contents(0.0f);
	message_box_ui_panel->update_contents(0.0f);

	return {};
}

m2::void_expected m2::Level::reset_sheet_editor() {
	objects.clear();

	// Create default objects
	player_id = obj::create_god();
	obj::create_camera();
	obj::create_origin();

	return {};
}
m2::void_expected m2::Level::reset_bulk_sheet_editor() {
	objects.clear();

	// Create default objects
	player_id = obj::create_god();
	obj::create_camera();
	obj::create_origin();

	return {};
}

float m2::Level::horizontal_fov() const { return _lb ? _lb->horizontal_fov() : M2_GAME.Dimensions().GameM().x; }

m2::sdl::ticks_t m2::Level::get_level_duration() const {
	return sdl::get_ticks_since(*level_start_ticks, M2_GAME.pause_ticks - *level_start_pause_ticks);
}

void m2::Level::begin_game_loop() {
	if (!level_start_ticks || !level_start_pause_ticks) {
		// This means this is the first time the game loop is executing
		// Initialize start_ticks counters
		level_start_ticks = sdl::get_ticks();
		level_start_pause_ticks = M2_GAME.pause_ticks;
	}
}

void m2::Level::enable_dimming_with_exceptions(std::set<ObjectId>&& exceptions) {
	LOG_DEBUG("Enabling dimming with a number of exceptions", exceptions.size());
	_dimming_exceptions = std::move(exceptions);
}

void m2::Level::disable_dimming_with_exceptions() {
	LOG_DEBUG("Disabling dimming");
	_dimming_exceptions.reset();
}

void m2::Level::enable_hud() {
	LOG_DEBUG("Enabling HUD");
	left_hud_ui_panel->enabled = true;
	right_hud_ui_panel->enabled = true;
}

void m2::Level::disable_hud() {
	LOG_DEBUG("Disabling HUD");
	left_hud_ui_panel->enabled = false;
	right_hud_ui_panel->enabled = false;
}

void m2::Level::remove_custom_nonblocking_ui_panel(std::list<UiPanel>::iterator it) {
	TRACE_FN();
	_customNonblockingUiPanels.erase(it);
}
void m2::Level::remove_custom_nonblocking_ui_panel_deferred(std::list<UiPanel>::iterator it) {
	TRACE_FN();
	M2_DEFER(([this,it]() { _customNonblockingUiPanels.erase(it); }));
}
void m2::Level::ShowSemiBlockingUiPanel(RectF position_ratio, std::variant<const UiPanelBlueprint*, std::unique_ptr<UiPanelBlueprint>> blueprint) {
	_semiBlockingUiPanel.emplace(std::move(blueprint), position_ratio);
}
void m2::Level::DismissSemiBlockingUiPanel() {
	_semiBlockingUiPanel.reset();
}
void m2::Level::DismissSemiBlockingUiPanelDeferred() {
	M2_DEFER([this]() { this->DismissSemiBlockingUiPanel(); });
}
void m2::Level::display_message(const std::string& msg) {
	message = msg;
	message_box_ui_panel->widgets[0]->enabled = true;
}
void m2::Level::remove_message() {
	message.reset();
	message_box_ui_panel->widgets[0]->enabled = false;
}

m2::void_expected m2::Level::init_any_player(
    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name,
    bool physical_world, void (m2g::Proxy::*pre_level_init)(const std::string&, const pb::Level&),
    void (m2g::Proxy::*post_level_init)(const std::string&, const pb::Level&)) {
	if (std::holds_alternative<std::filesystem::path>(level_path_or_blueprint)) {
		_lb_path = std::get<std::filesystem::path>(level_path_or_blueprint);
		auto lb = pb::json_file_to_message<pb::Level>(*_lb_path);
		m2_reflect_unexpected(lb);
		_lb = *lb;
	} else {
		_lb_path = {};
		_lb = std::get<pb::Level>(level_path_or_blueprint);
	}
	_name = name;

	(M2G_PROXY.*pre_level_init)(_name, *_lb);

	left_hud_ui_panel.emplace(M2G_PROXY.left_hud(), M2_GAME.Dimensions().LeftHud());
	right_hud_ui_panel.emplace(M2G_PROXY.right_hud(), M2_GAME.Dimensions().RightHud());
	message_box_ui_panel.emplace(&message_box_ui, M2_GAME.Dimensions().MessageBox());

	if (physical_world) {
		world = new b2World(M2G_PROXY.gravity ? b2Vec2{0.0f, 10.0f} : box2d::vec2_zero());
		contact_listener = new m2::box2d::ContactListener(
		    m2::Physique::default_begin_contact_cb, m2::Physique::default_end_contact_cb);
		world->SetContactListener(contact_listener);
	}

	// Create background tiles
	for (int l = 0; l < _lb->background_layers_size(); ++l) {
		const auto& layer = _lb->background_layers(l);
		for (int y = 0; y < layer.background_rows_size(); ++y) {
			for (int x = 0; x < layer.background_rows(y).items_size(); ++x) {
				if (const auto sprite_type = layer.background_rows(y).items(x); sprite_type) {
					// Adjust the background boundary
					_background_boundary.x = std::min(_background_boundary.x, x);
					_background_boundary.y = std::min(_background_boundary.y, y);
					if (_background_boundary.x2() < x) {
						_background_boundary.w = x - _background_boundary.x;
					}
					if (_background_boundary.y2() < y) {
						_background_boundary.h = y - _background_boundary.y;
					}

					LOGF_TRACE("Creating tile from %d sprite at (%d,%d)...", sprite_type, x, y);
					auto it = obj::create_tile(static_cast<BackgroundLayer>(l), VecF{x, y} + VecF{0.5f, 0.5f}, sprite_type);
					M2G_PROXY.post_tile_create(*it, sprite_type);
					LOG_TRACE("Created tile", it.id());
				}
			}
		}
	}
	LOG_DEBUG("Background boundary", _background_boundary);
	// Create foreground objects
	for (const auto& fg_object : _lb->objects()) {
		LOGF_TRACE(
		    "Creating %d type object at (%d,%d)...", fg_object.type(), fg_object.position().x(),
		    fg_object.position().y());
		auto it = m2::create_object(m2::VecF{fg_object.position()} + VecF{0.5f, 0.5f}, fg_object.type());

		// Assign to group
		if (fg_object.has_group() && fg_object.group().type() != m2g::pb::GroupType::NO_GROUP) {
			GroupId group_id{fg_object.group()};

			Group* group;
			auto group_it = groups.find(group_id);
			if (group_it != groups.end()) {
				group = group_it->second.get();
			} else {
				group = M2G_PROXY.create_group(group_id.type);
				groups[group_id] = std::unique_ptr<Group>(group);
			}
			it->set_group(group_id, group->add_member(it.id()));
		}

		auto load_result = M2G_PROXY.init_level_blueprint_fg_object(*it);
		m2_reflect_unexpected(load_result);
		LOG_TRACE("Created object", it.id());
	}

	if (physical_world) {
		// Init pathfinder map
		pathfinder = Pathfinder{*_lb};
	}

	// Create default objects
	obj::create_camera();
	obj::create_pointer();

	// Init HUD
	//left_hud_ui_panel->update_contents(); // Update should happen after the level is full initialized
	//right_hud_ui_panel->update_contents();
	//message_box_ui_panel->update_contents();

	(M2G_PROXY.*post_level_init)(_name, *_lb);

	return {};
}

m2::VecI m2::Level::CalculateMouseHoverUiPanelTopLeftPosition() const {
	// Check the height of the panel
	const auto panelHeight = _mouseHoverUiPanel->rect_px().h;
	// Check if there's enough space below the mouse
	const auto heightUnderMouse = M2_GAME.Dimensions().GameAndHud().y2() - M2_GAME.MousePositionPx().y;
	int finalY;
	if (panelHeight <= heightUnderMouse) {
		// We CAN fit the panel under the mouse
		finalY = M2_GAME.MousePositionPx().y;
	} else {
		// We CAN'T fit the panel under the mouse
		finalY = M2_GAME.MousePositionPx().y - panelHeight;
	}

	// Check the width of the panel
	const auto panelWidth = _mouseHoverUiPanel->rect_px().w;
	// Check if there's enough space to the right of the mouse
	const auto widthLeftOfTheMouse = M2_GAME.Dimensions().GameAndHud().x2() - M2_GAME.MousePositionPx().x;
	int finalX;
	if (panelWidth <= widthLeftOfTheMouse) {
		// We CAN fit the panel to the right of the mouse
		finalX = M2_GAME.MousePositionPx().x;
	} else {
		// We CAN'T fit the panel to the right of the mouse
		finalX = M2_GAME.MousePositionPx().x - panelWidth;
	}

	return {finalX - M2_GAME.Dimensions().GameAndHud().x, finalY - M2_GAME.Dimensions().GameAndHud().y};
}
