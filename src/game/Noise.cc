#include "m2/game/Noise.h"

#include <cmath>

#include "m2/math/VecI.h"

using namespace m2;

namespace internal::perlin {
	// Permutation table
	unsigned p[] = {
			151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
			140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
			247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
			57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
			74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
			60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
			65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,
			200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,
			52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,
			207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,
			119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,
			129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
			218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,
			81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,
			184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,
			222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
			// Same table twice
			151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
			140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
			247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
			57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
			74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
			60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
			65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,
			200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,
			52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,
			207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,
			119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,
			129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
			218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,
			81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,
			184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,
			222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
	};

	float grad(unsigned hash, float x, float y, float z) {
		// Calculate dot product with one of these vectors:
		switch(hash & 0xF) {
			case 0x0: return  x + y;
			case 0x1: return -x + y;
			case 0x2: return  x - y;
			case 0x3: return -x - y;
			case 0x4: return  x + z;
			case 0x5: return -x + z;
			case 0x6: return  x - z;
			case 0x7: return -x - z;
			case 0x8: return  y + z;
			case 0x9: return -y + z;
			case 0xA: return  y - z;
			case 0xB: return -y - z;
			case 0xC: return  y + x;
			case 0xD: return -y + z;
			case 0xE: return  y - x;
			case 0xF: return -y - z;
			default: return 0; // never happens
		}
	}

	float fade(float t) {
		// Fade function as defined by Ken Perlin: 6t^5 - 15t^4 + 10t^3
		return t * t * t * (t * (t * 6 - 15) + 10);
	}
}

float m2::perlin(const VecF& point, float depth) {
	using namespace internal::perlin;

	auto point_i = VecI{point};
	auto depth_i = static_cast<int>(depth);

	// Find the unit cube the point lies in
	unsigned x_i = point_i.x & 0xFF;
	unsigned y_i = point_i.y & 0xFF;
	unsigned z_i = depth_i & 0xFF;
	// Calculate location of the point inside unit cube
	float x_f = point.GetX() - static_cast<float>(point_i.x);
	float y_f = point.GetY() - static_cast<float>(point_i.y);
	float z_f = depth - static_cast<float>(depth_i);
	// Find smoothing factors of the point
	float u = fade(x_f);
	float v = fade(y_f);
	float w = fade(z_f);

	// Find the random vectors of each gradient point
	unsigned aaa, aba, aab, abb, baa, bba, bab, bbb;
	aaa = p[p[p[x_i  ]+   y_i]+   z_i];
	aba = p[p[p[x_i  ]+ y_i+1]+   z_i];
	aab = p[p[p[x_i  ]+   y_i]+ z_i+1];
	abb = p[p[p[x_i  ]+ y_i+1]+ z_i+1];
	baa = p[p[p[x_i+1]+   y_i]+   z_i];
	bba = p[p[p[x_i+1]+ y_i+1]+   z_i];
	bab = p[p[p[x_i+1]+   y_i]+ z_i+1];
	bbb = p[p[p[x_i+1]+ y_i+1]+ z_i+1];

	// Calculate dot product against 8 corners
	auto dot_aaa = grad(aaa, x_f, y_f, z_f);
	auto dot_aba = grad(aba, x_f, y_f - 1.0f, z_f);
	auto dot_aab = grad(aab, x_f, y_f, z_f - 1.0f);
	auto dot_abb = grad(abb, x_f, y_f - 1.0f, z_f - 1.0f);
	auto dot_baa = grad(baa, x_f - 1.0f, y_f, z_f);
	auto dot_bba = grad(bba, x_f - 1.0f, y_f - 1.0f, z_f);
	auto dot_bab = grad(bab, x_f - 1.0f, y_f, z_f - 1.0f);
	auto dot_bbb = grad(bbb, x_f - 1.0f, y_f - 1.0f, z_f - 1.0f);

	// Interpolate between dot products
	float x1, x2, y1, y2;
	x1 = std::lerp(dot_aaa, dot_baa, u);
	x2 = std::lerp(dot_aba, dot_bba, u);
	y1 = std::lerp(x1, x2, v);

	x1 = std::lerp(dot_aab, dot_bab, u);
	x2 = std::lerp(dot_abb, dot_bbb, u);
	y2 = std::lerp(x1, x2, v);

	return (std::lerp(y1, y2, w) + 1.0f) / 2.0f;
}

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
