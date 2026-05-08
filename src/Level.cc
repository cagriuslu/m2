#include <m2/Game.h>
#include <m2/Level.h>
#include <m2/Meta.h>
#include <m2/Proxy.h>
#include <m2/bulksheeteditor/Ui.h>
#include <m2/leveleditor/Ui.h>
#include <m2/game/object/Camera.h>
#include <m2/game/object/God.h>
#include <m2/game/object/Origin.h>
#include <m2/game/object/Pointer.h>
#include <m2/game/object/Tile.h>
#include <m2/protobuf/Detail.h>
#include <m2/sheeteditor/Ui.h>
#include <m2/thirdparty/physics/box2d/DebugDraw.h>
#include <m2/ui/widget/Text.h>
#include <m2/Log.h>
#include <M2.orm.h>
#include <CMakeProject.h>
#include <filesystem>
#include <utility>

using namespace m2;

Level::~Level() {
	// Custom destructor is provided because the order is important

	objects.Clear();
	groups.clear();
	for (auto& g : flatGraphics) {
		g.Clear();
	}
	uprightGraphics.Clear();
	physics.Clear();
	lights.Clear();
	soundEmitters.Clear();
	_characterStorage.ClearPools();

	if (_debugDraw) {
		delete static_cast<thirdparty::physics::box2d::DebugDraw*>(_debugDraw);
		_debugDraw = nullptr;
	}
	delete contactListener;
	contactListener = nullptr;
	delete world[I(pb::PhysicsLayer::SPACE)];
	delete world[I(pb::PhysicsLayer::AIRBORNE)];
	delete world[I(pb::PhysicsLayer::ABOVE_GROUND)];
	delete world[I(pb::PhysicsLayer::SEA_LEVEL)];
	delete world[I(pb::PhysicsLayer::UNDER_WATER)];
	delete world[I(pb::PhysicsLayer::SEABED)];
	delete world[I(pb::PhysicsLayer::BEDROCK)];
}

