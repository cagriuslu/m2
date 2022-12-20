#include "m2/Pathfinder.hh"
#include "m2/Component.h"
#include "m2/Game.hh"
#include <m2/box2d/RayCast.h>
#include <m2/Vec2i.h>
#include <m2/box2d/Utils.h>
#include <m2/M2.h>
#include <cfloat>
#include <unordered_map>
#include <map>
#include <list>

m2::Pathfinder::Pathfinder(const pb::Level &lb) {
	for (int y = 0; y < lb.background_rows_size(); ++y) {
		for (int x = 0; x < lb.background_rows(y).items_size(); ++x) {
			auto sprite_type = lb.background_rows(y).items(x);
			if (sprite_type && GAME.sprites[sprite_type].sprite().has_background_collider()) {
				_blocked_locations.emplace(x, y);
			}
		}
	}
}

std::vector<m2::Vec2i> m2::Pathfinder::find_smooth_path(const Vec2f& from_f, const Vec2f& to_f, float max_distance_m) {
	auto from = m2::Vec2i{from_f};
	auto to = m2::Vec2i{to_f};
	if (from == to) {
		return {};
	}

	auto distance_sq = from_f.distance_sq(to_f);
	auto max_distance_sq = max_distance_m * max_distance_m;
	if (max_distance_sq < distance_sq) {
		return {};
	}

	// Check if there is direct eyesight
	if (check_eyesight(from, to)) {
		return {to, from};
	}

	auto grid_path = find_grid_path(from, to, max_distance_m);
	if (not grid_path.empty()) {
		return smoothen_path(grid_path, max_distance_m);
	}

	return {};
}

std::vector<m2::Vec2i> m2::Pathfinder::find_grid_path(const Vec2i& from, const Vec2i& to, float max_distance_m) {
	auto max_grid_distance_m = max_distance_m * SQROOT_2;

	// Holds the positions which will be explored next. Key is the priority, value is the position.
	std::multimap<float, Vec2i> frontiers{{0.0f, from}};

	// Holds from which position should you approach a certain position. Key should be approached from value.
	std::unordered_map<Vec2i, Vec2i, Vec2iHash> approach_from{{from, from}};

	// Holds accumulated cost of reaching a position. Key is the position, value is its cost.
	std::unordered_map<Vec2i, float, Vec2iHash> provisional_cost{{from, 0.0f}};

	while (not frontiers.empty()) {
		auto current_frontier_it = frontiers.begin();
		auto frontier = current_frontier_it->second;

		// If next location to process is the destination, a path is found. Stop.
		if (frontier == to) {
			break;
		}

		// Gather neighbors of frontierItem
		m2::Vec2i reachable_neighbors[4];
		uint32_t reachable_neighbor_count = 0;
		for (const auto& direction : {Vec2i{0, +1}, Vec2i{+1, 0}, Vec2i{0, -1}, Vec2i{-1, 0}}) {
			auto neighbor = frontier + direction;

			bool is_reachable = false;
			if (frontier == from) {
				is_reachable = _blocked_locations.contains(from) ? check_eyesight(frontier, neighbor) : not _blocked_locations.contains(neighbor);
			} else if (not _blocked_locations.contains(neighbor)) {
				// If frontier is on a blocked location, neighbor may lay outside the playable area
				// Check if there is eyesight from frontier to neighbor
				is_reachable = _blocked_locations.contains(frontier) ? check_eyesight(frontier, neighbor) : true;
			} else if (neighbor == to) {
				is_reachable = check_eyesight(frontier, neighbor);
			}

			if (is_reachable) {
				reachable_neighbors[reachable_neighbor_count++] = neighbor;
			}
		}

		// Iterate over neighbors
		for (uint32_t i = 0; i < reachable_neighbor_count; ++i) {
			const auto& neighbor = reachable_neighbors[i];

			// Calculate the cost of traveling to neighbor from current location
			auto new_cost = provisional_cost[frontier] + 1.0f;
			// Find the previous cost of traveling to neighbor
			auto it = provisional_cost.find(neighbor);
			auto old_cost = (it != provisional_cost.end()) ? it->second : FLT_MAX;

			// If new path to neighbor is cheaper than the old AND cheaper than max_grid_distance
			if (new_cost < old_cost && new_cost < max_grid_distance_m) {
				// Save new cost
				provisional_cost[neighbor] = new_cost;
				// Calculate priority of neighbor with heuristic parameter (which is Manhattan distance to `to`)
				auto neighbor_priority = new_cost + (float)neighbor.manhattan_distance(to);
				// Insert into frontiers
				frontiers.insert({neighbor_priority, neighbor});
				// Set the previous position of neighbor as the current position
				approach_from[neighbor] = frontier;
			}
		}

		// Remove the current frontier as we have processed it
		frontiers.erase(current_frontier_it);
	}

	// Check if there is a path
	auto it = approach_from.find(to);
	if (it == approach_from.end()) {
		return {};
	} else {
		std::vector<m2::Vec2i> path{to};
		// Built reverse list of positions
		while (it != approach_from.end() && from != it->second) {
			path.emplace_back(it->second);
			it = approach_from.find(it->second);
		}
		path.emplace_back(from);
		return path;
	}
}

