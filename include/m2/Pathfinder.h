#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <m2/Vec2i.h>
#include <m2/Vec2f.h>
#include "Value.h"
#include <Level.pb.h>
#include <unordered_set>
#include <list>

namespace m2 {
	class Pathfinder {
		std::unordered_set<m2::Vec2i, m2::Vec2iHash> _blocked_locations;

		// Holds from which position should you approach another position, while trying to reach a certain destination.
		// First key is the destination, first value is the corresponding approach map.
		// The key of the approach map should be approached from the value.
		using ApproachMap = std::unordered_map<Vec2i, Vec2i, Vec2iHash>;
		std::unordered_map<Vec2i, ApproachMap, Vec2iHash> _approach_from_cache; // TODO cache is not used for look-up, only storage

	public:
		explicit Pathfinder(const pb::Level& level_blueprint);

		/// Returns reverse path [to, to - 1, to - 2, ..., from + 1, from]
		/// Returns empty vector if path not found
		std::vector<Vec2i> find_smooth_path(const Vec2f& from, const Vec2f& to, float max_distance_m);

		/// If the world is not static, the cache should be cleared after the physics step
		inline void clear_cache() { _approach_from_cache.clear(); }

	private:
		std::vector<Vec2i> find_grid_path(const Vec2i& from, const Vec2i& to, float max_distance_m);
		static std::vector<Vec2i> smoothen_path(const std::vector<Vec2i>& reverse_path, float max_distance_m);
		static bool check_eyesight(const Vec2i& from, const Vec2i& to);
	};
}

#endif
