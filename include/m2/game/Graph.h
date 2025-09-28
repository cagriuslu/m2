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
	class Graph {
	public:
		using Node = long long;
		struct Edge {
			Node toNode;
			FE cost;
		};

	private:
		// 2D position of nodes, helps with A* path finding.
		std::unordered_map<Node, VecFE> _nodePositions;
		std::unordered_map<Node, std::vector<Edge>> _edges;
		FE _tolerance;

	public:
		explicit Graph(const FE tolerance = FE{0.001f}) : _tolerance(tolerance) {}
		explicit Graph(const std::function<std::optional<std::pair<Node, Edge>>()>& generator, FE tolerance = FE{0.001f});

		/// Add a new edge between two nodes in the graph
		void AddEdge(Node from, Edge edge);
		/// This operation is optional because the position information is not used for every functionality.
		void SetNodePosition(Node node, const VecFE& position);

		/// Returns the nodes that can be reached from the `source` while spending at most `maxCost`.
		/// If there are multiple paths, the shortest one's cost is returned.
		using ReachableNodesAndCosts = std::unordered_map<Node, FE>;
		[[nodiscard]] ReachableNodesAndCosts FindNodesReachableFrom(Node source, FE maxCost) const;

		/// Returns the reversed path between two nodes. [to, to - 1, ..., from + 1, from].
		/// Returns [to] if `from` equals to `to`.
		/// Returns empty vector if path not found.
		using ReversePath = std::vector<Node>;
		ReversePath FindPathTo(Node from, Node to) const;

		// Utilities

		static std::multimap<FE, Node> order_by_cost(const ReachableNodesAndCosts&);
		/// If two sources are deemed equal, the reachable_nodes maps of those sources can be merged.
		/// If a node appears in both maps, the one with the lowest cost is used.
		static ReachableNodesAndCosts merge_reachable_nodes(const ReachableNodesAndCosts& nodes_1, const ReachableNodesAndCosts& nodes_2);
	};
}
