#pragma once
#include <m2g/Proxy.h>
#include <m2/Meta.h>
#include <m2/Object.h>
#include <m2/bulk_sheet_editor/State.h>
#include <m2/containers/DrawList.h>
#include <m2/game/Pathfinder.h>
#include <m2/game/Selection.h>
#include <m2/level_editor/State.h>
#include <m2/multi_player/State.h>
#include <m2/m3/VecF.h>
#include <m2/physics/World.h>
#include <m2/sdl/Detail.h>
#include <m2/sheet_editor/State.h>
#include <m2/single_player/State.h>
#include <m2/ui/UiPanel.h>
#include <Level.pb.h>
#include <string>
#include <queue>
#include <functional>
#include <optional>

namespace m2 {
	// Forward declarations
	class Game;

	class Level final {
		bool _markedForDeletion{false};
		std::optional<std::filesystem::path> _lbPath;
		std::optional<pb::Level> _lb;
		std::string _name;
		RectI _backgroundBoundary; // In meters
		std::optional<sdl::ticks_t> _beginTicks, _pauseTicks;

		// Special properties effecting the simulation

		std::optional<std::set<ObjectId>> _dimmingExceptions;
		std::optional<Selection> _primarySelection, _secondarySelection;

		// UI panels (the order is significant)

		std::optional<UiPanel> _leftHudUiPanel, _rightHudUiPanel, _messageBoxUiPanel;
		std::list<UiPanel> _customNonblockingUiPanels;
		/// If activated, the panel floats next to the cursor. This panel doesn't receive events, but is updated.
		std::optional<UiPanel> _mouseHoverUiPanel;
		/// If there's a blocking panel, the events are cleared after they are delivered to it. Other panels are still
		/// updated and the world is still simulated.
		std::optional<UiPanel> _semiBlockingUiPanel;

		/// Opaque pointer to box2d::DebugDraw
		void* _debugDraw{};

	public:
		~Level();

		// Objects are not meant to be iterated over, as it holds all types of objects. If a certain type of object
		// needs to be iterated over, create a component, attach the component to an object, put component in separate
		// pool, and iterate over that pool. Another reason to put a component inside a Pool: if the type of object that
		// is using that component is created/destroyed very rapidly.
		Pool<Object> objects;
		std::map<GroupId, std::unique_ptr<Group>, GroupId::Less> groups;
		DrawList drawList;
		Pool<Physique> physics;
		Pool<Graphic> graphics;
		std::array<Pool<Graphic>, gBackgroundLayerCount> terrainGraphics;
		Pool<Light> lights;
		Pool<SoundEmitter> soundEmitters;
		Pool<CharacterVariant> characters;
		b2World* world{};
		World world2;
		box2d::ContactListener* contactListener{};
		Id cameraId{}, playerId{}, pointer_id{};
		std::optional<SoundListener> leftListener, rightListener;
		std::optional<Pathfinder> pathfinder;

		std::optional<sdl::ticks_t> rootBlockingUiBeginTicks;  // Exists only if there is an ongoing blocking UI
		std::queue<std::function<void()>> deferredActions;
		std::variant<std::monostate, splayer::State, mplayer::State, level_editor::State, sheet_editor::State, bulk_sheet_editor::State> stateVariant;

		/// In panning mode, mouse states are not cleared by UI elements so that panning the map is possible even
		/// thought the mouse spills into UI elements.
		bool isPanning{};

		void_expected InitSinglePlayer(const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name);
		void_expected InitMultiPlayerAsHost(const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name);
		void_expected InitMultiPlayerAsGuest(const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name);
		void_expected InitLevelEditor(const std::filesystem::path& lb_path);
		void_expected InitSheetEditor(const std::filesystem::path& path);
		void_expected InitBulkSheetEditor(const std::filesystem::path& path);
		void_expected ResetSheetEditor();
		void_expected ResetBulkSheetEditor();

		// Accessors

		[[nodiscard]] bool IsEditor() const;
		[[nodiscard]] bool IsMarkedForDeletion() const { return _markedForDeletion; }
		std::optional<std::filesystem::path> Path() const { return _lbPath; }
		std::optional<pb::Level> LevelBlueprint() const { return _lb; }
		const std::string& Name() const { return _name; }
		World& World2() { return world2; }
		/// Inclusive rectangle that contains all terrain graphics inside. The unit is meters.
		[[nodiscard]] const RectI& BackgroundBoundary() const { return _backgroundBoundary; }
		const std::string& Identifier() const { return _lb ? _lb->identifier() : gEmptyString; }
		pb::ProjectionType ProjectionType() const {
			return (std::holds_alternative<splayer::State>(stateVariant) || std::holds_alternative<mplayer::State>(stateVariant)) && _lb
			    ? _lb->projection_type()
			    : pb::ProjectionType::PARALLEL;
		}
		m3::VecF CameraOffset() const {
			return _lb
			    ? m3::VecF{ProjectionType() == pb::PERSPECTIVE_XYZ ? _lb->camera_offset() : 0.0f, _lb->camera_offset(), _lb->camera_z_offset()}
			    : m3::VecF{};
		}
		float HorizontalFov() const;
		Object* Player() { return objects.Get(playerId); }
		Object* Camera() { return objects.Get(cameraId); }
		const sdl::ticks_t* PauseTicksHandle() const { return &*_pauseTicks; }
		sdl::ticks_t GetLevelDuration() const;

		// Modifiers

		void BeginGameLoop();
		void AddPauseTicks(const sdl::ticks_t ticks) { _pauseTicks = _pauseTicks ? *_pauseTicks + ticks : ticks; }
		void MarkForDeletion() { _markedForDeletion = true; }

		// Features

		/// "Dimming with exceptions" is a mod where sprite sheets are dimmed for all objects except the exceptions.
		const std::optional<std::set<ObjectId>>& DimmingExceptions() const { return _dimmingExceptions; }
		void EnableDimmingWithExceptions(std::set<ObjectId>&& exceptions);
		void DisableDimmingWithExceptions();

		void EnablePrimarySelection(RectI screenBoundaryPx) { _primarySelection.emplace(screenBoundaryPx); }
		void EnableSecondarySelection(RectI screenBoundaryPx) { _secondarySelection.emplace(screenBoundaryPx); }
		Selection* PrimarySelection() { return _primarySelection ? &*_primarySelection : nullptr; }
		Selection* SecondarySelection() { return _secondarySelection ? &*_secondarySelection : nullptr; }
		void DisablePrimarySelection() { _primarySelection.reset(); }
		void DisableSecondarySelection() { _secondarySelection.reset(); }

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
		void SetMouseHoverUiPanel(Args&&... args) {
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

	   private:
		void_expected InitAnyPlayer(
		    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name,
		    bool physical_world, void (m2g::Proxy::*pre_level_init)(const std::string&, const pb::Level&),
		    void (m2g::Proxy::*post_level_init)(const std::string&, const pb::Level&));

		/// Returns the position with respect to GameAndHud
		VecI CalculateMouseHoverUiPanelTopLeftPosition() const;

		friend class Game;
	};
}
