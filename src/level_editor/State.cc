#include <m2/level_editor/State.h>
#include <m2/level_editor/Ui.h>
#include <m2/Game.h>
#include <m2/game/object/Placeholder.h>
#include <m2/game/Selection.h>
#include <Level.pb.h>
#include <m2/ui/widget/CheckboxWithText.h>
#include <m2/ui/widget/IntegerInput.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextSelection.h>

namespace {
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 80};
}

m2::BackgroundLayer m2::level_editor::State::GetSelectedBackgroundLayer() const {
	return static_cast<BackgroundLayer>(
			std::get<int>(
				M2_LEVEL.LeftHud()->find_first_widget_by_name<widget::TextSelection>("BackgroundLayerSelection")->selections()[0]));
}
bool m2::level_editor::State::GetSnapToGridStatus() const {
	return M2_LEVEL.LeftHud()->find_first_widget_by_name<widget::CheckboxWithText>("SnapToGridCheckbox")->GetState();
}

void m2::level_editor::State::LoadLevelBlueprint(const pb::Level& lb) {
	// Create background tiles
	for (int l = 0; l < lb.background_layers_size(); ++l) {
		const auto& layer = lb.background_layers(l);
		for (int y = 0; y < layer.background_rows_size(); ++y) {
			for (int x = 0; x < layer.background_rows(y).items_size(); ++x) {
				if (const auto spriteType = layer.background_rows(y).items(x)) {
					PaintBackground(VecI{x, y}, spriteType);
				}
			}
		}
	}
	// Create foreground objects
	for (const auto& fgObject : lb.objects()) {
		PlaceForeground(VecF{fgObject.position()}, fgObject.orientation(), fgObject.type(), fgObject.group().type(), fgObject.group().instance());
	}
}

void m2::level_editor::State::HandleMousePrimaryButton(const VecF& position) {
	// Background operations are supported only for the positive quadrant
	if (not position.iround().is_negative()) {
		if (M2_LEVEL.RightHud()->Name() == "PaintBgRightHud") {
			if (const auto selections = M2_LEVEL.RightHud()->find_first_widget_by_name<widget::TextSelection>("SpriteTypeSelection")->selections();
					not selections.empty()) {
				const auto selectedSpriteType = static_cast<m2g::pb::SpriteType>(std::get<int>(selections[0]));
				PaintBackground(VecI{position.iround().x, position.iround().y}, selectedSpriteType);
			}
		} else if (M2_LEVEL.RightHud()->Name() == "SampleBgRightHud") {
			if (const auto it = _backgroundSpritePlaceholders[I(GetSelectedBackgroundLayer())].find(position.iround());
					it != _backgroundSpritePlaceholders[I(GetSelectedBackgroundLayer())].end()) {
				// Find and press the Paint button
				M2_LEVEL.LeftHud()->find_first_widget_by_name<widget::Text>("PaintBgButton")->trigger_action();
				// Find sprite type selection and set it
				auto* spriteTypeSelection = M2_LEVEL.RightHud()->find_first_widget_by_name<widget::TextSelection>("SpriteTypeSelection");
				// Find the index of the option that corresponds to the sprite
				const auto& options = spriteTypeSelection->GetOptions();
				for (size_t i = 0; i < options.size(); ++i) {
					if (std::get<int>(options[i].blueprint_option.return_value) == static_cast<int>(std::get<m2g::pb::SpriteType>(it->second))) {
						spriteTypeSelection->set_unique_selection(i);
						break;
					}
				}
			}
		}
	}

	// Foreground operations can be on any quadrant
	if (M2_LEVEL.RightHud()->Name() == "PlaceFgRightHud") {
		if (const auto selections = M2_LEVEL.RightHud()->find_first_widget_by_name<widget::TextSelection>("ObjectTypeSelection")->selections();
					not selections.empty()) {
			const auto selectedObjectType = static_cast<m2g::pb::ObjectType>(std::get<int>(selections[0]));
			const auto selectedGroupType = static_cast<m2g::pb::GroupType>(
					std::get<int>(
						M2_LEVEL.RightHud()->find_first_widget_by_name<widget::TextSelection>("GroupTypeSelection")->selections()[0]));
			const auto selectedGroupInstance = M2_LEVEL.RightHud()->find_first_widget_by_name<widget::IntegerInput>("GroupInstanceSelection")->value();
			const auto orientation = to_radians(M2_LEVEL.RightHud()->find_first_widget_by_name<widget::IntegerInput>("OrientationInput")->value());
			PlaceForeground(GetSnapToGridStatus() ? position.round() : position, orientation, selectedObjectType, selectedGroupType, selectedGroupInstance);
		}
	} else if (M2_LEVEL.RightHud()->Name() == "SampleFgRightHud") {
		auto foundIt = _foregroundObjectPlaceholders.end();
		float closestDistanceSquare = INFINITY;
		for (auto it = _foregroundObjectPlaceholders.begin(); it != _foregroundObjectPlaceholders.end(); ++it) {
			// Find the closest object in 1-meter radius
			if (const auto distanceSquared = position.distance_sq(it->first);
					distanceSquared <= 1.0f && distanceSquared < closestDistanceSquare) {
				foundIt = it;
				closestDistanceSquare = distanceSquared;
			}
		}
		if (foundIt != _foregroundObjectPlaceholders.end()) {
			// Find and press the Place button
			M2_LEVEL.LeftHud()->find_first_widget_by_name<widget::Text>("PlaceFgButton")->trigger_action();

			// Find object properties and set it

			const auto orientationDegreesUnbounded = iround(to_degrees(std::get<pb::LevelObject>(foundIt->second).orientation()));
			const auto orientationDegrees = ((orientationDegreesUnbounded % 360) + 360) % 360;
			auto* orientationInput = M2_LEVEL.RightHud()->find_first_widget_by_name<widget::IntegerInput>("OrientationInput");
			orientationInput->SetValue(orientationDegrees);

			const auto objectType = std::get<pb::LevelObject>(foundIt->second).type();
			auto* objectTypeSelection = M2_LEVEL.RightHud()->find_first_widget_by_name<widget::TextSelection>("ObjectTypeSelection");
			// Find the index of the option that corresponds to the object type
			const auto& objectTypeOptions = objectTypeSelection->GetOptions();
			for (size_t i = 0; i < objectTypeOptions.size(); ++i) {
				if (std::get<int>(objectTypeOptions[i].blueprint_option.return_value) == I(objectType)) {
					objectTypeSelection->set_unique_selection(i);
					break;
				}
			}

			const auto groupType = std::get<pb::LevelObject>(foundIt->second).group().type();
			auto* groupTypeSelection = M2_LEVEL.RightHud()->find_first_widget_by_name<widget::TextSelection>("GroupTypeSelection");
			// Find the index of the option that corresponds to the group type
			const auto& groupTypeOptions = groupTypeSelection->GetOptions();
			for (size_t i = 0; i < groupTypeOptions.size(); ++i) {
				if (std::get<int>(groupTypeOptions[i].blueprint_option.return_value) == I(groupType)) {
					groupTypeSelection->set_unique_selection(i);
					break;
				}
			}

			const auto groupInstance = std::get<pb::LevelObject>(foundIt->second).group().instance();
			auto* groupInstanceSelection = M2_LEVEL.RightHud()->find_first_widget_by_name<widget::IntegerInput>("GroupInstanceSelection");
			groupInstanceSelection->SetValue(groupInstance);
		}
	}
}

