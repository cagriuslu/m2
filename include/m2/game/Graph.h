#pragma once
#include <m2/math/primitives/Exact.h>
#include <m2/math/primitives/Float.h>
#include <m2/math/VecE.h>
#include <m2/math/VecF.h>
#include <m2/ProxyTypes.h>
#include <unordered_map>
#include <map>
#include <vector>
#include <functional>
#include <optional>

namespace m2 {
	// A data structure containing nodes and edges, for finding paths between nodes. All edges have an associated cost.
	// For NodeT and EdgeT, integral types should be used, because they are copied around internally.
	template <typename NodeT, typename EdgeT = Void, typename NodeHash = std::hash<NodeT>, typename NodeEqualityComparator = std::equal_to<NodeT>> class Graph {
	public:
		struct Edge {
			NodeT from, to;
			FE cost;
			EdgeT edge{};
		};

	private:
		/// 2D position of nodes, helps with A* path finding.
		std::unordered_map<NodeT, VecFE, NodeHash, NodeEqualityComparator> _nodePositions;
		std::unordered_map<NodeT, std::vector<Edge>, NodeHash, NodeEqualityComparator> _edges;
		FE _tolerance;

	public:
		/// Create empty graph
		explicit Graph(const FE tolerance = FE{0.001f}) : _tolerance(tolerance) {}
		/// Create graph by repeatedly calling a generator
		explicit Graph(const std::function<std::optional<Edge>()>& generator, const FE tolerance = FE{0.001f}) : _tolerance(tolerance) {
			while (true) {
				const auto edge = generator();
				if (not edge) { break; } // Escape
				AddEdge(*edge);
			}
		}

		/// Add a new edge between two nodes in the graph
		void AddEdge(Edge edge) {
			if (edge.from == edge.to) { throw M2_ERROR("Source and destination nodes are the same"); }
			if (edge.cost.IsLess(FE::Zero(), _tolerance)) { throw M2_ERROR("Negative edge cost"); }
			// Check if node already exists
			if (const auto src_node_it = _edges.find(edge.from); src_node_it != _edges.end()) {
				// Check if edge already exists
				const auto dst_node_it = std::ranges::find_if(src_node_it->second, [dst_node = edge.to](const auto& e) {
					return e.to == dst_node;
				});
				if (dst_node_it != src_node_it->second.end()) { throw M2_ERROR("Edge already exists"); }
				src_node_it->second.emplace_back(edge); // Add to edges
			} else {
				_edges[edge.from] = std::vector{edge}; // Insert to map
			}
		}
		/// This operation is optional because the position information is not used for every functionality.
		void SetNodePosition(NodeT node, const VecFE& position) {
			_nodePositions[node] = position;
		}

		/// Returns the nodes that can be reached from the `source` while spending at most `maxCost`.
		/// If there are multiple paths, the shortest one's cost is returned.
		using ReachableNodesAndCosts = std::unordered_map<NodeT, FE, NodeHash, NodeEqualityComparator>;
		[[nodiscard]] ReachableNodesAndCosts FindNodesReachableFrom(NodeT source, FE maxCost) const {
			// TODO add cache
			// Check if there are any edges from the source
			const auto source_it = _edges.find(source);
			if (source_it == _edges.end()) { return {{source, FE::Zero()}}; } // Only return the source city

			// Add the first set of edges from the source into the nodes_to_visit list
			std::deque<std::pair<NodeT, FE>> nodes_to_visit;
			for (const auto& edge : source_it->second) {
				if (edge.cost.IsLessOrEqual(maxCost, _tolerance)) {
					nodes_to_visit.emplace_back(edge.to, edge.cost);
				}
			}

			// Visit each node in nodes_to_visit, accumulate all reachable nodes into reachable_nodes map
			ReachableNodesAndCosts reachable_nodes;
			reachable_nodes[source] = FE::Zero(); // Add the source node as reachable with no cost
			while (not nodes_to_visit.empty()) {
				// Visit node
				auto visit = nodes_to_visit.front();
				nodes_to_visit.pop_front();

				// Check if the node can be reached
				FE lowest_cost = FE::Max();
				if (auto was_already_reachable = reachable_nodes.find(visit.first);
					was_already_reachable != reachable_nodes.end()) {
					// If the node was already reachable, pick the least-cost path
					lowest_cost = std::min(was_already_reachable->second, visit.second);
					// If the cost has changed, revisit the node later with the new cost
					if (lowest_cost != was_already_reachable->second) {
						nodes_to_visit.emplace_back(visit.first, lowest_cost);
					}
					was_already_reachable->second = lowest_cost;
				} else {
					// Otherwise, add node to reachable nodes
					lowest_cost = visit.second;
					reachable_nodes[visit.first] = lowest_cost;
				}

				// Add the next edges to nodes_to_visit
				if (auto next_step_it = _edges.find(visit.first); next_step_it != _edges.end()) {
					for (const auto& edge : next_step_it->second) {
						if (edge.to != source) {
							if (auto already_was_gonna_visit = std::ranges::find_if(nodes_to_visit, IsFirstEquals<NodeT, FE>(edge.to));
								already_was_gonna_visit != nodes_to_visit.end()) {
								// If the node was already going to be visited, update its cost
								already_was_gonna_visit->second = std::min(already_was_gonna_visit->second, lowest_cost + edge.cost);
							} else if ((lowest_cost + edge.cost).IsLessOrEqual(maxCost, _tolerance)) {
								// Check if this next node was already reachable with a lower cost of reaching
								if (auto edge_node_it = reachable_nodes.find(edge.to); edge_node_it != reachable_nodes.end()) {
									if (lowest_cost + edge.cost < edge_node_it->second) {
										nodes_to_visit.emplace_back(edge.to, lowest_cost + edge.cost);
									}
								} else {
									// Otherwise, add node to nodes_to_visit
									nodes_to_visit.emplace_back(edge.to, lowest_cost + edge.cost);
								}
							}
						}
					}
				}
			}
			return reachable_nodes;
		}

		/// Returns the reversed path between two nodes.
		/// Returns empty vector if path not found, or `from` is equal to `to`.
		using ReversePath = std::vector<Edge>;
		ReversePath FindPathTo(NodeT from, NodeT to) const {
			if (from == to) { return {}; }

			const auto destinationPositionIt = _nodePositions.find(to);
			if (destinationPositionIt == _nodePositions.end()) { throw M2_ERROR("Destination node has no position"); }
			const auto destinationPosition = destinationPositionIt->second;

			// Holds the nodes which will be explored next. Key is the exploration priority, value is the Node to explore.
			std::multimap<FE, NodeT> frontiers{{FE::Zero(), from}};
			// The key Node should be approached via the value Edge.
			std::unordered_map<NodeT, Edge, NodeHash, NodeEqualityComparator> approachVia;
			// Holds accumulated cost of reaching a node. Key is the node, value is its accumulated cost.
			std::unordered_map<NodeT, FE, NodeHash, NodeEqualityComparator> provisionalCost{{from, FE::Zero()}};

			// While there are frontiers to explore
			while (not frontiers.empty()) {
				auto frontier = frontiers.begin()->second;
				// If next location to process is the destination, a path is found. Stop.
				if (frontier == to) { break; }

				// Check if the frontier has outgoing edges
				if (const auto edgesIt = _edges.find(frontier); edgesIt != _edges.end()) {
					// Iterate over edges from the frontier
					for (const auto& edge : edgesIt->second) {
						auto neighbor = edge.to;
						// Previous accumulative cost of travelling to the neighbor
						const auto prevCostIt = provisionalCost.find(neighbor);
						const auto prevCost = (prevCostIt != provisionalCost.end()) ? prevCostIt->second : FE::Max();
						// Accumulative cost of travelling to the neighbor from **current frontier**
						auto newCost = provisionalCost[frontier] + edge.cost;

						// If new path to neighbor is cheaper than the old path
						if (newCost < prevCost) {
							// Save new cost
							provisionalCost[neighbor] = newCost;
							// Calculate priority of neighbor with heuristic parameter
							const auto neighborPositionIt = _nodePositions.find(neighbor);
							if (neighborPositionIt == _nodePositions.end()) { throw M2_ERROR("Neighbor node has no position"); }
							const auto neighborPosition = neighborPositionIt->second;
							auto neighborPriority = newCost * newCost + neighborPosition.GetDistanceToSquaredFE(destinationPosition);
							// Insert neighbor into frontiers
							frontiers.insert({neighborPriority, neighbor});
							// Set the previous position of neighbor as the current position
							approachVia[neighbor] = edge;
						}
					}
				}
				// Remove the current frontier as we have processed it
				frontiers.erase(frontiers.begin());
			}

			auto it = approachVia.find(to); // Check if there is a path
			if (it == approachVia.end()) { return {}; } // Path not found
			ReversePath path; // Built reverse list of edges
			while (it != approachVia.end()) {
				path.emplace_back(it->second);
				if (it->second.from == from) {
					return path;
				}
				it = approachVia.find(it->second.from);
			}
			throw M2_ERROR("Implementation error, approach map doesn't contain the source");
		}

		// Utilities

		static std::multimap<FE, NodeT> OrderByBestCost(const ReachableNodesAndCosts& nodes) {
			std::multimap<FE, NodeT> ordered_map;
			for (const auto& [node, cost] : nodes) {
				ordered_map.emplace(cost, node);
			}
			return ordered_map;
		}
		/// If two sources are deemed equal, the reachable_nodes maps of those sources can be merged.
		/// If a node appears in both maps, the one with the lowest cost is used.
		static ReachableNodesAndCosts merge_reachable_nodes(const ReachableNodesAndCosts& nodes_1, const ReachableNodesAndCosts& nodes_2) {
			auto copy_of_nodes_1 = nodes_1;
			for (const auto& [node, cost] : nodes_2) {
				if (auto it = copy_of_nodes_1.find(node); it == copy_of_nodes_1.end() || cost < it->second) {
					// If the node doesn't exist in the copy, or it exists but the cost is higher,
					// insert the node from second map.
					copy_of_nodes_1[node] = cost;
				}
			}
			return copy_of_nodes_1;
		}
	};
}
