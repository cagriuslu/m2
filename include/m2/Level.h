#pragma once
#include <Level.pb.h>
#include <box2d/b2_world.h>
#include <m2/game/DynamicGridLinesLoader.h>
#include <m2g/Proxy.h>
#include <m2/Log.h>
#include <functional>
#include <optional>
#include <string>
#include <m2/sdl/Detail.h>
#include <m2/physics/World.h>
#include <m2/containers/DrawList.h>
#include "Meta.h"
#include "Object.h"
#include "bulk_sheet_editor/State.h"
#include "level_editor/Detail.h"
#include "level_editor/State.h"
#include "m2/game/Pathfinder.h"
#include "m3/VecF.h"
#include "multi_player/State.h"
#include "multi_player/Type.h"
#include "pixel_editor/State.h"
#include "sheet_editor/DynamicImageLoader.h"
#include "sheet_editor/State.h"
#include "single_player/State.h"
#include "ui/UiPanel.h"
#include <queue>

namespace m2 {
	// Forward declarations
	class Game;

	class Level final {
		std::optional<std::filesystem::path> _lbPath;
		std::optional<pb::Level> _lb;
		std::string _name;

		RectI _backgroundBoundary; // In meters
		std::optional<std::set<ObjectId>> _dimmingExceptions;
		bool _isPanning{};

		// UI panels (the order is significant)

		std::optional<UiPanel> _leftHudUiPanel, _rightHudUiPanel, _messageBoxUiPanel;
		std::list<UiPanel> _customNonblockingUiPanels;
		/// If activated, the panel floats next to the cursor. This panel doesn't receive events, but is updated.
		std::optional<UiPanel> _mouseHoverUiPanel;
		/// If there's a blocking panel, the events are cleared after they are delivered to it. Other panels are still
		/// updated and the world is still simulated.
		std::optional<UiPanel> _semiBlockingUiPanel;


	public:
		~Level();

		// Objects are not meant to be iterated over, as it holds all types of objects. If a certain type of object
		// needs to be iterated over, create a component, attach the component to an object, put component in separate
		// pool, and iterate over that pool. Another reason to put a component inside a Pool: if the type of object that
		// is using that component is created/destroyed very rapidly.
		Pool<Object> objects;
		std::map<GroupId, std::unique_ptr<Group>, GroupId::Less> groups;
		DrawList draw_list;
		Pool<Physique> physics;
		Pool<Graphic> graphics;
		std::array<Pool<Graphic>, static_cast<int>(BackgroundLayer::n)> terrain_graphics;  // First pool is the front-most terrain
		Pool<Light> lights;
		Pool<SoundEmitter> sound_emitters;
		Pool<CharacterVariant> characters;
		b2World* world{};
		World _world2;
		box2d::ContactListener* contact_listener{};
		Id camera_id{}, player_id{}, pointer_id{};
		std::optional<SoundListener> left_listener, right_listener;
		std::optional<Pathfinder> pathfinder;

		std::optional<sdl::ticks_t> level_start_ticks;
		std::optional<sdl::ticks_t> level_start_pause_ticks;
		std::queue<std::function<void()>> deferred_actions;
		std::variant<
		    std::monostate, splayer::State, mplayer::State, ledit::State, pedit::State, sedit::State, bsedit::State>
		    type_state;
		std::optional<DynamicGridLinesLoader> dynamic_grid_lines_loader;
		std::optional<DynamicGridLinesLoader> dynamic_sheet_grid_lines_loader;

		void_expected InitSinglePlayer(
		    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name);
		void_expected InitMultiPlayerAsHost(
		    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name);
		void_expected InitMultiPlayerAsGuest(
		    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name);
		void_expected InitLevelEditor(const std::filesystem::path& lb_path);
		void_expected InitPixelEditor(const std::filesystem::path& path, int x_offset, int y_offset);
		void_expected InitSheetEditor(const std::filesystem::path& path);
		void_expected InitBulkSheetEditor(const std::filesystem::path& path);
		void_expected ResetSheetEditor();
		void_expected ResetBulkSheetEditor();

		// Accessors

		std::optional<std::filesystem::path> path() const { return _lbPath; }
		std::optional<pb::Level> level_blueprint() const { return _lb; }
		const std::string& name() const { return _name; }
		World& World2() { return _world2; }
		/// Inclusive rectangle that contains all terrain graphics inside. The unit is meters.
		[[nodiscard]] const RectI& BackgroundBoundary() const { return _backgroundBoundary; }
		const std::string& identifier() const { return _lb ? _lb->identifier() : empty_string; }
		pb::ProjectionType ProjectionType() const {
			return ((std::holds_alternative<splayer::State>(type_state) ||
			         std::holds_alternative<mplayer::State>(type_state)) &&
			        _lb)
			    ? _lb->projection_type()
			    : pb::ProjectionType::PARALLEL;
		}
		m3::VecF camera_offset() const {
			return _lb
			    ? m3::VecF{ProjectionType() == pb::PERSPECTIVE_XYZ ? _lb->camera_offset() : 0.0f, _lb->camera_offset(), _lb->camera_z_offset()}
			    : m3::VecF{};
		}
		float horizontal_fov() const;
		Object* player() { return objects.get(player_id); }
		Object* camera() { return objects.get(camera_id); }
		sdl::ticks_t get_level_duration() const;

