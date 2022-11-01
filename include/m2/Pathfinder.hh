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

	public:
		explicit Pathfinder(const pb::Level& level_blueprint);

		// Returns reverse path [to, to - 1, to - 2, ..., from + 1, from]
		// Returns empty vector if path not found
		std::vector<Vec2i> find_smooth_path(const Vec2f& from, const Vec2f& to, float max_distance_m);

	private:
		std::vector<Vec2i> find_grid_path(const Vec2i& from, const Vec2i& to, float max_distance_m);
		static std::vector<Vec2i> smoothen_path(const std::vector<Vec2i>& reverse_path, float max_distance_m);
		static bool check_eyesight(const Vec2i& from, const Vec2i& to);
	};
}

typedef struct {
	std::unordered_set<m2::Vec2i, m2::Vec2iHash> blocked_locations;
} PathfinderMap;

int PathfinderMap_Init(PathfinderMap* pm);
void PathfinderMap_Term(PathfinderMap* pm);

/// Returns XOK if outReverseListOfVec2Is has more than one points
/// Otherwise, returns XERR_PATH_NOT_FOUND.

/// Returns reverse list of Vec2i's
m2::Value<std::list<m2::Vec2i>> PathfinderMap_FindPath(PathfinderMap* pm, m2::Vec2f from, m2::Vec2f to);

/// Returns reverse list of Vec2i's
m2::Value<std::list<m2::Vec2i>> _PathfinderMap_FindGridSteps(PathfinderMap* pm, m2::Vec2f from, m2::Vec2f to);

std::list<m2::Vec2i> _PathfinderMap_GridStepsToAnyAngle(const std::list<m2::Vec2i>& listOfVec2Is);

#endif
