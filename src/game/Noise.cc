#include "m2/game/Noise.h"

#include "m2/common/math/VecI.h"

using namespace m2;

std::pair<std::vector<VecE>, int> m2::GeneratePointField(const GeneratePointFieldParams& params) {
	std::vector<VecE> points;
	int stride = 0;
	for (auto y = params.inclusiveRect.y; y <= params.inclusiveRect.GetY2(); y += params.step) {
		for (auto x = params.inclusiveRect.x; x <= params.inclusiveRect.GetX2(); x += params.step) {
			points.emplace_back(x, y);
			if (y == params.inclusiveRect.y) {
				++stride;
			}
		}
	}
	return std::make_pair(std::move(points), stride);
}

void m2::ApplyNoiseToAllPoints(std::vector<VecE>& points, Distribution& offsetDistribution, Rng& rng) {
	for (auto& point : points) {
		Exact first, second;
		offsetDistribution.GenerateNextExact(rng, first);
		offsetDistribution.GenerateNextExact(rng, second);
		const auto offset = VecE{first, second};
		point += offset;
	}
}

Exact DecayEnvelope::GetValueAt(const Exact t) const {
	if (t < attackDuration) {
		// Attack: Linear increase from zero to peakAmplitude. Use "similar triangles".
		//         t / attackDuration = retval / peakAmplitude
		return t.MultiplyDivide(peakAmplitude, attackDuration);
	}
	if (t < attackDuration + decayDuration) {
		// Decay: Linear decrease from peakAmplitude to sustainAmplitude.
		//        decay per t => (peakAmplitude - sustainAmplitude) / decayDuration
		//        initial value => peakAmplitude
		//        time spent in decay => t - attackDuration
		//        retval = (initial value) - (unit decay per t) * (time spent in decay)
		const auto differenceBetweenLevels = peakAmplitude - sustainAmplitude;
		const auto timeInDecay = t - attackDuration;
		const auto step = timeInDecay.MultiplyDivide(differenceBetweenLevels, decayDuration);
		return peakAmplitude - step;
	}
	if (t < attackDuration + decayDuration + sustainDuration) {
		// Sustain
		return sustainAmplitude;
	}
	// Release: Linear decrease from sustainAmplitude to zero.
	//          decay per t => sustainAmplitude / releaseDuration
	//          initial value => sustainAmplitude
	//          time spent in release => t - (sustain end)
	//          retval = (initial value) - (decay per t) * (time spent in release)
	const auto timeInRelease = t - attackDuration - decayDuration - sustainDuration;
	const auto step = timeInRelease.MultiplyDivide(sustainAmplitude, releaseDuration);
	const auto releaseValue = sustainAmplitude - step;
	return Exact::Zero() < releaseValue ? releaseValue : Exact::Zero();
}
Exact DecayEnvelope::GetExtend() const {
	return attackDuration + decayDuration + sustainDuration + releaseDuration;
}

void m2::ApplyPointSourceOffset(std::vector<VecE>& points, const ApplyPointSourceOffsetParams& params) {
	for (auto& point : points) {
		const auto distanceToSource = params.source.GetDistanceToFE(point);
		const auto gainAtPoint = params.gainEnvelope.GetValueAt(distanceToSource);
		const auto offsetAtPoint = params.offsetAtUnitGain * gainAtPoint;
		point += offsetAtPoint;
	}
}

void m2::ApplyPointSourceAttraction(std::vector<VecE>& points, const ApplyPointSourceAttractionParams& params) {
	for (auto& point : points) {
		const auto vectorToSource = params.source - point;
		const auto distanceToSource = vectorToSource.GetLengthFE();
		const auto gainAtPoint = params.gainEnvelope.GetValueAt(distanceToSource);
		const auto attractionAmplitudeAtPoint = params.attractionAtUnitGain * gainAtPoint;
		const auto attractionAtPointX = vectorToSource.GetX().MultiplyDivide(attractionAmplitudeAtPoint, distanceToSource);
		const auto attractionAtPointY = vectorToSource.GetY().MultiplyDivide(attractionAmplitudeAtPoint, distanceToSource);
		const auto attractionVector = VecE{attractionAtPointX, attractionAtPointY};
		point += attractionVector;
	}
}

