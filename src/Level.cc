#include <SDL2/SDL_image.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/Level.h>
#include <m2/Meta.h>
#include <m2/Proxy.h>
#include <m2/bulk_sheet_editor/Ui.h>
#include <m2/level_editor/Ui.h>
#include <m2/game/object/Camera.h>
#include <m2/game/object/God.h>
#include <m2/game/object/Origin.h>
#include <m2/game/object/Pointer.h>
#include <m2/game/object/Tile.h>
#include <m2/protobuf/Detail.h>
#include <m2/sheet_editor/Ui.h>
#include <filesystem>
#include <iterator>
#include <m2/ui/widget/Text.h>
#include <m2/third_party/physics/box2d/DebugDraw.h>

m2::Level::~Level() {
	// Custom destructor is provided because the order is important
	characters.Clear();
	lights.Clear();
	for (auto& terrain : terrainGraphics) {
		terrain.Clear();
	}
	graphics.Clear();
	physics.Clear();
	objects.Clear();
	groups.clear();

	if (_debugDraw) {
		delete static_cast<third_party::physics::box2d::DebugDraw*>(_debugDraw);
		_debugDraw = nullptr;
	}
	delete contactListener;
	contactListener = nullptr;
	delete world;
	world = nullptr;
}

m2::void_expected m2::Level::InitSinglePlayer(
    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name) {
	stateVariant.emplace<splayer::State>();
	return InitAnyPlayer(
	    level_path_or_blueprint, name, true, &m2g::Proxy::pre_single_player_level_init, &m2g::Proxy::post_single_player_level_init);
}

m2::void_expected m2::Level::InitMultiPlayerAsHost(
    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name) {
	INFO_FN();
	stateVariant.emplace<mplayer::State>();
	return InitAnyPlayer(
	    level_path_or_blueprint, name, false, &m2g::Proxy::pre_multi_player_level_client_init, &m2g::Proxy::post_multi_player_level_client_init);
}

m2::void_expected m2::Level::InitMultiPlayerAsGuest(
    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name) {
	DEBUG_FN();
	stateVariant.emplace<mplayer::State>();
	return InitAnyPlayer(
	    level_path_or_blueprint, name, false, &m2g::Proxy::pre_multi_player_level_client_init, &m2g::Proxy::post_multi_player_level_client_init);
}

m2::void_expected m2::Level::InitLevelEditor(const std::filesystem::path& lb_path) {
	_lbPath = lb_path;
	stateVariant.emplace<level_editor::State>();

	// Create message box initially disabled
	_messageBoxUiPanel.emplace(&DefaultMessageBoxBlueprint, DefaultMessageBoxArea);
	_messageBoxUiPanel->enabled = false;

	if (exists(*_lbPath)) {
		auto lb = pb::json_file_to_message<pb::Level>(*_lbPath);
		m2ReflectUnexpected(lb);
		_lb.emplace(*lb);

		std::get<level_editor::State>(stateVariant).LoadLevelBlueprint(*_lb);
	}

	// Create default objects
	playerId = obj::create_god();
	obj::create_camera();
	obj::create_origin();

	// UI Hud
	_leftHudUiPanel.emplace(&level_editor::gLeftHudBlueprint, M2_GAME.Dimensions().LeftHud());
	_leftHudUiPanel->UpdateContents(0.0f);
	_rightHudUiPanel.emplace(&level_editor::gRightHudBlueprint, M2_GAME.Dimensions().RightHud());
	_rightHudUiPanel->UpdateContents(0.0f);
	_messageBoxUiPanel->UpdateContents(0.0f);

	return {};
}

m2::void_expected m2::Level::InitSheetEditor(const std::filesystem::path& path) {
	// Create state
	auto state = sheet_editor::State::create(path);
	m2ReflectUnexpected(state);
	stateVariant.emplace<sheet_editor::State>(std::move(*state));

	// Create message box initially disabled
	_messageBoxUiPanel.emplace(&DefaultMessageBoxBlueprint, DefaultMessageBoxArea);
	_messageBoxUiPanel->enabled = false;

	// Create default objects
	playerId = obj::create_god();
	obj::create_camera();
	obj::create_origin();

	// UI Hud
	_leftHudUiPanel.emplace(&sheet_editor_left_hud, M2_GAME.Dimensions().LeftHud());
	_leftHudUiPanel->UpdateContents(0.0f);
	_rightHudUiPanel.emplace(&sheet_editor_right_hud, M2_GAME.Dimensions().RightHud());
	_rightHudUiPanel->UpdateContents(0.0f);
	_messageBoxUiPanel->UpdateContents(0.0f);

	return {};
}