		// Modifiers

		void BeginGameLoop();

		// Features

		/// "Dimming with exceptions" is a mod where sprite sheets are dimmed for all objects except the exceptions.
		const std::optional<std::set<ObjectId>>& DimmingExceptions() const { return _dimmingExceptions; }
		void EnableDimmingWithExceptions(std::set<ObjectId>&& exceptions);
		void DisableDimmingWithExceptions();

		/// Show the HUD UI elements. HUD is set to be shown at start of the game.
		void EnableHud();
		/// Hides the HUD UI elements. UI elements would get disabled, thus they won't receive any events or updates.
		void DisableHud();

		UiPanel* LeftHud() { return _leftHudUiPanel ? &*_leftHudUiPanel : nullptr; }
		UiPanel* RightHud() { return _rightHudUiPanel ? &*_rightHudUiPanel : nullptr; }
		template <typename... Args> void ReplaceLeftHud(Args&&... args) {
			_leftHudUiPanel.emplace(std::forward<Args>(args)...);
		}
		template <typename... Args> void ReplaceRightHud(Args&&... args) {
			_rightHudUiPanel.emplace(std::forward<Args>(args)...);
		}

		void ShowMessage(const std::string& msg, float timeoutS = 0.0f);
		void HideMessage();

		/// Adds a UI element that is drawn above the HUD. The UI doesn't block the game loop and consumes only the
		/// events meant for itself.
		template <typename... Args>
		std::list<UiPanel>::iterator AddCustomNonblockingUiPanel(Args&&... args) {
			return _customNonblockingUiPanels.emplace(_customNonblockingUiPanels.end(), std::forward<Args>(args)...);
		}
		/// Removes the custom UI immediately. Can be called from the UI itself if the UI blueprint is static
		/// (won't cause lambdas to be deallocated). Can be called from outside the UI safely.
		void RemoveCustomNonblockingUiPanel(std::list<UiPanel>::iterator it);
		/// Removes the custom UI at next step. Can be called from anywhere.
		void RemoveCustomNonblockingUiPanelDeferred(std::list<UiPanel>::iterator it);

		/// Add a UI panel that follows the location of the mouse. The given position of the UiPanel will be overridden,
		/// but the size of the panel is preserved.
		template <typename... Args>
		void AddMouseHoverUiPanel(Args&&... args) {
			_mouseHoverUiPanel.emplace(std::forward<Args>(args)...);
			_mouseHoverUiPanel->SetTopLeftPosition(CalculateMouseHoverUiPanelTopLeftPosition());
		}
		void RemoveMouseHoverUiPanel() {
			_mouseHoverUiPanel.reset();
		}

		/// Displays a semi-blocking UI panel above all other UI panels. The UI is blocking in the sense that mouse
		/// movement, button, and key presses are not delivered to other panels or game objects. The UI is semi-blocking
		/// in the sense that other panels are still updated and the game loop keeps running.
		void ShowSemiBlockingUiPanel(RectF position_ratio, std::variant<const UiPanelBlueprint*, std::unique_ptr<UiPanelBlueprint>> blueprint);
		UiPanel* SemiBlockingUiPanel() { return _semiBlockingUiPanel ? &*_semiBlockingUiPanel : nullptr; }
		void DismissSemiBlockingUiPanel(); // Should not be called from the custom UI itself
		void DismissSemiBlockingUiPanelDeferred(); // Can be called from the custom UI itself

		/// In panning mode, mouse states are not cleared by UI elements so that panning the map is possible even
		/// thought the mouse spills into UI elements.
		bool IsPanning() const {  return _isPanning; }
		void EnablePanning() { _isPanning = true; }
		void DisablePanning() { _isPanning = false; }

	   private:
		void_expected InitAnyPlayer(
		    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name,
		    bool physical_world, void (m2g::Proxy::*pre_level_init)(const std::string&, const pb::Level&),
		    void (m2g::Proxy::*post_level_init)(const std::string&, const pb::Level&));

		/// Returns the position with respect to GameAndHud
		VecI CalculateMouseHoverUiPanelTopLeftPosition() const;

		friend class Game;
	};
}  // namespace m2