std::vector<m2::Vec2i> m2::Pathfinder::smoothen_path(const std::vector<Vec2i>& reverse_path, float max_distance_m) {
	if (reverse_path.size() < 2) {
		throw M2ERROR("Path contains less than two points");
	}

	std::vector<Vec2i> smooth_path{reverse_path.front()}; // insert `to`
	const auto* point1 = &reverse_path[0];
	const auto* prev_point2 = point1;

	float cost = 0.0f;
	auto insert_point = [&](const Vec2i* point) {
		if (point) {
			auto cost_to_point = smooth_path.back().distance(*point);
			if (max_distance_m < cost + cost_to_point) {
				return false;
			}
			cost += cost_to_point;

			smooth_path.push_back(*point);
		}
		return true;
	};

	for (auto point2_it = reverse_path.begin() + 1; point2_it != reverse_path.end(); ++point2_it) {
		auto* point2 = &(*point2_it);

		bool eyesight = m2::box2d::check_eyesight(*GAME.world, m2::Vec2f{*point1}, m2::Vec2f{*point2}, m2::box2d::FIXTURE_CATEGORY_OBSTACLE);
		if (point2_it == std::prev(reverse_path.end(), 1)) {
			if (not eyesight) {
				// If we are processing the last point AND there is no eyesight, add the previous point
				if (not insert_point(prev_point2)) {
					return {};
				}
			}
			// Add the last point
			if (not insert_point(point2)) {
				return {};
			}
		} else if (eyesight) {
			// There is an eyesight, continue iterating
		} else {
			// There is no eyesight, add previous point
			if (not insert_point(prev_point2)) {
				return {};
			}
			point1 = prev_point2;
		}
		prev_point2 = point2;
	}
	return smooth_path;
}

bool m2::Pathfinder::check_eyesight(const Vec2i& from, const Vec2i& to) {
	return box2d::check_eyesight(*GAME.world, Vec2f{from} + Vec2f{0.5f, 0.5f}, Vec2f{to} + Vec2f{0.5f, 0.5f}, box2d::FIXTURE_CATEGORY_OBSTACLE);
}

#define ManhattanDistance(a, b) (abs((a).x - (b).x) + abs((a).y - (b).y))

int PathfinderMap_Init(PathfinderMap* pm) {
	pm->blocked_locations.clear();
    for (auto physique_it : GAME.physics) {
        auto* phy = physique_it.first;
		if (phy->body) {
			// TODO here it is assumed that bodies has one fixture
			b2Fixture* fixture = phy->body->GetFixtureList();
			if (m2::box2d::is_obstacle(fixture)) {
				// TODO here it is assumed that fixtures have one child
				b2AABB aabb = fixture->GetAABB(0);
				// AABB is bigger 0.01 meters than the object at each side
				// Decrease its size by 0.02 so that rounding can work
                b2AABB convervative_aabb = m2::box2d::expand_aabb(aabb, -0.02f);
				int lowerX = (int) roundf(convervative_aabb.lowerBound.x);
				int upperX = (int) roundf(convervative_aabb.upperBound.x);
				int lowerY = (int) roundf(convervative_aabb.lowerBound.y);
				int upperY = (int) roundf(convervative_aabb.upperBound.y);
				for (int y = lowerY; y <= upperY; y++) {
					for (int x = lowerX; x <= upperX; x++) {
						pm->blocked_locations.insert({x, y});
					}
				}
			}
		}
	}
	return 0;
}