m2::void_expected m2::Level::InitBulkSheetEditor(const std::filesystem::path& path) {
	// Create state
	auto state = bulk_sheet_editor::State::Create(path);
	m2ReflectUnexpected(state);
	stateVariant.emplace<bulk_sheet_editor::State>(std::move(*state));

	// Create message box initially disabled
	_messageBoxUiPanel.emplace(&DefaultMessageBoxBlueprint, DefaultMessageBoxArea);
	_messageBoxUiPanel->enabled = false;

	// Create default objects
	playerId = obj::create_god();
	obj::create_camera();
	obj::create_origin();

	// UI Hud
	_messageBoxUiPanel->UpdateContents(0.0f);

	return {};
}

m2::void_expected m2::Level::ResetSheetEditor() {
	objects.Clear();

	// Create default objects
	playerId = obj::create_god();
	obj::create_camera();
	obj::create_origin();

	return {};
}
m2::void_expected m2::Level::ResetBulkSheetEditor() {
	objects.Clear();

	// Create default objects
	playerId = obj::create_god();
	obj::create_camera();
	obj::create_origin();

	return {};
}

bool m2::Level::IsEditor() const {
	return std::holds_alternative<level_editor::State>(stateVariant)
			|| std::holds_alternative<sheet_editor::State>(stateVariant)
			|| std::holds_alternative<bulk_sheet_editor::State>(stateVariant);
}
float m2::Level::HorizontalFov() const { return _lb ? _lb->horizontal_fov() : M2_GAME.Dimensions().GameM().x; }

m2::sdl::ticks_t m2::Level::GetLevelDuration() const {
	return sdl::get_ticks_since(*_beginTicks, *_pauseTicks);
}

void m2::Level::BeginGameLoop() {
	if (_beginTicks) {
		throw M2_ERROR("BeginGameLoop called for a second time");
	}

	// This means this is the first time the game loop is executing
	// Initialize start_ticks counters
	_beginTicks = sdl::get_ticks();
	_pauseTicks = 0;
}

void m2::Level::EnableDimmingWithExceptions(std::set<ObjectId>&& exceptions) {
	LOG_DEBUG("Enabling dimming with a number of exceptions", exceptions.size());
	_dimmingExceptions = std::move(exceptions);
}

void m2::Level::DisableDimmingWithExceptions() {
	LOG_DEBUG("Disabling dimming");
	_dimmingExceptions.reset();
}

void m2::Level::EnableHud() {
	LOG_DEBUG("Enabling HUD");
	_leftHudUiPanel->enabled = true;
	_rightHudUiPanel->enabled = true;
}

void m2::Level::DisableHud() {
	LOG_DEBUG("Disabling HUD");
	_leftHudUiPanel->enabled = false;
	_rightHudUiPanel->enabled = false;
}

void m2::Level::ShowMessage(const std::string& msg, const float timeoutS) {
	if (_messageBoxUiPanel) {
		auto* messageTextWidget = _messageBoxUiPanel->FindWidget<widget::Text>("MessageText");
		if (not messageTextWidget) {
			throw M2_ERROR("MessageBox does not contain a widget with name MessageText");
		}
		messageTextWidget->set_text(msg);
		if (timeoutS == 0.0f) {
			_messageBoxUiPanel->ClearTimeout();
		} else {
			_messageBoxUiPanel->SetTimeout(timeoutS);
		}
		_messageBoxUiPanel->enabled = true;
	} else {
		throw M2_ERROR("Attempt to display a message but MessageBox is not defined");
	}
}
void m2::Level::HideMessage() {
	if (_messageBoxUiPanel) {
		_messageBoxUiPanel->enabled = false;
		_messageBoxUiPanel->ClearTimeout();
	} else {
		throw M2_ERROR("Attempt to discard a message but MessageBox is not defined");
	}
}

void m2::Level::RemoveCustomNonblockingUiPanel(const std::list<UiPanel>::iterator it) {
	TRACE_FN();
	_customNonblockingUiPanels.erase(it);
}
void m2::Level::RemoveCustomNonblockingUiPanelDeferred(std::list<UiPanel>::iterator it) {
	TRACE_FN();
	M2_DEFER(([this,it] { _customNonblockingUiPanels.erase(it); }));
}
void m2::Level::ShowSemiBlockingUiPanel(RectF position_ratio, std::variant<const UiPanelBlueprint*, std::unique_ptr<UiPanelBlueprint>> blueprint) {
	_semiBlockingUiPanel.emplace(std::move(blueprint), position_ratio);
}
void m2::Level::DismissSemiBlockingUiPanel() {
	_semiBlockingUiPanel.reset();
}
void m2::Level::DismissSemiBlockingUiPanelDeferred() {
	M2_DEFER([this] { this->DismissSemiBlockingUiPanel(); });
}

