#pragma once
#include <m2/math/primitives/Exact.h>
#include <m2/math/primitives/Float.h>
#include <m2/ProxyTypes.h>
#include <unordered_map>
#include <map>
#include <vector>
#include <functional>
#include <optional>

namespace m2 {
	// Graph is a data structure that contains nodes, and unidirectional edges between them. All edges have an
	// associated cost. Graph is optimized for high number of nodes but low number of edges, because nodes are stored in
	// a map, but edges are stored in a vector.
	class Graph {
	public:
		using Node = long long;
		struct Edge {
			Node toNode;
			FE cost;
		};

	private:
		std::unordered_map<Node, std::vector<Edge>> _edges;
		FE _tolerance;

	public:
		explicit Graph(const FE tolerance = FE{0.001f}) : _tolerance(tolerance) {}
		explicit Graph(const std::function<std::optional<std::pair<Node, Edge>>()>& generator, FE tolerance = FE{0.001f});

		void AddEdge(Node from, Edge edge);

		// Returns the nodes that can be reached from the `source` while spending at most `inclusive_cost`.
		// If there are multiple paths, the shortest one's cost is returned.
		using ReachableNodesAndCosts = std::unordered_map<Node, FE>;
		[[nodiscard]] ReachableNodesAndCosts FindNodesReachableFrom(Node source, FE inclusive_cost) const;

		// Utilities

		static std::multimap<FE, Node> order_by_cost(const ReachableNodesAndCosts&);
		// If two sources are deemed equal, the reachable_nodes maps of those sources can be merged.
		// If a node appears in both maps, the one with the lowest cost is used.
		static ReachableNodesAndCosts merge_reachable_nodes(const ReachableNodesAndCosts& nodes_1, const ReachableNodesAndCosts& nodes_2);
	};
}