void m2::level_editor::State::EraseBackground(const RectI& area) {
	const auto selectedBgLayerIndex = I(GetSelectedBackgroundLayer());
	area.for_each_cell([&](const VecI& pos) {
		if (const auto it = _backgroundSpritePlaceholders[selectedBgLayerIndex].find(pos);
				it != _backgroundSpritePlaceholders[selectedBgLayerIndex].end()) {
			const auto id = std::get<Id>(it->second);
			M2_DEFER(create_object_deleter(id));
			_backgroundSpritePlaceholders[selectedBgLayerIndex].erase(it);
		}
	});
}
void m2::level_editor::State::CopyBackground(const RectI& area) {
	_backgroundSpriteClipboard.clear();

	const auto selectedBgLayerIndex = I(GetSelectedBackgroundLayer());
	area.for_each_cell([&](const VecI& pos) {
		const auto positionInClipboard = pos - area.top_left();
		if (const auto it = _backgroundSpritePlaceholders[selectedBgLayerIndex].find(pos);
				it != _backgroundSpritePlaceholders[selectedBgLayerIndex].end()) {
			const auto spriteType = std::get<m2g::pb::SpriteType>(it->second);
			_backgroundSpriteClipboard.emplace(positionInClipboard, spriteType);
		} else {
			_backgroundSpriteClipboard.emplace(positionInClipboard, m2g::pb::NO_SPRITE);
		}
	});
}
void m2::level_editor::State::PasteBackground(const VecI& position) {
	for (const auto&[clipboardPosition, spriteType] : _backgroundSpriteClipboard) {
		if (spriteType) {
			PaintBackground(position + clipboardPosition, spriteType);
		}
	}
}
void m2::level_editor::State::RandomFillBackground(const RectI& area, const std::vector<m2g::pb::SpriteType>& spriteSet) {
	area.for_each_cell([&](const VecI& pos) {
		const auto index = Random(spriteSet.size());
		PaintBackground(pos, spriteSet[index]);
	});
}

