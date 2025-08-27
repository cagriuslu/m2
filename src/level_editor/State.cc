#include <m2/level_editor/State.h>
#include <m2/level_editor/Ui.h>
#include <m2/Game.h>
#include <m2/game/object/Placeholder.h>
#include <m2/game/Selection.h>
#include <Level.pb.h>
#include <m2/ui/widget/CheckboxWithText.h>
#include <m2/ui/widget/IntegerSelection.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/sheet_editor/Ui.h>
#include <m2/math/Line.h>
#include <m2/Log.h>

namespace {
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 80};

	/// If the selection state contains this object, the mouse is carrying an object, thus when the selection is ended,
	/// the object should be placed.
	class MovingAnObjectIndicator final : public m2::SelectionStateBase {};
}

m2::level_editor::State::State() : _persistentSpriteSheets(m2MoveOrThrowError(sheet_editor::PersistentSpriteSheets::LoadFile(M2_GAME.GetResources().GetSpriteSheetsPath()))) {}

m2::BackgroundDrawLayer m2::level_editor::State::GetSelectedBackgroundLayer() const {
	return static_cast<BackgroundDrawLayer>(
			I(M2_LEVEL.GetLeftHud()->FindWidget<widget::TextSelection>("BackgroundLayerSelection")->GetSelectedOptions()[0]));
}
bool m2::level_editor::State::GetSnapToGridStatus() const {
	return M2_LEVEL.GetLeftHud()->FindWidget<widget::CheckboxWithText>("SnapToGridCheckbox")->GetState();
}
std::vector<m2::level_editor::State::ForegroundObjectPlaceholderMap::const_iterator> m2::level_editor::State::GetForegroundObjectsOfType(m2g::pb::ObjectType objType) const {
	std::vector<ForegroundObjectPlaceholderMap::const_iterator> its;
	for (auto it = _foregroundObjectPlaceholders.begin(); it != _foregroundObjectPlaceholders.end(); ++it) {
		if (std::get<pb::LevelObject>(it->second).type() == objType) {
			its.emplace_back(it);
		}
	}
	return its;
}

