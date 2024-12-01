#pragma once
#include <Level.pb.h>

#include <list>
#include <unordered_set>

#include "m2/Meta.h"
#include "m2/math/VecF.h"
#include "m2/math/VecI.h"

namespace m2 {
	using Path = std::vector<VecI>;

	class Pathfinder {
		std::unordered_set<m2::VecI, m2::VecIHash> _blocked_locations;

		// Holds from which position should you approach another position, while trying to reach a certain destination.
		// First key is the destination, first value is the corresponding approach map.
		// The key of the approach map should be approached from the value, with the stored cost.
		using ApproachMap = std::unordered_map<VecI, std::pair<VecI,float>, VecIHash>;
		std::unordered_map<VecI, ApproachMap, VecIHash> _approach_from_cache;

	public:
		explicit Pathfinder(const pb::Level& level_blueprint);

		/// Returns reverse path [to, to - 1, to - 2, ..., from + 1, from]
		/// Returns empty vector if path not found
		Path find_grid_path(const VecI& from, const VecI& to, float max_distance_m);
		inline Path find_grid_path(const VecF& from, const VecF& to, float max_distance_m) { return find_grid_path(VecI{from}, VecI{to}, max_distance_m); }

		/// Returns reverse path [to, to - 1, to - 2, ..., from + 1, from]
		/// Returns empty vector if path not found
		Path find_smooth_path(const VecF& from, const VecF& to, float max_distance_m);

		/// If the world is not static, the cache should be cleared after the physics step
		inline void clear_cache() { _approach_from_cache.clear(); }

		static void draw_path(const Path& path, SDL_Color color);

	private:
		static Path smoothen_path(const Path& reverse_path, float max_distance_m);
		static bool check_eyesight(const VecI& from, const VecI& to);
	};
}