void m2::level_editor::State::RemoveForegroundObject() {
	const auto selection = ForegroundSelectionArea();
	for (auto it = _foregroundObjectPlaceholders.begin(); it != _foregroundObjectPlaceholders.end();) {
		if (selection.contains(it->first)) {
			const auto id = std::get<Id>(it->second);
			M2_DEFER(create_object_deleter(id));
			it = _foregroundObjectPlaceholders.erase(it);
		} else {
			++it;
		}
	}
}
void m2::level_editor::State::CopyForeground() {
	_foregroundObjectClipboard.clear();

	const auto selection = ForegroundSelectionArea();
	for (auto it = _foregroundObjectPlaceholders.begin(); it != _foregroundObjectPlaceholders.end(); ++it) {
		if (selection.contains(it->first)) {
			const auto positionInClipboard = it->first - selection.top_left();
			auto levelObject = std::get<pb::LevelObject>(it->second);
			levelObject.mutable_position()->set_x(positionInClipboard.x);
			levelObject.mutable_position()->set_y(positionInClipboard.y);
			_foregroundObjectClipboard.emplace(positionInClipboard, levelObject);
		}
	}
}
void m2::level_editor::State::PasteForeground() {
	const auto selection = ForegroundSelectionArea();
	for (const auto& [clipboardPosition, levelObject] : _foregroundObjectClipboard) {
		const auto position = selection.top_left() + VecF{levelObject.position().x(), levelObject.position().y()};
		PlaceForeground(position, levelObject.orientation(), levelObject.type(), levelObject.group().type(), levelObject.group().instance());
	}
}

void m2::level_editor::State::Draw() const {
	const auto drawGridSelectionIfActive = [] {
		if (const auto* selection = M2_LEVEL.PrimarySelection()) {
			if (const auto integerSelection = selection->IntegerSelectionRectM()) {
				integerSelection->for_each_cell([=](const VecI& cell) {
					Graphic::ColorCell(cell, SELECTION_COLOR);
				});
			}
		}
	};
	if (M2_LEVEL.RightHud()->Name() == "SelectBgRightHud") {
		drawGridSelectionIfActive();
	} else if (M2_LEVEL.RightHud()->Name() == "SelectFgRightHud") {
		if (GetSnapToGridStatus()) {
			drawGridSelectionIfActive();
		} else {
			if (const auto* selection = M2_LEVEL.PrimarySelection()) {
				if (const auto rawSelection = selection->SelectionRectM()) {
					Graphic::ColorRect(*rawSelection, SELECTION_COLOR);
				}
			}
		}
	}

	// Draw grid if enabled
	if (M2_LEVEL.LeftHud()->find_first_widget_by_name<widget::CheckboxWithText>("ShowGridCheckbox")->GetState()) {
		Graphic::DrawGridLines({127, 127, 255, 80});
	}
}
m2::void_expected m2::level_editor::State::Save() const {
	pb::Level level;
	// Start from the current level
	if (const auto lb = M2_LEVEL.LevelBlueprint()) {
		level = *lb;
	}

	// Clear fields that'll be filled here
	level.clear_background_layers();
	level.clear_objects();

	for (int i = 0; i < gBackgroundLayerCount; ++i) {
		for (const auto& [position, idAndspriteType] : _backgroundSpritePlaceholders[i]) {
			pb::mutable_get_or_create(level.mutable_background_layers(), i);
			auto* row = pb::mutable_get_or_create(level.mutable_background_layers(i)->mutable_background_rows(), position.y); // Get or create row
			*pb::mutable_get_or_create(row->mutable_items(), position.x) = std::get<m2g::pb::SpriteType>(idAndspriteType); // Set sprite type
		}
	}
	for (const auto& [position, idAndlevelObject] : _foregroundObjectPlaceholders) {
		level.add_objects()->CopyFrom(std::get<pb::LevelObject>(idAndlevelObject));
	}
	return pb::message_to_json_file(level, *M2_LEVEL.Path());
}

void m2::level_editor::State::PaintBackground(const VecI& position, m2g::pb::SpriteType spriteType) {
	// Delete previous placeholder
	EraseBackground({position.x, position.y, 1, 1});
	// Create new placeholder
	const auto bgPlaceholderId = obj::create_background_placeholder(VecF{position}, spriteType, GetSelectedBackgroundLayer());
	_backgroundSpritePlaceholders[I(GetSelectedBackgroundLayer())]
			.emplace(position, std::make_tuple(bgPlaceholderId, spriteType));
}
void m2::level_editor::State::PlaceForeground(const VecF& position, float orientation, m2g::pb::ObjectType objectType, m2g::pb::GroupType groupType, unsigned groupInstance) {
	const auto fgPlaceholderId = obj::create_foreground_placeholder(position, orientation, M2_GAME.object_main_sprites[objectType]);
	m2::pb::LevelObject levelObject;
	levelObject.mutable_position()->set_x(position.x);
	levelObject.mutable_position()->set_y(position.y);
	levelObject.set_orientation(orientation);
	levelObject.set_type(objectType);
	levelObject.mutable_group()->set_type(groupType);
	levelObject.mutable_group()->set_instance(groupInstance);
	_foregroundObjectPlaceholders.emplace(position, std::make_tuple(fgPlaceholderId, levelObject));
}
m2::RectF m2::level_editor::State::ForegroundSelectionArea() const {
	const auto* selection = M2_LEVEL.PrimarySelection();
	if (GetSnapToGridStatus()) {
		return *selection->CellSelectionRectM();
	} else {
		return *selection->SelectionRectM();
	}
}
