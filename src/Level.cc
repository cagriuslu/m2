#include <SDL2/SDL_image.h>
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
#include <m2/third_party/physics/box2d/DebugDraw.h>
#include <m2/ui/widget/Text.h>
#include <m2/Log.h>
#include <filesystem>

using namespace m2;

Level::~Level() {
	// Custom destructor is provided because the order is important

	objects.Clear();
	groups.clear();
	physics.Clear();
	for (auto& bg : bgGraphics) {
		bg.Clear();
	}
	fgGraphics.Clear();
	lights.Clear();
	characters.Clear();

	if (_debugDraw) {
		delete static_cast<third_party::physics::box2d::DebugDraw*>(_debugDraw);
		_debugDraw = nullptr;
	}
	delete contactListener;
	contactListener = nullptr;
	delete world[I(PhysicsLayer::P1)];
	delete world[I(PhysicsLayer::P0)];
	delete world[I(PhysicsLayer::PM1)];
}

void_expected Level::InitSinglePlayer(
    const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name) {
	stateVariant.emplace<splayer::State>();
	return InitAnyPlayer(levelPathOrBlueprint, name, true,
		[](const std::string& name_, const pb::Level& lb) { M2G_PROXY.pre_single_player_level_init(name_, lb); },
		[](const std::string& name_, const pb::Level& lb) { M2G_PROXY.post_single_player_level_init(name_, lb); });
}
void_expected Level::InitTurnBasedMultiPlayerAsHost(
    const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name) {
	INFO_FN();
	stateVariant.emplace<multiplayer::turnbased::State>();
	return InitAnyPlayer(levelPathOrBlueprint, name, false,
		[](const std::string&, const pb::Level&) {},
		[](const std::string& name_, const pb::Level& lb) { M2G_PROXY.postTurnBasedLevelClientInit(name_, lb); });
}
void_expected Level::InitTurnBasedMultiPlayerAsGuest(
    const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name) {
	INFO_FN();
	stateVariant.emplace<multiplayer::turnbased::State>();
	return InitAnyPlayer(levelPathOrBlueprint, name, false,
		[](const std::string&, const pb::Level&) {},
		[](const std::string& name_, const pb::Level& lb) { M2G_PROXY.postTurnBasedLevelClientInit(name_, lb); });
}
void_expected Level::InitLockstepMultiPlayer(const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name, const m2g::pb::LockstepGameInitParams& gameInitParams) {
	INFO_FN();
	stateVariant.emplace<multiplayer::lockstep::State>();
	return InitAnyPlayer(levelPathOrBlueprint, name, false,
		[](const std::string&, const pb::Level&) {},
		[&gameInitParams](const std::string& name_, const pb::Level& lb) { M2G_PROXY.postLockstepLevelInit(name_, lb, gameInitParams); });
}
void_expected Level::InitLevelEditor(const std::filesystem::path& lb_path) {
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
	playerId = obj::CreateGod();
	obj::CreateCamera();
	obj::CreateOrigin();

	// UI Hud
	_leftHudUiPanel.emplace(&level_editor::gLeftHudBlueprint, M2_GAME.Dimensions().LeftHud());
	_leftHudUiPanel->UpdateContents(0.0f);
	_rightHudUiPanel.emplace(&level_editor::gRightHudBlueprint, M2_GAME.Dimensions().RightHud());
	_rightHudUiPanel->UpdateContents(0.0f);
	_messageBoxUiPanel->UpdateContents(0.0f);

	return {};
}
void_expected Level::InitSheetEditor(const std::filesystem::path& path) {
	// Create state
	auto state = sheet_editor::State::create(path);
	m2ReflectUnexpected(state);
	stateVariant.emplace<sheet_editor::State>(std::move(*state));

	// Create message box initially disabled
	_messageBoxUiPanel.emplace(&DefaultMessageBoxBlueprint, DefaultMessageBoxArea);
	_messageBoxUiPanel->enabled = false;

	// Create default objects
	playerId = obj::CreateGod();
	obj::CreateCamera();
	obj::CreateOrigin();

	// UI Hud
	_leftHudUiPanel.emplace(&sheet_editor_left_hud, M2_GAME.Dimensions().LeftHud());
	_leftHudUiPanel->UpdateContents(0.0f);
	_rightHudUiPanel.emplace(&sheet_editor_right_hud, M2_GAME.Dimensions().RightHud());
	_rightHudUiPanel->UpdateContents(0.0f);
	_messageBoxUiPanel->UpdateContents(0.0f);

	return {};
}
void_expected Level::InitBulkSheetEditor(const std::filesystem::path& path) {
	// Create state
	auto state = bulk_sheet_editor::State::Create(path);
	m2ReflectUnexpected(state);
	stateVariant.emplace<bulk_sheet_editor::State>(std::move(*state));

	// Create message box initially disabled
	_messageBoxUiPanel.emplace(&DefaultMessageBoxBlueprint, DefaultMessageBoxArea);
	_messageBoxUiPanel->enabled = false;

	// Create default objects
	playerId = obj::CreateGod();
	obj::CreateCamera();
	obj::CreateOrigin();

	// UI Hud
	_messageBoxUiPanel->UpdateContents(0.0f);

	return {};
}
void_expected Level::ResetSheetEditor() {
	objects.Clear();

	// Create default objects
	playerId = obj::CreateGod();
	obj::CreateCamera();
	obj::CreateOrigin();

	return {};
}
void_expected Level::ResetBulkSheetEditor() {
	objects.Clear();

	// Create default objects
	playerId = obj::CreateGod();
	obj::CreateCamera();
	obj::CreateOrigin();

	return {};
}

