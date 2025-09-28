#include <gtest/gtest.h>
#include <m2/game/Graph.h>

TEST(Graph, basic) {
	int i = 0;
	auto generator = [&i]() -> std::optional<std::pair<int, m2::Graph<int,int>::Edge>> {
		if (i < 100) {
			auto node_and_edge = std::make_pair(i, m2::Graph<int, int>::Edge{i + 1, m2::FE{static_cast<float>(i) / 2.0f}});
			i++;
			return node_and_edge; // Generates 0 -> 1 -> 2 -> ... -> 98 -> 99 -> 100
		} else if (100 <= i && i < 200) {
			int j = 200 - i; // j: 100, 99, 98, 97
			i++;
			return std::make_pair(j, m2::Graph<int,int>::Edge{j - 1, m2::FE{static_cast<float>(j) / 2.0f}}); // Generates 100 -> 99 -> ... -> 1 -> 0
		} else {
			return std::nullopt;
		}
	};
	m2::Graph<int,int> graph{generator};

	auto reachable = graph.FindNodesReachableFrom(50, m2::FE{75.0f});
	EXPECT_EQ(reachable.size(), 6);
}

TEST(Graph, FindNodesReachableFrom) {
	m2::Graph<int,int> graph;
	graph.AddEdge(0, {1, m2::FE{1.0f}});
	graph.AddEdge(1, {2, m2::FE{1.0f}});
	graph.AddEdge(2, {-1, m2::FE{1.0f}});
	graph.AddEdge(0, {-1, m2::FE{1.0f}});
	graph.AddEdge(-1, {2, m2::FE{1.0f}});
	graph.AddEdge(-1, {1, m2::FE{1.0f}});
	graph.AddEdge(-1, {0, m2::FE{1.0f}});
	graph.AddEdge(-1, {-2, m2::FE{1.0f}});
	graph.AddEdge(2, {3, m2::FE{1.0f}});
	graph.AddEdge(-2, {-3, m2::FE{1.0f}});

	auto reachable = graph.FindNodesReachableFrom(0, m2::FE{2.0f});
	EXPECT_EQ(reachable.size(), 5);
	EXPECT_TRUE(reachable.contains(1));
	EXPECT_TRUE(reachable.contains(2));
	EXPECT_TRUE(reachable.contains(-1));
	EXPECT_TRUE(reachable.contains(-2));
}

TEST(Graph, FindPathToSimple) {
	// 0 --> 1 --> 2 --> 3
	m2::Graph<int,int> graph;
	graph.AddEdge(0, {1, m2::FE::One()});
	graph.SetNodePosition(0, m2::VecFE{m2::FE{}, m2::FE{}});
	graph.AddEdge(1, {2, m2::FE::One()});
	graph.SetNodePosition(1, m2::VecFE{m2::FE{1}, m2::FE{}});
	graph.AddEdge(2, {3, m2::FE::One()});
	graph.SetNodePosition(2, m2::VecFE{m2::FE{2}, m2::FE{}});
	graph.SetNodePosition(3, m2::VecFE{m2::FE{3}, m2::FE{}});
	const auto reversePath = graph.FindPathTo(0, 3);
	EXPECT_EQ(reversePath[0], 3);
	EXPECT_EQ(reversePath[1], 2);
	EXPECT_EQ(reversePath[2], 1);
	EXPECT_EQ(reversePath[3], 0);
}

TEST(Graph, FindPathToComplex) {
	// 0 --> 1 --> 2 --> 3 seemingly long, but cheap route
	// 0 --> 3 seemingly short, but expensive route
	m2::Graph<int,int> graph;
	graph.AddEdge(0, {1, m2::FE::One()});
	graph.SetNodePosition(0, m2::VecFE{m2::FE{}, m2::FE{}});
	graph.AddEdge(1, {2, m2::FE::One()});
	graph.SetNodePosition(1, m2::VecFE{m2::FE{1}, m2::FE{}});
	graph.AddEdge(2, {3, m2::FE::One()});
	graph.SetNodePosition(2, m2::VecFE{m2::FE{2}, m2::FE{}});
	graph.AddEdge(0, {3, m2::FE{4}});
	graph.SetNodePosition(3, m2::VecFE{m2::FE{3}, m2::FE{}});
	const auto reversePath = graph.FindPathTo(0, 3);
	EXPECT_EQ(reversePath[0], 3);
	EXPECT_EQ(reversePath[1], 2);
	EXPECT_EQ(reversePath[2], 1);
	EXPECT_EQ(reversePath[3], 0);
}

TEST(Graph, FindPathToNaive) {
	// 0 --> 1 --> 2 --> 3
	// 0 --> 3
	m2::Graph<int,int> graph;
	graph.AddEdge(0, {1, m2::FE::One()});
	graph.SetNodePosition(0, m2::VecFE{m2::FE{}, m2::FE{}});
	graph.AddEdge(1, {2, m2::FE::One()});
	graph.SetNodePosition(1, m2::VecFE{m2::FE{1}, m2::FE{}});
	graph.AddEdge(2, {3, m2::FE::One()});
	graph.SetNodePosition(2, m2::VecFE{m2::FE{2}, m2::FE{}});
	graph.AddEdge(0, {3, m2::FE::One()});
	graph.SetNodePosition(3, m2::VecFE{m2::FE{3}, m2::FE{}});
	const auto reversePath = graph.FindPathTo(0, 3);
	EXPECT_EQ(reversePath[0], 3);
	EXPECT_EQ(reversePath[1], 0);
}
