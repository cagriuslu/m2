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

	/// Generates points inside inclusiveRect starting from {inclusiveRext.x, inclusiveRect.y}. Also returns the stride
	/// of the array.
	std::pair<std::vector<VecE>, int> GeneratePointField(const RectE& inclusiveRect, Exact step);
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
	void ApplyPointSourceOffset(std::vector<VecE>& points, VecE source, VecE offsetAtUnitGain, const DecayEnvelope& gainEnvelope);
	/// Applies attraction or repulsion at a point source with a gain decay. If attraction is negative, repulsion is applied.
	void ApplyPointSourceAttraction(std::vector<VecE>& points, VecE source, Exact attractionAtUnitGain, const DecayEnvelope& gainEnvelope);
	/// Applies vertical offset at a column of points with gain decay to nearby points. Beware: this function is not
	/// aware of the positions of the points and may cause overlapping.
	void ApplyVerticalOffsetToColumnIndex(std::vector<VecE>& points, int stride, int xSource, Exact yOffsetAtUnitGain, const DecayEnvelope& leftGainEnvelope, const DecayEnvelope& rightGainEnvelope);
	/// Applies horizontal offset at a row of points with gain decay to nearby points. Beware: this function is not
	/// aware of the positions of the points and may cause overlapping
	void ApplyHorizontalOffsetToRowIndex(std::vector<VecE>& points, int stride, int ySource, Exact xOffsetAtUnitGain, const DecayEnvelope& topGainEnvelope, const DecayEnvelope& bottomGainEnvelope);
	/// Applies vertical offset at points that lay near a virtual column with gain decay to nearby area
	void ApplyVerticalOffsetToColumnPoints(std::vector<VecE>& points, Exact xSource, Exact yOffsetAtUnitGain, const DecayEnvelope& leftGainEnvelope, const DecayEnvelope& rightGainEnvelope);
	/// Applies horizontal offset at points that lay near a virtual row with gain decay to nearby area
	void ApplyHorizontalOffsetToColumnPoints(std::vector<VecE>& points, Exact ySource, Exact xOffsetAtUnitGain, const DecayEnvelope& topGainEnvelope, const DecayEnvelope& bottomGainEnvelope);

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