bool Level::IsEditor() const {
	return std::holds_alternative<level_editor::State>(stateVariant)
			|| std::holds_alternative<sheet_editor::State>(stateVariant)
			|| std::holds_alternative<bulk_sheet_editor::State>(stateVariant);
}
Stopwatch::Duration Level::GetTotalSimulatedDuration() const {
	const auto durationSinceLevelBegan = _beganAt->GetDurationSince();
	const auto unsimulatedDuration = _totalPauseDuration + (_pausedAt ? _pausedAt->GetDurationSince() : Stopwatch::Duration{});
	return durationSinceLevelBegan - unsimulatedDuration;
}
DrawLayer Level::GetDrawLayer(const GraphicId gfxId) {
	const auto shiftedGfxPoolId = ToShiftedPoolId(gfxId);

	for (int i = 0; i < I(BackgroundDrawLayer::_n); ++i) {
		if (bgGraphics[i].GetShiftedPoolId() == shiftedGfxPoolId) {
			return static_cast<BackgroundDrawLayer>(i);
		}
	}
	if (M2_LEVEL.fgGraphics.GetShiftedPoolId() == shiftedGfxPoolId) {
		const auto& gfx = fgGraphics[gfxId];
		for (auto i = 0; i < I(ForegroundDrawLayer::_n); ++i) {
			if (M2_LEVEL.fgDrawLists[i].ContainsObject(gfx.OwnerId())) {
				return static_cast<ForegroundDrawLayer>(i);
			}
		}
	}
	throw M2_ERROR("Graphic component does not belong to any of the pools");
}
std::pair<Pool<Graphic>&, DrawList*> Level::GetGraphicPoolAndDrawList(const GraphicId gfxId) {
	const auto shiftedGfxPoolId = ToShiftedPoolId(gfxId);

	for (auto& bgPool : bgGraphics) {
		if (bgPool.GetShiftedPoolId() == shiftedGfxPoolId) {
			return std::pair<Pool<Graphic>&,DrawList*>{bgPool, nullptr};
		}
	}
	if (fgGraphics.GetShiftedPoolId() == shiftedGfxPoolId) {
		const auto& gfx = fgGraphics[gfxId];
		for (auto i = 0; i < I(ForegroundDrawLayer::_n); ++i) {
			if (fgDrawLists[i].ContainsObject(gfx.OwnerId())) {
				return std::pair<Pool<Graphic>&,DrawList*>{fgGraphics, &fgDrawLists[i]};
			}
		}
	}
	throw M2_ERROR("Graphic component does not belong to any of the pools");

}
std::pair<Pool<Graphic>&, DrawList*> Level::GetGraphicPoolAndDrawList(const DrawLayer drawLayer) {
	if (std::holds_alternative<BackgroundDrawLayer>(drawLayer)) {
		const auto bgLayer = std::get<BackgroundDrawLayer>(drawLayer);
		return std::pair<Pool<Graphic>&,DrawList*>{bgGraphics[I(bgLayer)], nullptr};
	}
	const auto fgLayer = std::get<ForegroundDrawLayer>(drawLayer);
	return std::pair<Pool<Graphic>&,DrawList*>{fgGraphics, &fgDrawLists[I(fgLayer)]};
}
pb::ProjectionType Level::GetProjectionType() const {
	const auto isEditor = std::holds_alternative<level_editor::State>(stateVariant)
		|| std::holds_alternative<sheet_editor::State>(stateVariant)
		|| std::holds_alternative<bulk_sheet_editor::State>(stateVariant);
	if (isEditor) {
		return pb::ProjectionType::PARALLEL;
	}
	return _lb ? _lb->projection_type() : pb::ProjectionType::PARALLEL;
}
m3::VecF Level::GetCameraOffset() const {
	if (not _lb) {
		return {};
	}
	return m3::VecF{
		0.0f,
		_lb->camera_offset(),
		_lb->camera_z_offset()};
}
float Level::GetHorizontalFov() const { return _lb ? _lb->horizontal_fov() : M2_GAME.Dimensions().GameM().x; }