std::vector<m2::pb::Fixture::FixtureTypeCase> m2::level_editor::State::GetSpriteFixtureTypes(const m2g::pb::SpriteType st) const {
	return _persistentSpriteSheets->SpriteFixtureTypes(st);
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
	if (M2_LEVEL.GetRightHud() && M2_LEVEL.GetRightHud()->Name() == "PlaceFgRightHud") {
		if (const auto sampledObject = ApplySampling(position); sampledObject != _foregroundObjectPlaceholders.end()) {
			if (not std::get<pb::LevelObject>(sampledObject->second).is_locked()) {
				// Remove the original object
				const auto id = std::get<Id>(sampledObject->second);
				M2_DEFER(CreateObjectDeleter(id));
				_foregroundObjectPlaceholders.erase(sampledObject);
				// Store the indicator that an object is being moved
				auto* primarySelection = M2_LEVEL.GetPrimarySelection();
				primarySelection->state = std::make_unique<MovingAnObjectIndicator>();
			}
		}
	} else if (M2_LEVEL.GetRightHud() && M2_LEVEL.GetRightHud()->Name() == "SampleFgRightHud") {
		// Applying sampling without removing the original object
		ApplySampling(position);
	}
}
void m2::level_editor::State::HandleMouseSecondaryButton(const VecF& position) {
	if (M2_LEVEL.GetRightHud()->Name() == "PaintBgRightHud") {
		// Background operations are supported only for the positive quadrant
		if (not position.RoundI().IsNegative()) {
			if (const auto selections = M2_LEVEL.GetRightHud()->FindWidget<widget::TextSelection>("SpriteTypeSelection")->GetSelectedOptions();
				not selections.empty()) {
				const auto selectedSpriteType = static_cast<m2g::pb::SpriteType>(I(selections[0]));
				PaintBackground(VecI{position.RoundI().x, position.RoundI().y}, selectedSpriteType);
			}
		}
	} else if (M2_LEVEL.GetRightHud()->Name() == "SampleBgRightHud") {
		// Background operations are supported only for the positive quadrant
		if (not position.RoundI().IsNegative()) {
			if (const auto it = _backgroundSpritePlaceholders[I(GetSelectedBackgroundLayer())].find(position.RoundI());
				it != _backgroundSpritePlaceholders[I(GetSelectedBackgroundLayer())].end()) {
				// Find and press the Paint button
				M2_LEVEL.GetLeftHud()->FindWidget<widget::Text>("PaintBgButton")->trigger_action();
				// Find sprite type selection and set it
				auto* spriteTypeSelection = M2_LEVEL.GetRightHud()->FindWidget<widget::TextSelection>("SpriteTypeSelection");
				// Find the index of the option that corresponds to the sprite
				const auto& options = spriteTypeSelection->GetOptions();
				for (size_t i = 0; i < options.size(); ++i) {
					if (I(options[i].blueprint_option.return_value) == static_cast<int>(std::get<m2g::pb::SpriteType>(it->second))) {
						spriteTypeSelection->SetUniqueSelectionIndex(i);
						break;
					}
				}
			}
		}
	} else if (M2_LEVEL.GetRightHud()->Name() == "PlaceFgRightHud") {
		if (const auto selections = M2_LEVEL.GetRightHud()->FindWidget<widget::TextSelection>("ObjectTypeSelection")->GetSelectedOptions(); not selections.empty()) {
			const auto selectedObjectType = static_cast<m2g::pb::ObjectType>(I(selections[0]));
			const auto selectedGroupType = static_cast<m2g::pb::GroupType>(I(M2_LEVEL.GetRightHud()->FindWidget<widget::TextSelection>("GroupTypeSelection")->GetSelectedOptions()[0]));
			const auto selectedGroupInstance = M2_LEVEL.GetRightHud()->FindWidget<widget::IntegerSelection>("GroupInstanceSelection")->value();
			const auto orientation = ToRadians(M2_LEVEL.GetRightHud()->FindWidget<widget::IntegerSelection>("OrientationInput")->value());
			const auto snapToGrid = M2_LEVEL.GetLeftHud()->FindWidget<widget::CheckboxWithText>("SnapToGridCheckbox")->GetState();
			const auto splitCount = M2_LEVEL.GetLeftHud()->FindWidget<widget::IntegerSelection>("CellSplitCount")->value();
			const auto placePosition = snapToGrid ? M2_GAME.MousePositionWorldM().RoundToBin(splitCount) : position;
			PlaceForeground(placePosition, orientation, selectedObjectType, selectedGroupType, selectedGroupInstance);
		}
	}
}
void m2::level_editor::State::HandleMousePrimarySelectionComplete(const VecF& firstPosition, const VecF& secondPosition) {
	if (M2_LEVEL.GetRightHud() && M2_LEVEL.GetRightHud()->Name() == "PlaceFgRightHud" && M2_LEVEL.GetPrimarySelection()->state) {
		if (dynamic_cast<MovingAnObjectIndicator*>(M2_LEVEL.GetPrimarySelection()->state.get())) {
			// An object was picked up from the ground. Place it.
			HandleMouseSecondaryButton(secondPosition);
			// No longer moving an object
			M2_LEVEL.GetPrimarySelection()->state.reset();
		}
	}
	if (M2_LEVEL.GetRightHud() && M2_LEVEL.GetRightHud()->Name() == "DrawFgRightHud" && M2_LEVEL.GetRightHud()->state) {
		if (const auto* drawFgState = dynamic_cast<DrawFgRightHudState*>(M2_LEVEL.GetRightHud()->state.get())) {
			const auto* fixtureSelectionWidget = M2_LEVEL.GetRightHud()->FindWidget<widget::TextSelection>("FixtureSelection");
			if (const auto selectedIndexes = fixtureSelectionWidget->GetSelectedIndexes(); not selectedIndexes.empty()) {
				const auto selectedIndex = selectedIndexes[0];
				if (const auto fixtureType = GetSpriteFixtureTypes(drawFgState->SelectedObjectMainSpriteType())[selectedIndex];
						fixtureType == pb::Fixture::FixtureTypeCase::kChain) {
					const auto fgObjectIt = GetForegroundObjectsOfType(drawFgState->SelectedObjectType())[0];
					const auto firstPositionPx = WorldCoordinateToSpriteCoordinate(fgObjectIt, firstPosition);
					const auto spritePpm = F(SpritePpm(fgObjectIt));
					const auto& spritePb = GetSpritePb(drawFgState->SelectedObjectMainSpriteType());
					const auto& chain = spritePb.regular().fixtures(selectedIndex).chain();
					const auto closestPointIndex = FindClosestChainPointInRange(chain, spritePpm, firstPositionPx);
					if (not closestPointIndex) {
						return;
					}

					const auto splitCount = M2_LEVEL.GetLeftHud()->FindWidget<widget::IntegerSelection>("CellSplitCount")->value();
					const auto binnedSecondPositionPx = secondPosition.RoundToBin(splitCount);
					const auto pointOffset = WorldCoordinateToSpriteCoordinate(fgObjectIt, binnedSecondPositionPx);
					_persistentSpriteSheets->ModifySprite(drawFgState->SelectedObjectMainSpriteType(), [&](pb::Sprite& sprite) {
						if (auto* fixture = sprite.mutable_regular()->mutable_fixtures(selectedIndex); fixture->has_chain()) {
							auto* chain_ = fixture->mutable_chain();
							if (auto* point = chain_->mutable_points(*closestPointIndex); point) {
								point->set_x(pointOffset.x);
								point->set_y(pointOffset.y);
							}
						}
					});
				}
			}
		}
	}
}

