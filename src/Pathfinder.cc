#include "m2/Pathfinder.h"
#include "m2/Component.h"
#include "m2/Game.h"
#include <m2/box2d/RayCast.h>
#include <m2/VecI.h>
#include <m2/box2d/Detail.h>
#include <m2/M2.h>
#include <cfloat>
#include <unordered_map>
#include <map>
#include <list>

m2::Pathfinder::Pathfinder(const pb::Level &lb) {
	const auto& first_layer = lb.background_layers(0);
	for (int y = 0; y < first_layer.background_rows_size(); ++y) {
		for (int x = 0; x < first_layer.background_rows(y).items_size(); ++x) {
			auto sprite_type = first_layer.background_rows(y).items(x);
			if (sprite_type && GAME.get_sprite(sprite_type).background_collider_type() != box2d::ColliderType::NONE) {
				_blocked_locations.emplace(x, y);
			}
		}
	}
}

m2::Path m2::Pathfinder::find_smooth_path(const VecF& from_f, const VecF& to_f, float max_distance_m) {
	auto from = m2::VecI{from_f};
	auto to = m2::VecI{to_f};
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

m2::Path m2::Pathfinder::find_grid_path(const VecI& from, const VecI& to, float max_distance_m) {
	if (from == to) {
		return {};
	}

	// Check if the target is too far even from bird's eye
	auto distance_sq = from.distance_sq(to);
	auto max_distance_sq = max_distance_m * max_distance_m;
	if (max_distance_sq < distance_sq) {
		return {};
	}

	// Check if there is direct eyesight
	if (check_eyesight(from, to)) {
		return {to, from};
	}

	auto max_grid_distance_m = max_distance_m * SQROOT_2;

	// Holds the positions which will be explored next. Key is the priority, value is the position.
	std::multimap<float, VecI> frontiers{{0.0f, from}};

	auto& approach_map = _approach_from_cache[to];

	// Holds accumulated cost of reaching a position. Key is the position, value is its cost.
	std::unordered_map<VecI, float, VecIHash> provisional_cost{{from, 0.0f}};

	while (not frontiers.empty()) {
		auto current_frontier_it = frontiers.begin();
		auto frontier = current_frontier_it->second;

		// If next location to process is the destination, a path is found. Stop.
		if (frontier == to) {
			break;
		}

		// Iterate over neighbors
		for (const auto& direction : {VecI{0, +1}, VecI{+1, 0}, VecI{0, -1}, VecI{-1, 0}}) {
			auto neighbor = frontier + direction;

			bool is_reachable = false;
			auto cache_it = approach_map.find(neighbor);
			if (cache_it != approach_map.end() && cache_it->second.first == frontier) {
				LOG_TRACE("Pathfinder cache hit", frontier, neighbor);
				is_reachable = true;
			} else if (frontier == from) {
				is_reachable = _blocked_locations.contains(from) ? check_eyesight(frontier, neighbor) : not _blocked_locations.contains(neighbor);
			} else if (not _blocked_locations.contains(neighbor)) {
				// If frontier is on a blocked location, neighbor may lay outside the playable area
				// Check if there is eyesight from frontier to neighbor
				is_reachable = _blocked_locations.contains(frontier) ? check_eyesight(frontier, neighbor) : true;
			} else if (neighbor == to) {
				is_reachable = check_eyesight(frontier, neighbor);
			}
			if (is_reachable) {
				// Calculate the cost of traveling to neighbor from current location
				// Use the cached value if cache was hit
				auto new_cost = (cache_it != approach_map.end()) ? cache_it->second.second + 1.0f : provisional_cost[frontier] + 1.0f;
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
					approach_map[neighbor] = {frontier, new_cost};
				}
			}
		}

		// Remove the current frontier as we have processed it
		frontiers.erase(current_frontier_it);
	}

	// Check if there is a path
	auto it = approach_map.find(to);
	if (it == approach_map.end()) {
		return {};
	} else {
		Path path{to};
		// Built reverse list of positions
		while (it != approach_map.end() && from != it->second.first) {
			path.emplace_back(it->second.first);
			it = approach_map.find(it->second.first);
		}
		path.emplace_back(from);
		return path;
	}
}

m2::Path m2::Pathfinder::smoothen_path(const Path& reverse_path, float max_distance_m) {
	if (reverse_path.size() < 2) {
		throw M2ERROR("Path contains less than two points");
	}

	Path smooth_path{reverse_path.front()}; // insert `to`
	const auto* point1 = &reverse_path[0];
	const auto* prev_point2 = point1;

	float cost = 0.0f;
	auto insert_point = [&](const VecI* point) {
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

		bool eyesight = m2::box2d::check_eyesight(*LEVEL.world, m2::VecF{*point1}, m2::VecF{*point2}, m2::box2d::FIXTURE_CATEGORY_OBSTACLE);
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

bool m2::Pathfinder::check_eyesight(const VecI& from, const VecI& to) {
	return box2d::check_eyesight(*LEVEL.world, VecF{from} + VecF{0.5f, 0.5f}, VecF{to} + VecF{0.5f, 0.5f}, box2d::FIXTURE_CATEGORY_OBSTACLE);
}