void Level::BeginGameLoop() {
	if (_beganAt) {
		throw M2_ERROR("Level already began");
	}
	_beganAt = Stopwatch{};
	_totalPauseDuration = {};
	_pausedAt.reset();
}
void Level::Pause() {
	if (_pausedAt) {
		throw M2_ERROR("Attempt to pause an already paused level");
	}
	_pausedAt = Stopwatch{};
}
void Level::Unpause() {
	if (not _pausedAt) {
		throw M2_ERROR("Attempt to unpause an already unpaused level");
	}
	const auto pauseDuration = _pausedAt->GetDurationSince();
	_totalPauseDuration += pauseDuration;
	_pausedAt.reset();
}

void Level::EnableDimmingWithExceptions(std::set<ObjectId>&& exceptions) {
	LOG_DEBUG("Enabling dimming with a number of exceptions", exceptions.size());
	_dimmingExceptions = std::move(exceptions);
}

void Level::DisableDimmingWithExceptions() {
	LOG_DEBUG("Disabling dimming");
	_dimmingExceptions.reset();
}

void Level::EnableHud() {
	LOG_DEBUG("Enabling HUD");
	_leftHudUiPanel->enabled = true;
	_rightHudUiPanel->enabled = true;
}

void Level::DisableHud() {
	LOG_DEBUG("Disabling HUD");
	_leftHudUiPanel->enabled = false;
	_rightHudUiPanel->enabled = false;
}

