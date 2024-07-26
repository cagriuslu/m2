#include <gtest/gtest.h>
#include <m2/game/Graph.h>

TEST(Graph, basic) {
	int i = 0;
	auto generator = [&i]() -> std::optional<std::pair<m2::Graph::Node, m2::Graph::Edge>> {
		if (i < 100) {
			auto node_and_edge = std::make_pair(i, m2::Graph::Edge{i + 1, static_cast<float>(i) / 2.0f});
			i++;
			return node_and_edge; // Generates 0 -> 1 -> 2 -> ... -> 98 -> 99 -> 100
		} else if (100 <= i && i < 200) {
			int j = 200 - i; // j: 100, 99, 98, 97
			i++;
			return std::make_pair(j, m2::Graph::Edge{j - 1, static_cast<float>(j) / 2.0f}); // Generates 100 -> 99 -> ... -> 1 -> 0
		} else {
			return std::nullopt;
		}
	};
	m2::Graph graph{generator};

	auto reachable = graph.reachable_nodes_from(50, 75.0f);
	EXPECT_EQ(reachable.size(), 6);
}

TEST(Graph, reachable_nodes_from) {
	m2::Graph graph;
	graph.add_edge(0, {1, 1.0f});
	graph.add_edge(1, {2, 1.0f});
	graph.add_edge(2, {-1, 1.0f});
	graph.add_edge(0, {-1, 1.0f});
	graph.add_edge(-1, {2, 1.0f});
	graph.add_edge(-1, {1, 1.0f});
	graph.add_edge(-1, {0, 1.0f});
	graph.add_edge(-1, {-2, 1.0f});
	graph.add_edge(2, {3, 1.0f});
	graph.add_edge(-2, {-3, 1.0f});

	auto reachable = graph.reachable_nodes_from(0, 2.0f);
	EXPECT_EQ(reachable.size(), 5);
	EXPECT_TRUE(reachable.contains(1));
	EXPECT_TRUE(reachable.contains(2));
	EXPECT_TRUE(reachable.contains(-1));
	EXPECT_TRUE(reachable.contains(-2));
}