void PathfinderMap_Term(PathfinderMap* pm) {
	pm->blocked_locations.clear();
}

m2::Value<std::list<m2::Vec2i>> PathfinderMap_FindPath(PathfinderMap* pm, m2::Vec2f from, m2::Vec2f to) {
	std::list<m2::Vec2i> outReverseListOfVec2Is;
	// Check if there is direct eyesight
	if (m2::box2d::check_eyesight(*GAME.world, from, to, m2::box2d::FIXTURE_CATEGORY_OBSTACLE)) {
		auto fromI = m2::Vec2i{from};
		auto toI = m2::Vec2i{to};

        outReverseListOfVec2Is.clear();
		// Add `to` to list
        outReverseListOfVec2Is.push_back(toI);
		// Add `from` to list, if it is different than `to`
		if (fromI == toI) {
			return m2::failure("Path not find");
		} else {
            outReverseListOfVec2Is.push_back(fromI);
			return outReverseListOfVec2Is;
		}
	} else {
		auto grid_steps = _PathfinderMap_FindGridSteps(pm, from, to);
		if (grid_steps) {
			auto any_angle_grid_steps = _PathfinderMap_GridStepsToAnyAngle(grid_steps.value());
			if (1 < any_angle_grid_steps.size()) {
				return any_angle_grid_steps;
			}
		}
		return m2::failure("Path not found");
	}
}

struct PriorityListItem {
	float priority;
	m2::Vec2i position;
};