void m2::ApplyVerticalOffsetToColumnIndex(std::vector<VecE>& points, const int stride, const ApplyVerticalOffsetToColumnIndexParams& params) {
	if (params.xSource < 0 || stride <= params.xSource) {
		throw M2_ERROR("xSource out-of-bounds");
	}

	const auto ApplyVerticalOffsetToColumnIndex = [&](const int x, const Exact yOffset) {
		ForEachPointInColumn(points, stride, x, [yOffset](const VecI&, VecE& point) {
			point = VecE{point.GetX(), point.GetY() + yOffset};
		});
	};

	const auto offsetAtSource = params.yOffsetAtUnitGain * (params.leftGainEnvelope.GetValueAt({}) + params.rightGainEnvelope.GetValueAt({})) / Exact{2};
	// At source
	ApplyVerticalOffsetToColumnIndex(params.xSource, offsetAtSource);
	// Left of source
	const auto leftBegin = params.xSource - 1;
	const auto leftCount = params.leftGainEnvelope.GetExtend().ToInteger();
	for (int i = 0, current = leftBegin; i < leftCount && 0 <= current; ++i, --current) {
		const auto t = params.xSource - current;
		const auto offset = params.yOffsetAtUnitGain * params.leftGainEnvelope.GetValueAt(Exact{t});
		ApplyVerticalOffsetToColumnIndex(current, offset);
	}
	// Right of source
	const auto rightBegin = params.xSource + 1;
	const auto rightCount = params.rightGainEnvelope.GetExtend().ToInteger();
	for (int i = 0, current = rightBegin; i < rightCount && current < stride; ++i, ++current) {
		const auto t = current - params.xSource;
		const auto offset = params.yOffsetAtUnitGain * params.rightGainEnvelope.GetValueAt(Exact{t});
		ApplyVerticalOffsetToColumnIndex(current, offset);
	}
}

void m2::ApplyHorizontalOffsetToRowIndex(std::vector<VecE>& points, const int stride, const ApplyHorizontalOffsetToRowIndexParams& params) {
	const auto rowCount = I(points.size()) / stride;
	if (params.ySource < 0 || rowCount <= params.ySource) {
		throw M2_ERROR("ySource out-of-bounds");
	}

	const auto ApplyHorizontalOffsetToRow = [&](const int y, const Exact xOffset) {
		ForEachPointInRow(points, stride, y, [xOffset](const VecI&, VecE& point) {
			point = VecE{point.GetX() + xOffset, point.GetY()};
		});
	};

	const auto offsetAtSource = params.xOffsetAtUnitGain * (params.topGainEnvelope.GetValueAt({}) + params.bottomGainEnvelope.GetValueAt({})) / Exact{2};
	// At source
	ApplyHorizontalOffsetToRow(params.ySource, offsetAtSource);
	// Top of source
	const auto topBegin = params.ySource - 1;
	const auto topCount = params.topGainEnvelope.GetExtend().ToInteger();
	for (int i = 0, current = topBegin; i < topCount && 0 <= current; ++i, --current) {
		const auto t = params.ySource - current;
		const auto offset = params.xOffsetAtUnitGain * params.topGainEnvelope.GetValueAt(Exact{t});
		ApplyHorizontalOffsetToRow(current, offset);
	}
	// Bottom of source
	const auto bottomBegin = params.ySource + 1;
	const auto bottomCount = params.bottomGainEnvelope.GetExtend().ToInteger();
	for (int i = 0, current = bottomBegin; i < bottomCount && current < rowCount; ++i, ++current) {
		const auto t = current - params.ySource;
		const auto offset = params.xOffsetAtUnitGain * params.bottomGainEnvelope.GetValueAt(Exact{t});
		ApplyHorizontalOffsetToRow(current, offset);
	}
}

void m2::ApplyVerticalOffsetToColumnPoints(std::vector<VecE>& points, const ApplyVerticalOffsetToColumnPointsParams& params) {
	const auto offsetAtSource = params.yOffsetAtUnitGain * (params.leftGainEnvelope.GetValueAt({}) + params.rightGainEnvelope.GetValueAt({})) / Exact{2};
	for (auto& point : points) {
		const auto distanceToSource = (params.xSource - point.GetX()).AbsoluteValue();
		const auto gainAtPoint = point.GetX() < params.xSource
			? params.leftGainEnvelope.GetValueAt(distanceToSource)
			: params.xSource < point.GetX()
				? params.rightGainEnvelope.GetValueAt(distanceToSource)
				: offsetAtSource;
		const auto yOffsetAtPoint = params.yOffsetAtUnitGain * gainAtPoint;
		point = VecE{point.GetX(), point.GetY() + yOffsetAtPoint};
	}
}

void m2::ApplyHorizontalOffsetToRowPoints(std::vector<VecE>& points, const ApplyHorizontalOffsetToRowPointsParams& params) {
	const auto offsetAtSource = params.xOffsetAtUnitGain * (params.topGainEnvelope.GetValueAt({}) + params.bottomGainEnvelope.GetValueAt({})) / Exact{2};
	for (auto& point : points) {
		const auto distanceToSource = (params.ySource - point.GetY()).AbsoluteValue();
		const auto gainAtPoint = point.GetY() < params.ySource
			? params.topGainEnvelope.GetValueAt(distanceToSource)
			: params.ySource < point.GetY()
				? params.bottomGainEnvelope.GetValueAt(distanceToSource)
				: offsetAtSource;
		const auto xOffsetAtPoint = params.xOffsetAtUnitGain * gainAtPoint;
		point = VecE{point.GetX() + xOffsetAtPoint, point.GetY()};
	}
}
