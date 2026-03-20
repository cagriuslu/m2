#pragma once
#include "m2/math/RectE.h"
#include "m2/math/VecF.h"
#include "m2/math/VecE.h"
#include "rng/Distribution.h"
#include "rng/Rng.h"

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
	};

	/// Applies offset at a point source with a gain decay
	void ApplyLinearDecayPointSourceOffset(std::vector<VecE>& points, VecE source, VecE offsetAtUnitGain, const DecayEnvelope& gainEnvelope);
	/// Applies attraction or repulsion at a point source with a gain decay. If attraction is negative, repulsion is applied.
	void ApplyLinearDecayPointSourceAttraction(std::vector<VecE>& points, VecE source, Exact attractionAtUnitGain, const DecayEnvelope& gainEnvelope);

	// offset a line in the direction of the line -> offset'i bir cok noktaya uygulamak yeterli olabilri, linear bir eksende
}