m2::Value<std::list<m2::Vec2i>> _PathfinderMap_FindGridSteps(PathfinderMap* pm, m2::Vec2f fromF, m2::Vec2f toF) {
	std::list<m2::Vec2i> outReverseListOfVec2Is;

	auto from = m2::Vec2i{fromF};
	auto to = m2::Vec2i{toF};

	PriorityListItem tmpPrioListItem;

	// Holds the positions where will be explored next
    std::list<PriorityListItem> frontiers;
    frontiers.push_front({0.0f, from});

	// Holds from which position should you approach a certain position
	std::unordered_map<m2::Vec2i, m2::Vec2i, m2::Vec2iHash> approach_from;
	approach_from[from] = from;

	std::unordered_map<m2::Vec2i, float, m2::Vec2iHash> cost_so_far;
	cost_so_far[from] = 0.0f;

	while (not frontiers.empty()) {
        auto curr_frontier_it = frontiers.begin();
        auto* frontierItem = &(*curr_frontier_it);

		// If next location to discover is the destination, stop
		if (frontierItem->position == to) {
			break;
		}

		// Find cost to current location
		const float costToCurrentFrontier = cost_so_far[frontierItem->position];

		// Gather neighbors of frontierItem
		m2::Vec2i neighbors[4];
		float frontierToNeighborCosts[4];
		uint32_t neighborCount = 0;
		m2::Vec2i topNeighbor = frontierItem->position + m2::Vec2i(0, -1);
		if (not pm->blocked_locations.contains(topNeighbor) || (to == topNeighbor)) {
			neighbors[neighborCount] = topNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		m2::Vec2i rightNeighbor = frontierItem->position + m2::Vec2i(+1, 0);
		if (not pm->blocked_locations.contains(rightNeighbor) || (to == rightNeighbor)) {
			neighbors[neighborCount] = rightNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		m2::Vec2i bottomNeighbor = frontierItem->position + m2::Vec2i(0, +1);
		if (not pm->blocked_locations.contains(bottomNeighbor) || (to == bottomNeighbor)) {
			neighbors[neighborCount] = bottomNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		m2::Vec2i leftNeighbor = frontierItem->position + m2::Vec2i(-1, 0);
		if (not pm->blocked_locations.contains(leftNeighbor) || (to == leftNeighbor)) {
			neighbors[neighborCount] = leftNeighbor;
			frontierToNeighborCosts[neighborCount++] = 1.0f;
		}
		// TODO: If (from) is on one of the block locations, and if one of the neighbors end up OUTSIDE of the game area,
		// (because from is on the edge of the map), the search spills outside the game area and never ends.

		// Iterate over neighbors
		for (uint32_t neighIdx = 0; neighIdx < neighborCount; neighIdx++) {
			const auto neighbor = neighbors[neighIdx];
			const float frontierToNeighborCost = frontierToNeighborCosts[neighIdx];

			// Calculate the cost of traveling to neighbor from current location
			float newCostToNeighbor = costToCurrentFrontier + frontierToNeighborCost;
			// Find the previous cost of traveling to neighbor
			auto it = cost_so_far.find(neighbor);
			const float oldCostToNeighbor = (it != cost_so_far.end()) ? it->second : FLT_MAX;

			// If new path to neighbor is cheaper than the old path
			if (newCostToNeighbor < oldCostToNeighbor) {
				// Save new cost
				cost_so_far[neighbor] = newCostToNeighbor;
				// Calculate priority of neighbor with heuristic parameter (which is Manhattan distance to `to`)
				tmpPrioListItem.priority = newCostToNeighbor + ManhattanDistance(to, neighbor);
				tmpPrioListItem.position = neighbor;
				// Insert neighbor into frontiers
				bool inserted = false;
                for (auto frontier_it = frontiers.begin(); frontier_it != frontiers.end(); ++frontier_it) {
                    if (tmpPrioListItem.priority < frontier_it->priority) {
                        frontiers.insert(frontier_it, tmpPrioListItem);
                        inserted = true;
                        break;
                    }
                }
				if (not inserted) {
                    frontiers.push_back(tmpPrioListItem);
				}
				// Set the previous position of neighbor as the current position
				approach_from[neighbor] = frontierItem->position;
			}
		}

		// Remove current position as we're done processing it
        frontiers.erase(curr_frontier_it);
	}

	// Check if there is a path
	auto it = approach_from.find(to);
	if (it == approach_from.end()) {
		return m2::failure("Path not found");
	} else {
        outReverseListOfVec2Is.clear();
		// Add `to` to list
        outReverseListOfVec2Is.push_back(to);
		// Built outReverseListOfVec2Is
		while (it != approach_from.end() && from != it->second) {
			m2::Vec2i data = it->second;
            outReverseListOfVec2Is.push_back(data);
			it = approach_from.find(it->second);
		}
		// Add `from` to list as well
		if (it != approach_from.end()) {
			m2::Vec2i data = it->second;
            outReverseListOfVec2Is.push_back(data);
		}
		return outReverseListOfVec2Is;
	}
}

std::list<m2::Vec2i> _PathfinderMap_GridStepsToAnyAngle(const std::list<m2::Vec2i>& listOfVec2Is) {
	std::list<m2::Vec2i> outListOfVec2Is;
	if (listOfVec2Is.size() < 2) {
		return outListOfVec2Is;
	}

    auto point_1_it = listOfVec2Is.begin();
    outListOfVec2Is.push_back(*point_1_it);
    auto* point1 = &(*point_1_it);

	const m2::Vec2i* prevPoint2 = point1;
    auto point_2_it = listOfVec2Is.begin();
	for (++point_2_it; point_2_it != listOfVec2Is.end(); ++point_2_it) {
        auto* point2 = &(*point_2_it);

		const bool eyeSight = m2::box2d::check_eyesight(*GAME.world, m2::Vec2f{*point1}, m2::Vec2f{*point2},
														m2::box2d::FIXTURE_CATEGORY_OBSTACLE);
        if (point_2_it == std::prev(listOfVec2Is.end(), 1)) {
            if (eyeSight) {
                // If we are processing the last point and there is an eye sight, add the last point
                outListOfVec2Is.push_back(*point2);
            } else {
                // If we are processing the last point and there is no eye sight, add previous and the last point
                if (prevPoint2) {
                    outListOfVec2Is.push_back(*prevPoint2);
                }
                outListOfVec2Is.push_back(*point2);
            }
        } else if (eyeSight) {
			// There is an eye sight, continue iterating
		} else {
			// There is no eye sight, add previous point
			if (prevPoint2) {
                outListOfVec2Is.push_back(*prevPoint2);
			}
			point1 = prevPoint2;
		}
		
		prevPoint2 = point2;
	}
	return outListOfVec2Is;
}
