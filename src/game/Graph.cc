#include <m2/game/Graph.h>
#include <m2/Exception.h>
#include <m2/M2.h>
#include <deque>

m2::Graph::Graph(const std::function<std::optional<std::pair<Node, Edge>>()>& generator, float tolerance) : _tolerance(tolerance) {
	while (true) {
		auto optional_node_edge = generator();
		if (not optional_node_edge) {
			break;
		}
		add_edge(optional_node_edge->first, optional_node_edge->second);
	}
}

void m2::Graph::add_edge(Node from, Edge edge) {
	if (from == edge.node) {
		throw M2ERROR("Source and destination nodes are the same");
	}
	if (is_less(edge.cost, 0.0f, _tolerance)) {
		throw M2ERROR("Negative edge cost");
	}
	// Check if the node already exists
	auto src_node_it = _edges.find(from);
	if (src_node_it != _edges.end()) {
		// Check if the edge already exists
		auto dst_node_it = std::find_if(src_node_it->second.begin(), src_node_it->second.end(), [dst_node = edge.node](const auto& e) {
			return e.node == dst_node;
		});
		if (dst_node_it != src_node_it->second.end()) {
			throw M2ERROR("Edge already exists");
		}
		src_node_it->second.emplace_back(edge); // Add to edges
	} else {
		_edges[from] = std::vector<Edge>{edge}; // Insert to map
	}
}

std::unordered_map<m2::Graph::Node, float> m2::Graph::reachable_nodes_from(Node source, float inclusive_cost) const {
	// Check if there are any edges from the source
	auto source_it = _edges.find(source);
	if (source_it == _edges.end()) {
		return {};
	}

	// Add the first set of edges from the source into the nodes_to_visit list
	std::deque<std::pair<Node, float>> nodes_to_visit;
	for (const auto& edge : source_it->second) {
		if (is_less_or_equal(edge.cost, inclusive_cost, _tolerance)) {
			nodes_to_visit.emplace_back(edge.node, edge.cost);
		}
	}

	// Visit each node in nodes_to_visit, accumulate all reachable nodes into reachable_nodes map
	std::unordered_map<Node, float> reachable_nodes;
	while (not nodes_to_visit.empty()) {
		// Visit node
		auto visit = nodes_to_visit.front();
		nodes_to_visit.pop_front();

		// Check if the node can be reached
		float lowest_cost = INFINITY;
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
		auto next_step_it = _edges.find(visit.first);
		if (next_step_it != _edges.end()) {
			for (const auto& edge : next_step_it->second) {
				if (edge.node != source) {
					if (auto already_was_gonna_visit = std::find_if(nodes_to_visit.begin(), nodes_to_visit.end(), is_first_equals<Node, float>(edge.node));
						already_was_gonna_visit != nodes_to_visit.end()) {
						// If the node was already going to be visited, update its cost
						already_was_gonna_visit->second = std::min(already_was_gonna_visit->second, lowest_cost + edge.cost);
					} else if (is_less_or_equal(lowest_cost + edge.cost, inclusive_cost, _tolerance)) {
						// Check if this next node was already reachable with a lower cost of reaching
						if (auto edge_node_it = reachable_nodes.find(edge.node); edge_node_it != reachable_nodes.end()) {
							if (lowest_cost + edge.cost < edge_node_it->second) {
								nodes_to_visit.emplace_back(edge.node, lowest_cost + edge.cost);
							}
						} else {
							// Otherwise, add node to nodes_to_visit
							nodes_to_visit.emplace_back(edge.node, lowest_cost + edge.cost);
						}
					}
				}
			}
		}
	}
	return reachable_nodes;
}
