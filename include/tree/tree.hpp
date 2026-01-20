#ifndef TREE_HPP
#define TREE_HPP

#include <cstddef>
#include <vector>

namespace tree
{
  struct Node
  {
    std::size_t id{};
    std::vector<std::size_t> children{};
  };

  inline std::vector<Node> make_chain(std::size_t n)
  {
    std::vector<Node> nodes;
    nodes.reserve(n);

    for (std::size_t i = 0; i < n; ++i)
      nodes.push_back(Node{i, {}});

    for (std::size_t i = 0; i + 1 < n; ++i)
      nodes[i].children.push_back(i + 1);

    return nodes;
  }

  inline std::size_t count_edges(const std::vector<Node> &nodes)
  {
    std::size_t edges = 0;
    for (const auto &node : nodes)
      edges += node.children.size();
    return edges;
  }

  inline std::size_t count_leaves(const std::vector<Node> &nodes)
  {
    std::size_t leaves = 0;
    for (const auto &node : nodes)
    {
      if (node.children.empty())
        ++leaves;
    }
    return leaves;
  }

  inline std::size_t count_nodes(const std::vector<Node> &nodes)
  {
    return nodes.size();
  }

}

#endif
