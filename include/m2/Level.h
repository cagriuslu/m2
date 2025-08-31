#pragma once
#include <m2g/Proxy.h>
#include <m2/Meta.h>
#include <m2/Object.h>
#include <m2/bulk_sheet_editor/State.h>
#include <m2/containers/DrawList.h>
#include <m2/game/Pathfinder.h>
#include <m2/game/Selection.h>
#include <m2/level_editor/State.h>
#include <m2/m3/VecF.h>
#include <m2/physics/World.h>
#include <m2/sdl/Detail.h>
#include <m2/sheet_editor/State.h>
#include <m2/single_player/State.h>
#include <m2/multi_player/lockstep/State.h>
#include <m2/multi_player/turn_based/State.h>
#include <m2/ui/UiPanel.h>
#include <Level.pb.h>
#include <box2d/b2_world.h>
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
		std::optional<Stopwatch> _beganAt; /// A stopwatch that began when the level started
		Stopwatch::Duration _totalPauseDuration; /// Total duration spent while the level was paused
		std::optional<Stopwatch> _pausedAt; //// A stopwatch that is initialized only when the level is first paused

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
		std::map<GroupIdentifier, std::unique_ptr<Group>, GroupIdentifier::Less> groups;
		std::array<Pool<Graphic>, gBackgroundDrawLayerCount> bgGraphics;
		Pool<Graphic> fgGraphics;
		std::array<DrawList, gForegroundDrawLayerCount> fgDrawLists;
		Pool<Physique> physics;
		Pool<Light> lights;
		Pool<SoundEmitter> soundEmitters;
		Pool<CharacterVariant> characters;
		std::array<b2World*, gPhysicsLayerCount> world{};
		World world2;
		box2d::ContactListener* contactListener{};
		Id cameraId{}, playerId{}, pointer_id{};
		std::optional<SoundListener> leftListener, rightListener;
		std::optional<Pathfinder> pathfinder; // Pathfinder works only on ForegroundLayer::F0 and background layers.

		std::queue<std::function<void()>> deferredActions;
		std::variant<
			std::monostate,
			splayer::State,
			multiplayer::lockstep::State,
			multiplayer::turnbased::State,
			level_editor::State,
			sheet_editor::State,
			bulk_sheet_editor::State> stateVariant;

		/// In panning mode, mouse states are not cleared by UI elements so that panning the map is possible even
		/// thought the mouse spills into UI elements.
		bool isPanning{};

		// Accessors

		[[nodiscard]] bool IsEditor() const;
		[[nodiscard]] bool IsMarkedForDeletion() const { return _markedForDeletion; }
		std::optional<std::filesystem::path> GetLevelFilePath() const { return _lbPath; }
		std::optional<pb::Level> GetLevelBlueprint() const { return _lb; }
		const std::string& GetName() const { return _name; }
		bool IsPaused() const { return static_cast<bool>(_pausedAt); }
		// TODO remove
		int GetLevelDurationMsTMP() const { return std::chrono::duration_cast<std::chrono::milliseconds>(_beganAt->GetDurationSince() - _totalPauseDuration).count(); }
		// TODO remove
		int GetTotalPauseDurationMsTMP() const { return std::chrono::duration_cast<std::chrono::milliseconds>(_totalPauseDuration).count(); }
		/// Returns the drawable layer that a graphics component belongs to.
		DrawLayer GetDrawLayer(GraphicId);
		/// Returns the pool (and optionally, the draw list) that a graphics component belongs to.
		std::pair<Pool<Graphic>&, DrawList*> GetGraphicPoolAndDrawList(GraphicId);
		/// Returns the pool (and optionally, the draw list) corresponding to a drawable layer.
		std::pair<Pool<Graphic>&, DrawList*> GetGraphicPoolAndDrawList(DrawLayer);
		World& World2() { return world2; }
		/// Inclusive rectangle that contains all terrain graphics inside. The unit is meters.
		[[nodiscard]] const RectI& GetBackgroundBoundary() const { return _backgroundBoundary; }
		const std::string& GetLevelIdentifier() const { return _lb ? _lb->identifier() : EMPTY_STRING; }
		pb::ProjectionType GetProjectionType() const;
		m3::VecF GetCameraOffset() const;
		float GetHorizontalFov() const;
		Object* GetPlayer() { return objects.Get(playerId); }
		Object* GetCamera() { return objects.Get(cameraId); }

		// Modifiers

		void BeginGameLoop();
		void Pause();
		void Unpause();
		void MarkForDeletion() { _markedForDeletion = true; }

		// Dimming control

		/// "Dimming with exceptions" is a mod where sprite sheets are dimmed for all objects except the exceptions.
		const std::optional<std::set<ObjectId>>& GetDimmingExceptions() const { return _dimmingExceptions; }
		void EnableDimmingWithExceptions(std::set<ObjectId>&& exceptions);
		void DisableDimmingWithExceptions();

		// Selection control

		void EnablePrimarySelection(RectI screenBoundaryPx) { _primarySelection.emplace(screenBoundaryPx); }
		void EnableSecondarySelection(RectI screenBoundaryPx) { _secondarySelection.emplace(screenBoundaryPx); }
		Selection* GetPrimarySelection() { return _primarySelection ? &*_primarySelection : nullptr; }
		Selection* GetSecondarySelection() { return _secondarySelection ? &*_secondarySelection : nullptr; }
		void DisablePrimarySelection() { _primarySelection.reset(); }
		void DisableSecondarySelection() { _secondarySelection.reset(); }

		// UI control

		/// Show the HUD UI elements. HUD is set to be shown at start of the game.
		void EnableHud();
		/// Hides the HUD UI elements. UI elements would get disabled, thus they won't receive any events or updates.
		void DisableHud();

		UiPanel* GetLeftHud() { return _leftHudUiPanel ? &*_leftHudUiPanel : nullptr; }
		UiPanel* GetRightHud() { return _rightHudUiPanel ? &*_rightHudUiPanel : nullptr; }
		template <typename... Args> void ReplaceLeftHud(Args&&... args) { _leftHudUiPanel.emplace(std::forward<Args>(args)...); }
		template <typename... Args> void ReplaceRightHud(Args&&... args) { _rightHudUiPanel.emplace(std::forward<Args>(args)...); }

		void ShowMessage(const std::string& msg, float timeoutS = 0.0f);
		void HideMessage();

		/// Adds a UI element that is drawn above the HUD. The UI doesn't block the game loop and consumes only the
		/// events meant for itself.
		template <typename... Args> std::list<UiPanel>::iterator AddCustomNonblockingUiPanel(Args&&... args) {
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
		UiPanel* GetSemiBlockingUiPanel() { return _semiBlockingUiPanel ? &*_semiBlockingUiPanel : nullptr; }
		void DismissSemiBlockingUiPanel(); // Should not be called from the custom UI itself
		void DismissSemiBlockingUiPanelDeferred(); // Can be called from the custom UI itself

	   private:
		void_expected InitAnyPlayer(
			const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name,
			bool physical_world, const std::function<void(const std::string&, const pb::Level&)>& preLevelInit,
			const std::function<void(const std::string&, const pb::Level&)>& postLevelInit);
		void_expected InitSinglePlayer(const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name);
		void_expected InitTurnBasedMultiPlayerAsHost(const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name);
		void_expected InitTurnBasedMultiPlayerAsGuest(const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name);
		void_expected InitLockstepMultiPlayer(const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& name, const m2g::pb::LockstepGameInitParams&);
		void_expected InitLevelEditor(const std::filesystem::path& lb_path);
		void_expected InitSheetEditor(const std::filesystem::path& path);
		void_expected InitBulkSheetEditor(const std::filesystem::path& path);
		void_expected ResetSheetEditor();
		void_expected ResetBulkSheetEditor();

		/// Returns the position with respect to GameAndHud
		VecI CalculateMouseHoverUiPanelTopLeftPosition() const;

		friend class Game;
	};
}