m2::void_expected m2::Level::InitAnyPlayer(
    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name,
    bool physical_world, void (m2g::Proxy::*pre_level_init)(const std::string&, const pb::Level&),
    void (m2g::Proxy::*post_level_init)(const std::string&, const pb::Level&)) {
	if (std::holds_alternative<std::filesystem::path>(level_path_or_blueprint)) {
		_lbPath = std::get<std::filesystem::path>(level_path_or_blueprint);
		auto lb = pb::json_file_to_message<pb::Level>(*_lbPath);
		m2ReflectUnexpected(lb);
		_lb = *lb;
	} else {
		_lbPath = {};
		_lb = std::get<pb::Level>(level_path_or_blueprint);
	}
	_name = name;

	(M2G_PROXY.*pre_level_init)(_name, *_lb);

	_leftHudUiPanel.emplace(M2G_PROXY.LeftHudBlueprint(), M2_GAME.Dimensions().LeftHud());
	_rightHudUiPanel.emplace(M2G_PROXY.RightHudBlueprint(), M2_GAME.Dimensions().RightHud());
	if (const auto [blueprint, area] = M2G_PROXY.MessageBoxBlueprintAndArea(); blueprint) {
		_messageBoxUiPanel.emplace(blueprint, area);
		_messageBoxUiPanel->enabled = false;
	}

	if (physical_world) {
		world = new b2World(M2G_PROXY.gravity ? b2Vec2{0.0f, 10.0f} : b2Vec2{0.0f, 0.0f});
		contactListener = new box2d::ContactListener(
		    Physique::DefaultBeginContactCallback, Physique::DefaultEndContactCallback);
		world->SetContactListener(contactListener);
#ifdef DEBUG
		auto* debugDraw = new third_party::physics::box2d::DebugDraw{};
		_debugDraw = debugDraw;
		world->SetDebugDraw(debugDraw);
#endif
	}

	// Create background tiles
	for (int l = 0; l < _lb->background_layers_size(); ++l) {
		const auto& layer = _lb->background_layers(l);
		for (int y = 0; y < layer.background_rows_size(); ++y) {
			for (int x = 0; x < layer.background_rows(y).items_size(); ++x) {
				if (const auto sprite_type = layer.background_rows(y).items(x); sprite_type) {
					// Adjust the background boundary
					_backgroundBoundary.x = std::min(_backgroundBoundary.x, x);
					_backgroundBoundary.y = std::min(_backgroundBoundary.y, y);
					if (_backgroundBoundary.x2() < x) {
						_backgroundBoundary.w = x - _backgroundBoundary.x;
					}
					if (_backgroundBoundary.y2() < y) {
						_backgroundBoundary.h = y - _backgroundBoundary.y;
					}

					LOGF_TRACE("Creating tile from %d sprite at (%d,%d)...", sprite_type, x, y);
					auto it = obj::create_tile(static_cast<BackgroundLayer>(l), VecF{x, y}, sprite_type);
					M2G_PROXY.post_tile_create(*it, sprite_type);
					LOG_TRACE("Created tile", it.GetId());
				}
			}
		}
	}
	LOG_DEBUG("Background boundary", _backgroundBoundary);
	// Create foreground objects
	for (const auto& fg_object : _lb->objects()) {
		const auto objectPosition = VecF{fg_object.position()};
		LOG_TRACE("Loading foreground object", fg_object.type(), objectPosition);
		auto it = CreateObject(objectPosition, fg_object.type());
		it->orientation = fg_object.orientation();

		// Assign to group
		if (fg_object.has_group() && fg_object.group().type() != m2g::pb::GroupType::NO_GROUP) {
			GroupId group_id{fg_object.group()};

			Group* group;
			if (auto group_it = groups.find(group_id); group_it != groups.end()) {
				group = group_it->second.get();
			} else {
				group = M2G_PROXY.create_group(group_id.type);
				groups[group_id] = std::unique_ptr<Group>(group);
			}
			it->SetGroup(group_id, group->AddMember(it.GetId()));
		}

		auto load_result = M2G_PROXY.LoadForegroundObjectFromLevelBlueprint(*it);
		m2ReflectUnexpected(load_result);
		LOG_TRACE("Created object", it.GetId());
	}

	if (physical_world) {
		// Init pathfinder map
		pathfinder = Pathfinder{*_lb};
	}

	// Create default objects
	obj::create_camera();
	obj::create_pointer();

	// Init HUD
	//_leftHudUiPanel->update_contents(); // Update should happen after the level is full initialized
	//_rightHudUiPanel->update_contents();
	//message_box_ui_panel->update_contents();

	(M2G_PROXY.*post_level_init)(_name, *_lb);

	return {};
}

m2::VecI m2::Level::CalculateMouseHoverUiPanelTopLeftPosition() const {
	// Check the height of the panel
	const auto panelHeight = _mouseHoverUiPanel->Rect().h;
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
	const auto panelWidth = _mouseHoverUiPanel->Rect().w;
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
