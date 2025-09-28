#include <m2/game/Graph.h>
#include <m2/Error.h>
#include <m2/M2.h>
#include <deque>

m2::Graph::Graph(const std::function<std::optional<std::pair<Node, Edge>>()>& generator, const FE tolerance) : _tolerance(tolerance) {
	while (true) {
		const auto nodeNEdge = generator();
		if (not nodeNEdge) {
			break;
		}
		AddEdge(nodeNEdge->first, nodeNEdge->second);
	}
}

void m2::Graph::AddEdge(const Node from, const Edge edge) {
	if (from == edge.toNode) {
		throw M2_ERROR("Source and destination nodes are the same");
	}
	if (edge.cost.IsLess(FE::Zero(), _tolerance)) {
		throw M2_ERROR("Negative edge cost");
	}
	// Check if the node already exists
	if (const auto src_node_it = _edges.find(from); src_node_it != _edges.end()) {
		// Check if the edge already exists
		const auto dst_node_it = std::ranges::find_if(src_node_it->second, [dst_node = edge.toNode](const auto& e) {
			return e.toNode == dst_node;
		});
		if (dst_node_it != src_node_it->second.end()) {
			throw M2_ERROR("Edge already exists");
		}
		src_node_it->second.emplace_back(edge); // Add to edges
	} else {
		_edges[from] = std::vector{edge}; // Insert to map
	}
}
void m2::Graph::SetNodePosition(const Node node, const VecFE& position) {
	_nodePositions[node] = position;
}

m2::Graph::ReachableNodesAndCosts m2::Graph::FindNodesReachableFrom(Node source, const FE maxCost) const {
	// Check if there are any edges from the source
	const auto source_it = _edges.find(source);
	if (source_it == _edges.end()) {
		// Only return the source city
		return {{source, FE::Zero()}};
	}

	// Add the first set of edges from the source into the nodes_to_visit list
	std::deque<std::pair<Node, FE>> nodes_to_visit;
	for (const auto& edge : source_it->second) {
		if (edge.cost.IsLessOrEqual(maxCost, _tolerance)) {
			nodes_to_visit.emplace_back(edge.toNode, edge.cost);
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
				if (edge.toNode != source) {
					if (auto already_was_gonna_visit = std::ranges::find_if(nodes_to_visit, IsFirstEquals<Node, FE>(edge.toNode));
						already_was_gonna_visit != nodes_to_visit.end()) {
						// If the node was already going to be visited, update its cost
						already_was_gonna_visit->second = std::min(already_was_gonna_visit->second, lowest_cost + edge.cost);
					} else if ((lowest_cost + edge.cost).IsLessOrEqual(maxCost, _tolerance)) {
						// Check if this next node was already reachable with a lower cost of reaching
						if (auto edge_node_it = reachable_nodes.find(edge.toNode); edge_node_it != reachable_nodes.end()) {
							if (lowest_cost + edge.cost < edge_node_it->second) {
								nodes_to_visit.emplace_back(edge.toNode, lowest_cost + edge.cost);
							}
						} else {
							// Otherwise, add node to nodes_to_visit
							nodes_to_visit.emplace_back(edge.toNode, lowest_cost + edge.cost);
						}
					}
				}
			}
		}
	}
	return reachable_nodes;
}

m2::Graph::ReversePath m2::Graph::FindPathTo(Node from, Node to) const {
	if (from == to) {
		return ReversePath{to};
	}

	const auto destinationPositionIt = _nodePositions.find(to);
	if (destinationPositionIt == _nodePositions.end()) {
		throw M2_ERROR("Node has no position: " + ToString(to));
	}
	const auto destinationPosition = destinationPositionIt->second;

	// Holds the nodes which will be explored next. Key is the exploration priority, value is the Node to explore.
	std::multimap<FE, Node> frontiers{{FE::Zero(), from}};

	// The key Node should be approached from the value Node, with the stored cost.
	std::unordered_map<Node, std::pair<Node,FE>> approachFrom;

	// Holds accumulated cost of reaching a node. Key is the node, value is its accumulated cost.
	std::unordered_map<Node, FE> provisionalCost{{from, FE::Zero()}};

	// While there are frontiers to explore
	while (not frontiers.empty()) {
		auto frontier = frontiers.begin()->second;

		// If next location to process is the destination, a path is found. Stop.
		if (frontier == to) {
			break;
		}

		// Check if the frontier has outgoing edges
		if (const auto edgesIt = _edges.find(frontier); edgesIt != _edges.end()) {
			// Iterate over edges from the frontier
			for (const auto& edge : edgesIt->second) {
				auto neighbor = edge.toNode;

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
					if (neighborPositionIt == _nodePositions.end()) {
						throw M2_ERROR("Node has no position: " + ToString(neighbor));
					}
					const auto neighborPosition = neighborPositionIt->second;
					auto neighborPriority = newCost * newCost + neighborPosition.GetDistanceToSquaredFE(destinationPosition);
					// Insert neighbor into frontiers
					frontiers.insert({neighborPriority, neighbor});
					// Set the previous position of neighbor as the current position
					approachFrom[neighbor] = {frontier, newCost};
				}
			}
		}

		// Remove the current frontier as we have processed it
		frontiers.erase(frontiers.begin());
	}

	// Check if there is a path
	auto it = approachFrom.find(to);
	if (it == approachFrom.end()) {
		return {}; // Path not found
	}
	ReversePath path{to};
	// Built reverse list of positions
	while (it != approachFrom.end() && from != it->second.first) {
		path.emplace_back(it->second.first);
		it = approachFrom.find(it->second.first);
	}
	path.emplace_back(from);
	return path;
}

std::multimap<m2::FE, m2::Graph::Node> m2::Graph::order_by_cost(const ReachableNodesAndCosts& nodes) {
	std::multimap<FE, Node> ordered_map;
	for (const auto& [node, cost] : nodes) {
		ordered_map.emplace(cost, node);
	}
	return ordered_map;
}

m2::Graph::ReachableNodesAndCosts m2::Graph::merge_reachable_nodes(const ReachableNodesAndCosts& nodes_1,
		const ReachableNodesAndCosts& nodes_2) {
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