void m2::level_editor::State::EraseBackground(const RectI& area) {
	const auto selectedBgLayerIndex = I(GetSelectedBackgroundLayer());
	area.ForEachCell([&](const VecI& pos) {
		if (const auto it = _backgroundSpritePlaceholders[selectedBgLayerIndex].find(pos);
				it != _backgroundSpritePlaceholders[selectedBgLayerIndex].end()) {
			const auto id = std::get<Id>(it->second);
			M2_DEFER(CreateObjectDeleter(id));
			_backgroundSpritePlaceholders[selectedBgLayerIndex].erase(it);
		}
	});
}
void m2::level_editor::State::CopyBackground(const RectI& area) {
	_backgroundSpriteClipboard.clear();

	const auto selectedBgLayerIndex = I(GetSelectedBackgroundLayer());
	area.ForEachCell([&](const VecI& pos) {
		const auto positionInClipboard = pos - area.GetTopLeftPoint();
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
	area.ForEachCell([&](const VecI& pos) {
		const auto index = Random(spriteSet.size());
		PaintBackground(pos, spriteSet[index]);
	});
}

void m2::level_editor::State::RemoveForegroundObject() {
	const auto selection = ForegroundSelectionArea();
	for (auto it = _foregroundObjectPlaceholders.begin(); it != _foregroundObjectPlaceholders.end();) {
		if (selection.DoesContain(it->first) && not std::get<pb::LevelObject>(it->second).is_locked()) {
			const auto id = std::get<Id>(it->second);
			M2_DEFER(CreateObjectDeleter(id));
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
		if (selection.DoesContain(it->first)) {
			const auto positionInClipboard = it->first - selection.GetTopLeftPoint();
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
		const auto position = selection.GetTopLeftPoint() + VecF{levelObject.position().x(), levelObject.position().y()};
		PlaceForeground(position, levelObject.orientation(), levelObject.type(), levelObject.group().type(), levelObject.group().instance());
	}
}

int m2::level_editor::State::AddChain(const m2g::pb::SpriteType st, const int insertIndex) {
	return _persistentSpriteSheets->AddFixtureToSprite(st, pb::Fixture::FixtureTypeCase::kChain, insertIndex);
}
void m2::level_editor::State::RemoveFixture(const m2g::pb::SpriteType st, const int selectedIndex) {
	_persistentSpriteSheets->RemoveFixtureFromSprite(st, selectedIndex);
}
void m2::level_editor::State::StoreWorldPoint(const int selectedIndex, const VecF& pointM) {
	const auto splitCount = M2_LEVEL.GetLeftHud()->FindWidget<widget::IntegerSelection>("CellSplitCount")->value();
	const auto binnedPointM = pointM.RoundToBin(splitCount);

	const auto& state = dynamic_cast<DrawFgRightHudState&>(*M2_LEVEL.GetRightHud()->state);
	const auto fgObjectIt = GetForegroundObjectsOfType(state.SelectedObjectType())[0];
	const auto pointOffset = WorldCoordinateToSpriteCoordinate(fgObjectIt, binnedPointM);
	StoreFixturePoint(state.SelectedObjectMainSpriteType(), selectedIndex, pointOffset);
}
void m2::level_editor::State::StoreArc(int selectedIndex, const VecF& pointM, const ArcDescription& arc) {
	const auto splitCount = M2_LEVEL.GetLeftHud()->FindWidget<widget::IntegerSelection>("CellSplitCount")->value();
	const auto binnedPointM = pointM.RoundToBin(splitCount);

	auto& state = dynamic_cast<DrawFgRightHudState&>(*M2_LEVEL.GetRightHud()->state);
	const auto fgObjectIt = GetForegroundObjectsOfType(state.SelectedObjectType())[0];
	const auto pointOffset = WorldCoordinateToSpriteCoordinate(fgObjectIt, binnedPointM);

	const auto& spritePb = GetSpritePb(state.SelectedObjectMainSpriteType());
	const auto& chain = spritePb.regular().fixtures(selectedIndex).chain();
	if (chain.points_size() == 0) {
		LOG_WARN("Unable to draw arc with single point");
		return;
	}
	const auto& prevPointOffset = VecF{chain.points(chain.points_size() - 1)};

	const auto angleInRads = ToRadians(arc.angleInDegrees);
	StoreArc(selectedIndex, prevPointOffset, pointOffset, angleInRads, arc.pieceCount, arc.drawTowardsRight);
}
void m2::level_editor::State::StoreTangent(const int selectedIndex, const TangentDescription& tangent) {
	const auto& state = dynamic_cast<DrawFgRightHudState&>(*M2_LEVEL.GetRightHud()->state);
	const auto& spritePb = GetSpritePb(state.SelectedObjectMainSpriteType());
	const auto& chain = spritePb.regular().fixtures(selectedIndex).chain();

	const auto chainPieceToLine = [&](const int point_index) -> Line {
		const auto point1 = VecF{chain.points(point_index)};
		const auto point2 = VecF{chain.points((point_index + 1) % chain.points_size())};
		return Line::FromPoints(point1, point2);
	};
	const auto line1 = chainPieceToLine(tangent.first);
	const auto line2 = chainPieceToLine(tangent.second);
	const auto intersectionOfLines = line1.GetIntersectionPointWith(line2);
	if (not intersectionOfLines) {
		return;
	}
	const auto absoluteAngleBetweenLines = fabsf(line1.GetAngleTo(line2));
	const auto cornerInnerAngle = PI - absoluteAngleBetweenLines;
	const auto distanceFromIntersectionToTangentPoint = tangent.radius / tanf(cornerInnerAngle / 2.0f);
	const auto tangentOnLine1 = intersectionOfLines->MoveTowards(line1.GetParallel() * -1.0f, distanceFromIntersectionToTangentPoint);
	const auto tangentOnLine2 = intersectionOfLines->MoveTowards(line2.GetParallel(), distanceFromIntersectionToTangentPoint);
	const auto arcAngle = (PI_DIV2 - (cornerInnerAngle / 2.0f)) * 2.0f;

	// Cache points before the first piece
	std::vector<pb::VecF> pointsBefore;
	for (int i = 0; i <= tangent.first; ++i) {
		pointsBefore.emplace_back(chain.points(i));
	}
	// Also add the first tangent
	pointsBefore.emplace_back(tangentOnLine1);

	// Cache points after the second piece
	std::vector<pb::VecF> pointsAfter;
	for (int i = tangent.second + 1; i < chain.points_size(); ++i) {
		pointsAfter.emplace_back(chain.points(i));
	}

	// Remove every point
	const auto pointCount = chain.points_size();
	for (int i = 0; i < pointCount; ++i) {
		UndoPoint(selectedIndex);
	}

	// Add points before and the first tangent
	for (const auto& pointBefore : pointsBefore) {
		StoreFixturePoint(state.SelectedObjectMainSpriteType(), selectedIndex, VecF{pointBefore});
	}

	// Draw the arc, sign of the angle determines the direction of the arc
	const auto angleTo = line1.GetAngleTo(line2);
	StoreArc(selectedIndex, tangentOnLine1, tangentOnLine2, arcAngle, tangent.pieceCount, angleTo < 0.0f);

	// Add point after
	for (const auto& pointAfter : pointsAfter) {
		StoreFixturePoint(state.SelectedObjectMainSpriteType(), selectedIndex, VecF{pointAfter});
	}
}
void m2::level_editor::State::UndoPoint(const int selectedIndex) {
	const auto& state = dynamic_cast<DrawFgRightHudState&>(*M2_LEVEL.GetRightHud()->state);
	_persistentSpriteSheets->ModifySprite(state.SelectedObjectMainSpriteType(), [&](pb::Sprite& sprite) {
		if (auto* fixture = sprite.mutable_regular()->mutable_fixtures(selectedIndex); fixture->has_chain()) {
			if (auto* chain = fixture->mutable_chain(); chain->points_size()) {
				auto* points = chain->mutable_points();
				points->erase(points->end() - 1);
			}
		}
	});
}

void m2::level_editor::State::Draw() const {
	const auto splitCount = M2_LEVEL.GetLeftHud()->FindWidget<widget::IntegerSelection>("CellSplitCount")->value();

	const auto drawGridSelectionIfActive = [] {
		if (const auto* selection = M2_LEVEL.GetPrimarySelection()) {
			if (const auto integerSelection = selection->IntegerSelectionRectM()) {
				integerSelection->ForEachCell([=](const VecI& cell) {
					Graphic::ColorCell(cell, SELECTION_COLOR);
				});
			}
		}
	};
	const auto drawFixturesOfObject = [this](const m2g::pb::ObjectType ot, const pb::Sprite& spritePb, const int ppm, const bool background, const std::optional<int>& selectedFixtureIndex, const std::optional<std::pair<int,VecF>>& overridePointPosition) {
		const auto color = background ? RGBA::TransparentRed : RGBA::TransparentGreen;
		const auto lastPointColor = RGBA::Green;
		for (int i = 0; i < spritePb.regular().fixtures_size(); ++i) {
			if (const auto& fixture = spritePb.regular().fixtures(i); fixture.has_chain()) {
				const auto fgObjectIt = GetForegroundObjectsOfType(ot)[0];
				const auto objectOrigin = fgObjectIt->first;
				if (const auto& points = fixture.chain().points(); points.size() == 1) {
					Graphic::DrawCross(objectOrigin + VecF{points[0]} / ppm, 10, lastPointColor);
				} else if (1 < points.size()) {
					for (int j = 0; j < points.size() - 1; ++j) {
						const auto thisPoint = overridePointPosition && *selectedFixtureIndex == i && overridePointPosition->first == j ? overridePointPosition->second : VecF{points[j]};
						const auto nextPoint = overridePointPosition && *selectedFixtureIndex == i && overridePointPosition->first == j + 1 ? overridePointPosition->second : VecF{points[j + 1]};
						Graphic::DrawLine(objectOrigin + thisPoint / ppm, objectOrigin + nextPoint / ppm, color);
						if (selectedFixtureIndex && *selectedFixtureIndex == i) {
							Graphic::DrawCross(objectOrigin + thisPoint / ppm, 10, color);
						}
					}
					// Draw the last point as well, which isn't visited in the for loop above
					if (selectedFixtureIndex && *selectedFixtureIndex == i) {
						const auto lastPoint = VecF{points[points.size() - 1]};
						Graphic::DrawCross(objectOrigin + lastPoint / ppm, 10, lastPointColor);
					}
				}
			}
		}
	};

	if (M2_LEVEL.GetRightHud()->Name() == "SelectBgRightHud") {
		drawGridSelectionIfActive();
	} else if (M2_LEVEL.GetRightHud()->Name() == "SelectFgRightHud") {
		if (GetSnapToGridStatus()) {
			drawGridSelectionIfActive();
		} else {
			if (const auto* selection = M2_LEVEL.GetPrimarySelection()) {
				if (const auto rawSelection = selection->SelectionRectM()) {
					Graphic::ColorRect(*rawSelection, SELECTION_COLOR);
				}
			}
		}
	} else if (M2_LEVEL.GetRightHud()->Name() == "DrawFgRightHud") {
		const auto* fixtureSelectionWidget = M2_LEVEL.GetRightHud()->FindWidget<widget::TextSelection>("FixtureSelection");
		const auto selectedIndexes = fixtureSelectionWidget->GetSelectedIndexes();
		const auto selectedFixtureIndex = selectedIndexes.empty() ? std::optional<int>{} : selectedIndexes[0];

		// Draw selection (as cross)
		if (const auto selections = M2_LEVEL.GetSecondarySelection()->SelectionsM()) {
			const auto point = selections->first.RoundToBin(splitCount);
			Graphic::DrawCross(point, sheet_editor::CROSS_COLOR);
		}
		// Draw already existing fixtures
		const auto& state = dynamic_cast<DrawFgRightHudState&>(*M2_LEVEL.GetRightHud()->state);
		const auto ppm = std::get<Sprite>(M2_GAME.GetSpriteOrTextLabel(state.SelectedObjectMainSpriteType())).Ppm();
		const auto overridePointPosition = [&]() -> std::optional<std::pair<int,VecF>> {
			// Draw drag-and-drop
			if (const auto selection = M2_LEVEL.GetPrimarySelection()->SelectionsM(); selection && not M2_LEVEL.GetPrimarySelection()->IsComplete()) {
				if (selectedFixtureIndex) {
					if (const auto fixtureType = GetSpriteFixtureTypes(state.SelectedObjectMainSpriteType())[*selectedFixtureIndex];
							fixtureType == pb::Fixture::FixtureTypeCase::kChain) {
						const auto fgObjectIt = GetForegroundObjectsOfType(state.SelectedObjectType())[0];
						const auto firstPositionPx = WorldCoordinateToSpriteCoordinate(fgObjectIt, selection->first);
						const auto spritePpm = F(SpritePpm(fgObjectIt));
						const auto& spritePb = GetSpritePb(state.SelectedObjectMainSpriteType());
						const auto& chain = spritePb.regular().fixtures(*selectedFixtureIndex).chain();
						const auto closestPointIndex = FindClosestChainPointInRange(chain, spritePpm, firstPositionPx);
						if (not closestPointIndex) {
							return std::nullopt;
						}

						const auto binnedSecondPositionPx = selection->second.RoundToBin(splitCount);
						const auto pointOffset = WorldCoordinateToSpriteCoordinate(fgObjectIt, binnedSecondPositionPx);
						return std::make_pair(*closestPointIndex, pointOffset);
					}
				}
			}
			return std::nullopt;
		}();
		drawFixturesOfObject(state.SelectedObjectType(), GetSpritePb(state.SelectedObjectMainSpriteType()), ppm, false, selectedFixtureIndex, overridePointPosition);
	}

	for (const auto objType : physicsObjectsToDraw) {
		const auto mainSpriteType = *M2_GAME.GetMainSpriteOfObject(objType);
		const auto mainSpritePpm = std::get<Sprite>(M2_GAME.GetSpriteOrTextLabel(mainSpriteType)).Ppm();
		drawFixturesOfObject(objType, GetSpritePb(mainSpriteType), mainSpritePpm, true, std::nullopt, std::nullopt);
	}

	// Draw grid if enabled
	if (M2_LEVEL.GetLeftHud()->FindWidget<widget::CheckboxWithText>("ShowGridCheckbox")->GetState()) {
		Graphic::DrawGridLines(-0.5f, 1.0f, {127, 127, 255, 80});
		if (splitCount != 1) {
			Graphic::DrawGridLines(-0.5f, 1.0f / F(splitCount), {127, 127, 255, 60});
		}
	}
	// Draw axes
	Graphic::DrawHorizontalLine(0.0f, RGBA::Red);
	Graphic::DrawVerticalLine(0.0f, RGBA::Red);
}
m2::void_expected m2::level_editor::State::Save() {
	pb::Level level;
	// Start from the current level
	if (const auto lb = M2_LEVEL.GetLevelBlueprint()) {
		level = *lb;
	}

	// Clear fields that'll be filled here
	level.clear_background_layers();
	level.clear_objects();

	for (int i = 0; i < gBackgroundDrawLayerCount; ++i) {
		for (const auto& [position, idAndspriteType] : _backgroundSpritePlaceholders[i]) {
			pb::mutable_get_or_create(level.mutable_background_layers(), i);
			auto* row = pb::mutable_get_or_create(level.mutable_background_layers(i)->mutable_background_rows(), position.y); // Get or create row
			*pb::mutable_get_or_create(row->mutable_items(), position.x) = std::get<m2g::pb::SpriteType>(idAndspriteType); // Set sprite type
		}
	}
	for (const auto& [position, idAndlevelObject] : _foregroundObjectPlaceholders) {
		level.add_objects()->CopyFrom(std::get<pb::LevelObject>(idAndlevelObject));
	}

	// Save level
	const auto levelSaveSuccess = pb::message_to_json_file(level, *M2_LEVEL.GetLevelFilePath());
	m2ReflectUnexpected(levelSaveSuccess);

	// Save sprite sheet
	return _persistentSpriteSheets->Save();
}

void m2::level_editor::State::PaintBackground(const VecI& position, m2g::pb::SpriteType spriteType) {
	// Delete previous placeholder
	EraseBackground({position.x, position.y, 1, 1});
	// Create new placeholder
	const auto bgPlaceholderId = obj::CreateBackgroundPlaceholder(VecF{position}, spriteType, GetSelectedBackgroundLayer());
	_backgroundSpritePlaceholders[I(GetSelectedBackgroundLayer())]
			.emplace(position, std::make_tuple(bgPlaceholderId, spriteType));
}
void m2::level_editor::State::PlaceForeground(const VecF& position, float orientation, m2g::pb::ObjectType objectType, m2g::pb::GroupType groupType, unsigned groupInstance) {
	const auto fgPlaceholderId = obj::CreateForegroundPlaceholder(position, orientation, M2_GAME.GetMainSpriteOfObject(objectType));
	m2::pb::LevelObject levelObject;
	levelObject.mutable_position()->set_x(position.x);
	levelObject.mutable_position()->set_y(position.y);
	levelObject.set_orientation(orientation);
	levelObject.set_type(objectType);
	levelObject.mutable_group()->set_type(groupType);
	levelObject.mutable_group()->set_instance(groupInstance);
	_foregroundObjectPlaceholders.emplace(position, std::make_tuple(fgPlaceholderId, levelObject));
}
m2::level_editor::State::ForegroundObjectPlaceholderMap::iterator m2::level_editor::State::ApplySampling(const VecF& position) {
	auto foundIt = _foregroundObjectPlaceholders.end();
	float closestDistanceSquare = INFINITY;
	for (auto it = _foregroundObjectPlaceholders.begin(); it != _foregroundObjectPlaceholders.end(); ++it) {
		// Find the closest object in 1-meter radius
		if (const auto distanceSquared = position.GetDistanceToSquared(it->first); distanceSquared <= 1.0f && distanceSquared < closestDistanceSquare) {
			foundIt = it;
			closestDistanceSquare = distanceSquared;
		}
	}
	if (foundIt != _foregroundObjectPlaceholders.end()) {
		// Find and press the Place button
		if (not M2_LEVEL.GetRightHud() || M2_LEVEL.GetRightHud()->Name() != "PlaceFgRightHud") {
			// Press the button only if not already pressed, otherwise the selection is recreated.
			M2_LEVEL.GetLeftHud()->FindWidget<widget::Text>("PlaceFgButton")->trigger_action();
		}

		// Find object properties and set it

		const auto orientationDegreesUnbounded = RoundI(ToDegrees(std::get<pb::LevelObject>(foundIt->second).orientation()));
		const auto orientationDegrees = ((orientationDegreesUnbounded % 360) + 360) % 360;
		auto* orientationInput = M2_LEVEL.GetRightHud()->FindWidget<widget::IntegerSelection>("OrientationInput");
		orientationInput->SetValue(orientationDegrees);

		const auto objectType = std::get<pb::LevelObject>(foundIt->second).type();
		auto* objectTypeSelection = M2_LEVEL.GetRightHud()->FindWidget<widget::TextSelection>("ObjectTypeSelection");
		// Find the index of the option that corresponds to the object type
		const auto& objectTypeOptions = objectTypeSelection->GetOptions();
		for (size_t i = 0; i < objectTypeOptions.size(); ++i) {
			if (I(objectTypeOptions[i].blueprint_option.return_value) == I(objectType)) {
				objectTypeSelection->SetUniqueSelectionIndex(i);
				break;
			}
		}

		const auto groupType = std::get<pb::LevelObject>(foundIt->second).group().type();
		auto* groupTypeSelection = M2_LEVEL.GetRightHud()->FindWidget<widget::TextSelection>("GroupTypeSelection");
		// Find the index of the option that corresponds to the group type
		const auto& groupTypeOptions = groupTypeSelection->GetOptions();
		for (size_t i = 0; i < groupTypeOptions.size(); ++i) {
			if (I(groupTypeOptions[i].blueprint_option.return_value) == I(groupType)) {
				groupTypeSelection->SetUniqueSelectionIndex(i);
				break;
			}
		}

		const auto groupInstance = std::get<pb::LevelObject>(foundIt->second).group().instance();
		auto* groupInstanceSelection = M2_LEVEL.GetRightHud()->FindWidget<widget::IntegerSelection>("GroupInstanceSelection");
		groupInstanceSelection->SetValue(groupInstance);
	}
	return foundIt;
}
m2::RectF m2::level_editor::State::ForegroundSelectionArea() const {
	const auto* selection = M2_LEVEL.GetPrimarySelection();
	if (GetSnapToGridStatus()) {
		return *selection->CellSelectionRectM();
	} else {
		return *selection->SelectionRectM();
	}
}
int m2::level_editor::State::SpritePpm(ForegroundObjectPlaceholderMap::const_iterator fgObject) {
	const auto objectType = std::get<pb::LevelObject>(fgObject->second).type();
	const auto spriteType = *M2_GAME.GetMainSpriteOfObject(objectType);
	const auto& sprite = std::get<Sprite>(M2_GAME.GetSpriteOrTextLabel(spriteType));
	return sprite.Ppm();
}
m2::VecF m2::level_editor::State::WorldCoordinateToSpriteCoordinate(const ForegroundObjectPlaceholderMap::const_iterator fgObject, const VecF& worldCoordinate) {
	const auto objectOriginM = fgObject->first;
	const auto pointOffsetM = worldCoordinate - objectOriginM;
	const auto ppm = SpritePpm(fgObject);
	return pointOffsetM * ppm;
}
void m2::level_editor::State::StoreFixturePoint(const m2g::pb::SpriteType st, const int selectedIndex, const VecF& spriteOriginToPointVec) {
	_persistentSpriteSheets->ModifySprite(st, [&](pb::Sprite& sprite) {
		if (auto* fixture = sprite.mutable_regular()->mutable_fixtures(selectedIndex); fixture->has_chain()) {
			auto* chain = fixture->mutable_chain();
			auto* point = chain->add_points();
			point->set_x(spriteOriginToPointVec.x);
			point->set_y(spriteOriginToPointVec.y);
		}
	});
}
void m2::level_editor::State::StoreArc(const int selectedIndex, const VecF& fromPointOffset, const VecF& toPointOffset, const float angleInRads, const int pieceCount, const bool drawTowardsRight) {
	const auto& state = dynamic_cast<DrawFgRightHudState&>(*M2_LEVEL.GetRightHud()->state);
	const auto& spritePb = GetSpritePb(state.SelectedObjectMainSpriteType());
	const auto& chain = spritePb.regular().fixtures(selectedIndex).chain();
	if (chain.points_size() == 0) {
		LOG_WARN("Unable to draw arc with single point");
		return;
	}
	const auto vectorBetweenPoints = toPointOffset - fromPointOffset;
	const auto distanceBetweenPoints = vectorBetweenPoints.GetLength();

	// Find the radius of the circle
	const auto smallAngle = PI < angleInRads ? PI_MUL2 - angleInRads : angleInRads;
	const auto radius = distanceBetweenPoints / 2.0f / sinf(smallAngle / 2.0f);
	// To find the center of the circle, rotate the vector between the points, and walk the line
	const auto angleToRotate = PI_DIV2 - (smallAngle / 2.0f);
	const auto angleToRotateWithSelectedDirection = drawTowardsRight ? -angleToRotate : angleToRotate;
	const auto rotatedVectorBetweenPoints = vectorBetweenPoints.Rotate(PI < angleInRads ? -angleToRotateWithSelectedDirection : angleToRotateWithSelectedDirection);
	const auto prevPointOffsetToCircleCenter = rotatedVectorBetweenPoints.WithLength(radius);
	const auto circleCenter = fromPointOffset + prevPointOffsetToCircleCenter;

	// Generate points from center and radius
	const auto circleCenterToPrevPoint = prevPointOffsetToCircleCenter * -1.0f;
	const auto angleStep = angleInRads / F(pieceCount);
	for (auto i = 0; i < pieceCount; ++i) {
		const auto rotationAmount = F(i + 1) * (drawTowardsRight ? angleStep * -1.0f : angleStep);
		const auto circleCenterToArcPoint = circleCenterToPrevPoint.Rotate(rotationAmount);
		const auto arcPoint = circleCenterToArcPoint + circleCenter;
		StoreFixturePoint(state.SelectedObjectMainSpriteType(), selectedIndex, arcPoint);
	}
}
std::optional<int> m2::level_editor::State::FindClosestChainPointInRange(const pb::Fixture_ChainFixture& chain, const int spritePpm, const VecF& positionPx) {
	std::optional<std::pair<int, float>> closestPointIndexAndDistanceSq;
	for (int i = 0; i < chain.points_size(); ++i) {
		if (const auto distanceSq = VecF{chain.points(i)}.GetDistanceToSquared(positionPx); distanceSq < spritePpm) {
			if (not closestPointIndexAndDistanceSq || distanceSq < closestPointIndexAndDistanceSq->second) {
				closestPointIndexAndDistanceSq = std::make_pair(i, distanceSq);
			}
		}
	}

	if (not closestPointIndexAndDistanceSq) {
		return std::nullopt;
	}
	return closestPointIndexAndDistanceSq->first;
}