void Level::ShowMessage(const std::string& msg, const float timeoutS) {
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
void Level::HideMessage() {
	if (_messageBoxUiPanel) {
		_messageBoxUiPanel->enabled = false;
		_messageBoxUiPanel->ClearTimeout();
	} else {
		throw M2_ERROR("Attempt to discard a message but MessageBox is not defined");
	}
}

void Level::RemoveCustomNonblockingUiPanel(const std::list<UiPanel>::iterator it) {
	TRACE_FN();
	_customNonblockingUiPanels.erase(it);
}
void Level::RemoveCustomNonblockingUiPanelDeferred(std::list<UiPanel>::iterator it) {
	TRACE_FN();
	M2_DEFER(([this,it] { _customNonblockingUiPanels.erase(it); }));
}
void Level::ShowSemiBlockingUiPanel(RectF position_ratio, std::variant<const UiPanelBlueprint*, std::unique_ptr<UiPanelBlueprint>> blueprint) {
	_semiBlockingUiPanel.emplace(std::move(blueprint), position_ratio);
}
void Level::DismissSemiBlockingUiPanel() {
	_semiBlockingUiPanel.reset();
}
void Level::DismissSemiBlockingUiPanelDeferred() {
	M2_DEFER([this] { this->DismissSemiBlockingUiPanel(); });
}

void_expected Level::InitAnyPlayer(
	    const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name,
	    bool physical_world, const std::function<void(const std::string&, const pb::Level&)>& preLevelInit,
		const std::function<void(const std::string&, const pb::Level&)>& postLevelInit) {
	if (std::holds_alternative<std::filesystem::path>(levelPathOrBlueprint)) {
		_lbPath = std::get<std::filesystem::path>(levelPathOrBlueprint);
		auto lb = pb::json_file_to_message<pb::Level>(*_lbPath);
		m2ReflectUnexpected(lb);
		_lb = *lb;
	} else {
		_lbPath = {};
		_lb = std::get<pb::Level>(levelPathOrBlueprint);
	}
	_name = name;

	preLevelInit(_name, *_lb);

	_leftHudUiPanel.emplace(M2G_PROXY.LeftHudBlueprint(), M2_GAME.Dimensions().LeftHud());
	_rightHudUiPanel.emplace(M2G_PROXY.RightHudBlueprint(), M2_GAME.Dimensions().RightHud());
	if (const auto [blueprint, area] = M2G_PROXY.MessageBoxBlueprintAndArea(); blueprint) {
		_messageBoxUiPanel.emplace(blueprint, area);
		_messageBoxUiPanel->enabled = false;
	}

	if (physical_world) {
		world[I(PhysicsLayer::PM1)] = new b2World(static_cast<b2Vec2>(M2G_PROXY.gravity));
		world[I(PhysicsLayer::P0)] = new b2World(static_cast<b2Vec2>(M2G_PROXY.gravity));
		world[I(PhysicsLayer::P1)] = new b2World(static_cast<b2Vec2>(M2G_PROXY.gravity));
		contactListener = new box2d::ContactListener(
		    Physique::DefaultBeginContactCallback, Physique::DefaultEndContactCallback);
		world[I(PhysicsLayer::PM1)]->SetContactListener(contactListener);
		world[I(PhysicsLayer::P0)]->SetContactListener(contactListener);
		world[I(PhysicsLayer::P1)]->SetContactListener(contactListener);
#ifdef DEBUG
		auto* debugDraw = new third_party::physics::box2d::DebugDraw{};
		_debugDraw = debugDraw;
		world[I(PhysicsLayer::PM1)]->SetDebugDraw(debugDraw);
		world[I(PhysicsLayer::P0)]->SetDebugDraw(debugDraw);
		world[I(PhysicsLayer::P1)]->SetDebugDraw(debugDraw);
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
					if (_backgroundBoundary.GetX2() < x) {
						_backgroundBoundary.w = x - _backgroundBoundary.x;
					}
					if (_backgroundBoundary.GetY2() < y) {
						_backgroundBoundary.h = y - _backgroundBoundary.y;
					}

					LOGF_TRACE("Creating tile from %d sprite at (%d,%d)...", sprite_type, x, y);
					auto it = obj::CreateTile(static_cast<BackgroundDrawLayer>(l), VecF{x, y}, sprite_type);
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
			GroupIdentifier group_id{fg_object.group()};

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
	obj::CreateCamera();
	obj::CreatePointer();

	// Init HUD
	//_leftHudUiPanel->update_contents(); // Update should happen after the level is full initialized
	//_rightHudUiPanel->update_contents();
	//message_box_ui_panel->update_contents();

	postLevelInit(_name, *_lb);

	return {};
}

VecI Level::CalculateMouseHoverUiPanelTopLeftPosition() const {
	// Check the height of the panel
	const auto panelHeight = _mouseHoverUiPanel->Rect().h;
	// Check if there's enough space below the mouse
	const auto heightUnderMouse = M2_GAME.Dimensions().GameAndHud().GetY2() - M2_GAME.MousePositionPx().y;
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
	const auto widthLeftOfTheMouse = M2_GAME.Dimensions().GameAndHud().GetX2() - M2_GAME.MousePositionPx().x;
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
