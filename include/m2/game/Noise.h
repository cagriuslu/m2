#pragma once
#include "m2/math/RectE.h"
#include "m2/math/VecF.h"
#include "m2/math/VecE.h"
#include "rng/Distribution.h"
#include "rng/Rng.h"
#include <m2/math/VecI.h>

namespace m2 {
	// TODO this implementation has a problem
	float perlin(const VecF& point, float depth); // [0, 1]

	/// Following functions receive their parameters as a struct because these functions are usually called with
	/// randomly generated numbers. This helps deterministic games as the order of evaluation of struct members is well
	/// defined (thus the random numbers will be generated in the same order across platforms) whereas the order of
	/// evaluation of function parameters isn't.

	/// Generates points inside inclusiveRect starting from {inclusiveRext.x, inclusiveRect.y}. Also returns the stride
	/// of the array.
	struct GeneratePointFieldParams {
		const RectE& inclusiveRect;
		Exact step;
	};
	std::pair<std::vector<VecE>, int> GeneratePointField(const GeneratePointFieldParams& params);

	/// Applies offset to every point in the given array
	void ApplyNoiseToAllPoints(std::vector<VecE>& points, Distribution& offsetDistribution, Rng&);

	struct DecayEnvelope {
		/// Duration during which the amplitude increases from zero to peak amplitude
		Exact attackDuration;
		/// Amplitude at the end of attack
		Exact peakAmplitude;
		/// Duration during which the amplitude changes from peak amplitude to sustain amplitude
		Exact decayDuration;
		/// Amplitude at the end of decay
		Exact sustainAmplitude;
		/// Duration during which the amplitude stays as sustain amplitude
		Exact sustainDuration;
		/// Duration during which the amplitude drops from sustain to zero
		Exact releaseDuration;

		Exact GetValueAt(Exact t) const;
		Exact GetExtend() const;
	};

	/// Applies offset at a point source with a gain decay
	struct ApplyPointSourceOffsetParams {
		VecE source;
		VecE offsetAtUnitGain;
		DecayEnvelope gainEnvelope;
	};
	void ApplyPointSourceOffset(std::vector<VecE>& points, const ApplyPointSourceOffsetParams& params);

	/// Applies attraction or repulsion at a point source with a gain decay. If attraction is negative, repulsion is applied.
	struct ApplyPointSourceAttractionParams {
		VecE source;
		Exact attractionAtUnitGain;
		DecayEnvelope gainEnvelope;
	};
	void ApplyPointSourceAttraction(std::vector<VecE>& points, const ApplyPointSourceAttractionParams& params);

	/// Applies vertical offset at a column of points with gain decay to nearby points. Beware: this function is not
	/// aware of the positions of the points and may cause overlapping.
	struct ApplyVerticalOffsetToColumnIndexParams {
		int xSource;
		Exact yOffsetAtUnitGain;
		DecayEnvelope leftGainEnvelope;
		DecayEnvelope rightGainEnvelope;
	};
	void ApplyVerticalOffsetToColumnIndex(std::vector<VecE>& points, int stride, const ApplyVerticalOffsetToColumnIndexParams& params);

	/// Applies horizontal offset at a row of points with gain decay to nearby points. Beware: this function is not
	/// aware of the positions of the points and may cause overlapping
	struct ApplyHorizontalOffsetToRowIndexParams {
		int ySource;
		Exact xOffsetAtUnitGain;
		DecayEnvelope topGainEnvelope;
		DecayEnvelope bottomGainEnvelope;
	};
	void ApplyHorizontalOffsetToRowIndex(std::vector<VecE>& points, int stride, const ApplyHorizontalOffsetToRowIndexParams& params);

	/// Applies vertical offset at points that lay near a virtual column with gain decay to nearby area
	struct ApplyVerticalOffsetToColumnPointsParams {
		Exact xSource;
		Exact yOffsetAtUnitGain;
		DecayEnvelope leftGainEnvelope;
		DecayEnvelope rightGainEnvelope;
	};
	void ApplyVerticalOffsetToColumnPoints(std::vector<VecE>& points, const ApplyVerticalOffsetToColumnPointsParams& params);

	/// Applies horizontal offset at points that lay near a virtual row with gain decay to nearby area
	struct ApplyHorizontalOffsetToRowPointsParams {
		Exact ySource;
		Exact xOffsetAtUnitGain;
		DecayEnvelope topGainEnvelope;
		DecayEnvelope bottomGainEnvelope;
	};
	void ApplyHorizontalOffsetToRowPoints(std::vector<VecE>& points, const ApplyHorizontalOffsetToRowPointsParams& params);

	// Helpers

	template <typename T, typename Op>
	void ForEachPointInColumn(std::vector<T>& array, const int stride, const int columnIndex, Op op) {
		const auto linearSize = I(array.size());
		for (int i = columnIndex, y = 0; i < linearSize; i += stride, ++y) {
			op(VecI{columnIndex, y}, array[i]);
		}
	}
	template <typename T, typename Op>
	void ForEachPointInRow(std::vector<T>& array, const int stride, const int rowIndex, Op op) {
		for (int i = rowIndex * stride, x = 0; i < (rowIndex + 1) * stride; ++i, ++x) {
			op(VecI{x, rowIndex}, array[i]);
		}
	}
}