void_expected Level::InitSinglePlayer(
    const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name) {
	stateVariant.emplace<m2g::Proxy::LevelState>();
	return InitAnyPlayer(levelPathOrBlueprint, name, true,
		[](const std::string& name_, const pb::Level& lb) { M2G_PROXY.pre_single_player_level_init(name_, lb); },
		[](const std::string& name_, const pb::Level& lb) { M2G_PROXY.post_single_player_level_init(name_, lb); });
}
void_expected Level::InitTurnBasedMultiPlayerAsHost(
    const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name) {
	INFO_FN();
	stateVariant.emplace<m2g::Proxy::LevelState>();
	return InitAnyPlayer(levelPathOrBlueprint, name, false,
		[](const std::string&, const pb::Level&) {},
		[](const std::string& name_, const pb::Level& lb) { M2G_PROXY.postTurnBasedLevelClientInit(name_, lb); });
}
void_expected Level::InitTurnBasedMultiPlayerAsGuest(
    const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name) {
	INFO_FN();
	stateVariant.emplace<m2g::Proxy::LevelState>();
	return InitAnyPlayer(levelPathOrBlueprint, name, false,
		[](const std::string&, const pb::Level&) {},
		[](const std::string& name_, const pb::Level& lb) { M2G_PROXY.postTurnBasedLevelClientInit(name_, lb); });
}
void_expected Level::InitLockstepMultiPlayer(const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name, const m2g::pb::LockstepGameInitParams& gameInitParams) {
	INFO_FN();
	stateVariant.emplace<m2g::Proxy::LevelState>();
	_lockstepGameInitParams = gameInitParams;
	return InitAnyPlayer(levelPathOrBlueprint, name, false,
		[&gameInitParams](const std::string& name_, const pb::Level& lb) { M2G_PROXY.PreLockstepLevelInit(name_, lb, gameInitParams); },
		[&gameInitParams](const std::string& name_, const pb::Level& lb) { M2G_PROXY.PostLockstepLevelInit(name_, lb, gameInitParams); });
}
void_expected Level::InitLevelEditor(const std::filesystem::path& lb_path) {
	_lbPath = lb_path;
	stateVariant.emplace<leveleditor::State>();

	// Create message box initially disabled
	_messageBoxUiPanel.emplace(&DefaultMessageBoxBlueprint, UiPanel::RelativeToWindow{DefaultMessageBoxArea});
	_messageBoxUiPanel->enabled = false;

	if (exists(*_lbPath)) {
		auto lb = pb::json_file_to_message<pb::Level>(*_lbPath);
		m2ReflectUnexpected(lb);
		_lb.emplace(*lb);

		std::get<leveleditor::State>(stateVariant).LoadLevelBlueprint(*_lb);
	}

	// Create default objects
	playerId = obj::CreateGod();
	obj::CreateCamera();
	obj::CreateOrigin();

	// UI Hud
	_leftHudUiPanel.emplace(&leveleditor::gLeftHudBlueprint, UiPanel::RelativeToWindow::CreateAnchoredToPosition(M2_GAME.Dimensions().LeftHud()));
	_leftHudUiPanel->UpdateContents(0.0f);
	_rightHudUiPanel.emplace(&leveleditor::gRightHudBlueprint, UiPanel::RelativeToWindow::CreateAnchoredToPosition(M2_GAME.Dimensions().RightHud()));
	_rightHudUiPanel->UpdateContents(0.0f);
	_messageBoxUiPanel->UpdateContents(0.0f);

	return {};
}
void_expected Level::InitSheetEditor(const std::filesystem::path& path) {
	// Create state
	auto state = sheeteditor::State::create(path);
	m2ReflectUnexpected(state);
	stateVariant.emplace<sheeteditor::State>(std::move(*state));

	// Create message box initially disabled
	_messageBoxUiPanel.emplace(&DefaultMessageBoxBlueprint, UiPanel::RelativeToWindow{DefaultMessageBoxArea});
	_messageBoxUiPanel->enabled = false;

	// Create default objects
	playerId = obj::CreateGod();
	obj::CreateCamera();
	obj::CreateOrigin();

	// UI Hud
	_leftHudUiPanel.emplace(&sheet_editor_left_hud, UiPanel::RelativeToWindow::CreateAnchoredToPosition(M2_GAME.Dimensions().LeftHud()));
	_leftHudUiPanel->UpdateContents(0.0f);
	_rightHudUiPanel.emplace(&sheet_editor_right_hud, UiPanel::RelativeToWindow::CreateAnchoredToPosition(M2_GAME.Dimensions().RightHud()));
	_rightHudUiPanel->UpdateContents(0.0f);
	_messageBoxUiPanel->UpdateContents(0.0f);

	return {};
}
void_expected Level::InitBulkSheetEditor(const std::filesystem::path& path) {
	// Create state
	auto state = bulksheeteditor::State::Create(path);
	m2ReflectUnexpected(state);
	stateVariant.emplace<bulksheeteditor::State>(std::move(*state));

	// Create message box initially disabled
	_messageBoxUiPanel.emplace(&DefaultMessageBoxBlueprint, UiPanel::RelativeToWindow{DefaultMessageBoxArea});
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
	return std::holds_alternative<leveleditor::State>(stateVariant)
			|| std::holds_alternative<sheeteditor::State>(stateVariant)
			|| std::holds_alternative<bulksheeteditor::State>(stateVariant);
}
Stopwatch::Duration Level::GetTotalSimulatedDuration() const {
	const auto durationSinceLevelBegan = _beganAt->GetDurationSince();
	const auto unsimulatedDuration = _totalPauseDuration + (_pausedAt ? _pausedAt->GetDurationSince() : Stopwatch::Duration{});
	return durationSinceLevelBegan - unsimulatedDuration;
}
int32_t Level::CalculateLockstepGameStateHash(const int32 initialValue) const {
	// ReSharper disable once CppDFAUnreachableCode
	if constexpr (not GAME_IS_DETERMINISTIC) {
		throw M2_ERROR("Game is not deterministic");
	}
	int32_t hash = initialValue;
	for (const auto& phy : physics) {
		hash = HashI(ToRawValue(phy.position.GetX()), hash);
		hash = HashI(ToRawValue(phy.position.GetY()), hash);
		hash = HashI(ToRawValue(phy.orientation), hash);
	}
	hash = _characterStorage.HashCharacters(hash);
	return hash;
}
pb::LockstepDebugStateReport Level::CalculateLockstepDebugStateReport(const int32 initialValue) const {
	if constexpr (not GAME_IS_DETERMINISTIC) {
		throw M2_ERROR("Game is not deterministic");
	}
	pb::LockstepDebugStateReport debugStateReport;
	debugStateReport.set_game_state_hash(CalculateLockstepGameStateHash(initialValue));
	for (const auto& phy : physics) {
		auto* physique = debugStateReport.add_physique();
		physique->set_phy_id(phy.GetOwner().GetPhysiqueId());
		physique->set_owner_id(phy.GetOwnerId());
		physique->set_object_type(phy.GetOwner().GetType());
		physique->set_parent_id(phy.GetOwner().GetParentId());
		// Do not record the position and orientation information for these objects
		const auto isHumanPlayer = std::ranges::find(multiPlayerObjectIds, phy.GetOwnerId()) != multiPlayerObjectIds.end();
		const auto isCamera = phy.GetOwnerId() == cameraId;
		const auto isPointer = phy.GetOwnerId() == pointer_id;
		if (not isHumanPlayer && not isCamera && not isPointer) {
			physique->set_exact_position_x(phy.position.GetX().ToRawValue());
			physique->set_exact_position_y(phy.position.GetY().ToRawValue());
			physique->set_exact_orientation(phy.orientation.ToRawValue());
		}
	}
	_characterStorage.FillDebugStateReport(debugStateReport);
	return debugStateReport;
}
DrawLayer Level::GetDrawLayer(const GraphicId gfxId) {
	const auto shiftedGfxPoolId = ToShiftedPoolId(gfxId);

	for (int i = 0; i < FLAT_GRAPHICS_LAYER_COUNT; ++i) {
		if (flatGraphics[i].GetShiftedPoolId() == shiftedGfxPoolId) {
			return static_cast<pb::FlatGraphicsLayer>(i);
		}
	}
	if (uprightGraphics.GetShiftedPoolId() == shiftedGfxPoolId) {
		const auto& gfx = uprightGraphics[gfxId];
		for (auto i = 0zu; i < uprightDrawLists.size(); ++i) {
			if (uprightDrawLists[i].ContainsObject(gfx.GetOwnerId())) {
				return static_cast<pb::UprightGraphicsLayer>(i);
			}
		}
	}
	throw M2_ERROR("Graphic component does not belong to any of the pools");
}
std::pair<Pool<Graphic>&, DrawList*> Level::GetGraphicPoolAndDrawList(const GraphicId gfxId) {
	const auto shiftedGfxPoolId = ToShiftedPoolId(gfxId);

	for (auto& pool : flatGraphics) {
		if (pool.GetShiftedPoolId() == shiftedGfxPoolId) {
			return std::pair<Pool<Graphic>&,DrawList*>{pool, nullptr};
		}
	}
	if (uprightGraphics.GetShiftedPoolId() == shiftedGfxPoolId) {
		const auto& gfx = uprightGraphics[gfxId];
		for (auto& uprightDrawList : uprightDrawLists) {
			if (uprightDrawList.ContainsObject(gfx.GetOwnerId())) {
				return std::pair<Pool<Graphic>&,DrawList*>{uprightGraphics, &uprightDrawList};
			}
		}
	}
	throw M2_ERROR("Graphic component does not belong to any of the pools");

}
std::pair<Pool<Graphic>&, DrawList*> Level::GetGraphicPoolAndDrawList(const DrawLayer drawLayer) {
	if (std::holds_alternative<pb::FlatGraphicsLayer>(drawLayer)) {
		const auto bgLayer = std::get<pb::FlatGraphicsLayer>(drawLayer);
		return std::pair<Pool<Graphic>&,DrawList*>{flatGraphics.at(I(bgLayer)), nullptr};
	}
	const auto fgLayer = std::get<pb::UprightGraphicsLayer>(drawLayer);
	return std::pair<Pool<Graphic>&,DrawList*>{uprightGraphics, &uprightDrawLists.at(I(fgLayer))};
}
const m2g::Proxy::LevelState& Level::GetProxyLevelState() const {
	return std::get<m2g::Proxy::LevelState>(stateVariant);
}
pb::ProjectionType Level::GetProjectionType() const {
	const auto isEditor = std::holds_alternative<leveleditor::State>(stateVariant)
		|| std::holds_alternative<sheeteditor::State>(stateVariant)
		|| std::holds_alternative<bulksheeteditor::State>(stateVariant);
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
float Level::GetHorizontalFov() const { return _lb ? _lb->horizontal_fov() : M2_GAME.Dimensions().GameM().GetX(); }
VecF Level::GetWorldPositionOfPixel(const VecI& pixelPosition) const {
	const auto screenCenterToPixelPx = VecI{pixelPosition.x - M2_GAME.Dimensions().WindowDimensions().x / 2, pixelPosition.y - M2_GAME.Dimensions().WindowDimensions().y / 2};
	const auto screenCenterToPixelM = VecF{ToFloat(screenCenterToPixelPx.x) / M2_GAME.Dimensions().OutputPixelsPerMeter(), ToFloat(screenCenterToPixelPx.y) / M2_GAME.Dimensions().OutputPixelsPerMeter()};

	if (IsProjectionTypePerspective(GetProjectionType())) {
		// Mouse moves on the plane centered at the player looking towards the camera
		// Find m3::VecF of the mouse position in the world starting from the player position
		const auto sin_of_player_to_camera_angle = GetCameraOffset().z / GetCameraOffset().length();
		const auto cos_of_player_to_camera_angle = sqrtf(1.0f - sin_of_player_to_camera_angle * sin_of_player_to_camera_angle);

		const auto y_offset = ToFloat(screenCenterToPixelPx.y) / m3::Ppm() * sin_of_player_to_camera_angle;
		const auto z_offset = -(ToFloat(screenCenterToPixelPx.y) / m3::Ppm()) * cos_of_player_to_camera_angle;
		const auto x_offset = ToFloat(screenCenterToPixelPx.x) / m3::Ppm();
		const auto player_position = m3::FocusPositionM();
		const auto mouse_position_world_m = m3::VecF{player_position.x + x_offset, player_position.y + y_offset, player_position.z + z_offset};

		// Create Line from camera to mouse position
		const auto ray_to_mouse = m3::Line::from_points(m3::CameraPositionM(), mouse_position_world_m);
		// Get the xy-plane
		const auto plane = m3::Plane::xy_plane(M2G_PROXY.xy_plane_z_component);
		// Get the intersection
		if (const auto [intersection_point, forward_intersection] = plane.intersection(ray_to_mouse); forward_intersection) {
			return VecF{intersection_point.x, intersection_point.y};
		} else {
			return VecF{-intersection_point.x, -10000.0f};  // Infinity is 10KM
		}
	} else {
		const auto camera_position = objects[cameraId].GetPhysique().position;
		return screenCenterToPixelM + static_cast<VecF>(camera_position);
	}
}

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
m2g::Proxy::LevelState& Level::GetProxyLevelState() {
	return std::get<m2g::Proxy::LevelState>(stateVariant);
}

void_expected Level::EmplaceLevelSaver(std::optional<multiplayer::lockstep::LevelSaverInterface>& out, const std::string& fpath) {
	if (_beganAt) {
		return make_unexpected("Level saver should have been started before starting the simulation");
	}
	const auto selfIt = std::ranges::find(multiPlayerObjectIds, playerId);
	if (selfIt == multiPlayerObjectIds.end()) {
		return make_unexpected("Self player ID is not found in multiplayer object IDs");
	}
	// Remove old and create new
	std::filesystem::remove(fpath);
	auto result = genORM::database::open_or_create(fpath.c_str());
	m2ReflectUnexpected(result);
	// Write metadata
	const auto commitHash = std::vector<uint8_t>{GIT_SHORT_COMMIT_HASH.cbegin(), GIT_SHORT_COMMIT_HASH.cend()};
	const auto selfIndex = I(std::distance(multiPlayerObjectIds.begin(), selfIt));
	const auto levelBlueprint = _lb->SerializeAsString();
	const auto gameInitParams = _lockstepGameInitParams->SerializeAsString();
	auto metadataResult = orm::LockstepGameMetadata::create(*result, commitHash, I(multiPlayerObjectIds.size()), selfIndex,
		std::vector<uint8_t>{levelBlueprint.cbegin(), levelBlueprint.cend()},
		std::vector<uint8_t>{_name.cbegin(), _name.cend()},
		std::vector<uint8_t>{gameInitParams.cbegin(), gameInitParams.cend()},
		std::nullopt);
	m2ReflectUnexpected(metadataResult);
	// Move db to heap
	auto db = std::make_unique<genORM::database>(std::move(result.value()));
	out.emplace(std::move(db));
	return {};
}

void Level::EnableDimmingWithExceptions(std::set<ObjectId>&& exceptions) {
	LOG_DEBUG("Enabling dimming with a number of exceptions", exceptions.size());
	_dimmingExceptions = std::move(exceptions);
}

void Level::DisableDimmingWithExceptions() {
	LOG_DEBUG("Disabling dimming");
	_dimmingExceptions.reset();
}

void Level::BeginPanning() {
	_panBeginPosition = std::make_pair(M2_GAME.events.MousePosition(), M2_GAME.events.GetWorldPositionOfMouse());
}
bool Level::IsPanning() const {
	return static_cast<bool>(_panBeginPosition);
}
std::optional<std::pair<VecI,VecF>> Level::GetPanBeginPosition() const {
	return _panBeginPosition;
}
void Level::EndPanning() {
	_panBeginPosition.reset();
}

void Level::EnableHud() {
	if (not _leftHudUiPanel || not _rightHudUiPanel) {
		throw M2_ERROR("HUD not found");
	}
	LOG_DEBUG("Enabling HUD");
	_leftHudUiPanel->enabled = true;
	_rightHudUiPanel->enabled = true;
}

void Level::DisableHud() {
	if (not _leftHudUiPanel || not _rightHudUiPanel) {
		throw M2_ERROR("HUD not found");
	}
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
	_semiBlockingUiPanel.emplace(std::move(blueprint), UiPanel::RelativeToWindow{position_ratio});
}
void Level::DismissSemiBlockingUiPanel() {
	_semiBlockingUiPanel.reset();
}
void Level::DismissSemiBlockingUiPanelDeferred() {
	M2_DEFER([this] { this->DismissSemiBlockingUiPanel(); });
}

const ObjectDebugOptions* Level::GetObjectDebugOptions(const ObjectId id) const {
	if (const auto it = _objectDebugObjects.find(id); it != _objectDebugObjects.end()) {
		return &it->second;
	}
	return nullptr;
}
const ObjectDebugOptions* Level::GetObjectTypeDebugOptions(const m2g::pb::ObjectType type) const {
	if (const auto it = _objectTypeDebugObjects.find(type); it != _objectTypeDebugObjects.end()) {
		return &it->second;
	}
	return nullptr;
}
void Level::SetObjectDebugOptions(const ObjectId id, ObjectDebugOptions options) {
	_objectDebugObjects[id] = std::move(options);
}
void Level::SetObjectTypeDebugOptions(const m2g::pb::ObjectType type, ObjectDebugOptions options) {
	_objectTypeDebugObjects[type] = std::move(options);
}
void Level::ClearObjectDebugOptions(const ObjectId id) {
	_objectDebugObjects.erase(id);
}
void Level::ClearObjectTypeDebugOptions(const m2g::pb::ObjectType type) {
	_objectTypeDebugObjects.erase(type);
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

	if (const auto* blueprint = M2G_PROXY.LeftHudBlueprint()) {
		_leftHudUiPanel.emplace(blueprint, UiPanel::RelativeToWindow::CreateAnchoredToPosition(M2_GAME.Dimensions().LeftHud()));
	}
	if (const auto* blueprint = M2G_PROXY.RightHudBlueprint()) {
		_rightHudUiPanel.emplace(blueprint, UiPanel::RelativeToWindow::CreateAnchoredToPosition(M2_GAME.Dimensions().RightHud()));
	}
	if (const auto [blueprint, area] = M2G_PROXY.MessageBoxBlueprintAndArea(); blueprint) {
		_messageBoxUiPanel.emplace(blueprint, area);
		_messageBoxUiPanel->enabled = false;
	}

	if (physical_world) {
		world[I(pb::PhysicsLayer::BEDROCK)] = new b2World(static_cast<b2Vec2>(M2G_PROXY.gravity));
		world[I(pb::PhysicsLayer::SEABED)] = new b2World(static_cast<b2Vec2>(M2G_PROXY.gravity));
		world[I(pb::PhysicsLayer::UNDER_WATER)] = new b2World(static_cast<b2Vec2>(M2G_PROXY.gravity));
		world[I(pb::PhysicsLayer::SEA_LEVEL)] = new b2World(static_cast<b2Vec2>(M2G_PROXY.gravity));
		world[I(pb::PhysicsLayer::ABOVE_GROUND)] = new b2World(static_cast<b2Vec2>(M2G_PROXY.gravity));
		world[I(pb::PhysicsLayer::AIRBORNE)] = new b2World(static_cast<b2Vec2>(M2G_PROXY.gravity));
		world[I(pb::PhysicsLayer::SPACE)] = new b2World(static_cast<b2Vec2>(M2G_PROXY.gravity));
		contactListener = new box2d::ContactListener(
		    Physique::DefaultBeginContactCallback, Physique::DefaultEndContactCallback);
		world[I(pb::PhysicsLayer::BEDROCK)]->SetContactListener(contactListener);
		world[I(pb::PhysicsLayer::SEABED)]->SetContactListener(contactListener);
		world[I(pb::PhysicsLayer::UNDER_WATER)]->SetContactListener(contactListener);
		world[I(pb::PhysicsLayer::SEA_LEVEL)]->SetContactListener(contactListener);
		world[I(pb::PhysicsLayer::ABOVE_GROUND)]->SetContactListener(contactListener);
		world[I(pb::PhysicsLayer::AIRBORNE)]->SetContactListener(contactListener);
		world[I(pb::PhysicsLayer::SPACE)]->SetContactListener(contactListener);
#ifdef DEBUG
		auto* debugDraw = new thirdparty::physics::box2d::DebugDraw{};
		_debugDraw = debugDraw;
		world[I(pb::PhysicsLayer::BEDROCK)]->SetDebugDraw(debugDraw);
		world[I(pb::PhysicsLayer::SEABED)]->SetDebugDraw(debugDraw);
		world[I(pb::PhysicsLayer::UNDER_WATER)]->SetDebugDraw(debugDraw);
		world[I(pb::PhysicsLayer::SEA_LEVEL)]->SetDebugDraw(debugDraw);
		world[I(pb::PhysicsLayer::ABOVE_GROUND)]->SetDebugDraw(debugDraw);
		world[I(pb::PhysicsLayer::AIRBORNE)]->SetDebugDraw(debugDraw);
		world[I(pb::PhysicsLayer::SPACE)]->SetDebugDraw(debugDraw);
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
					auto it = obj::CreateTile(static_cast<pb::FlatGraphicsLayer>(l), VecF{x, y}, sprite_type);
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
		auto it = CreateObject(fg_object.type());

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

		auto load_result = M2G_PROXY.LoadForegroundObjectFromLevelBlueprint(*it, objectPosition, fg_object.orientation());
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
