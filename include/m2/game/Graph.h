#pragma once
#include <unordered_map>
#include <vector>
#include <functional>
#include <optional>

namespace m2 {
	// The graph is optimized for high number of nodes, but low number of connections.
	// Nodes are stored in a map, but edges are stored in a vector.
	class Graph {
	public:
		using Node = long long;
		struct Edge {
			Node node;
			float cost;
		};

	private:
		std::unordered_map<Node, std::vector<Edge>> _edges;
		float _tolerance;

	public:
		explicit Graph(float tolerance = 0.001f) : _tolerance(tolerance) {}
		explicit Graph(const std::function<std::optional<std::pair<Node, Edge>>()>& generator, float tolerance = 0.001f);

		void add_edge(Node from, Edge edge);

		// Returns the nodes that can be reached from the `source` while spending at most `inclusive_cost`.
		// If there are multiple paths, the shortest one's cost is returned.
		[[nodiscard]] std::unordered_map<Node, float> reachable_nodes_from(Node source, float inclusive_cost) const;
	};
}
