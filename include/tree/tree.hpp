#ifndef TREE_HPP
#define TREE_HPP

#include <algorithm>
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

  inline std::vector<const Node *> index_by_id(const std::vector<Node> &nodes)
  {
    std::size_t maxId = 0;
    for (const auto &n : nodes)
      maxId = std::max(maxId, n.id);

    std::vector<const Node *> idx(maxId + 1, nullptr);
    for (const auto &n : nodes)
      idx[n.id] = &n;

    return idx;
  }

  template <typename NodeT>
  std::size_t count_nodes(const NodeT *root)
  {
    if (!root)
      return 0;

    std::size_t total = 1;

    for (const NodeT *child : root->children)
      total += count_nodes(child);

    return total;
  }

  template <typename NodeT>
  std::size_t max_depth(const NodeT *root)
  {
    if (!root)
      return 0;

    std::size_t depth = 0;

    for (const NodeT *child : root->children)
      depth = std::max(depth, max_depth(child));

    return depth + 1;
  }

  inline std::size_t count_internal_nodes(const std::vector<Node> &nodes)
  {
    std::size_t count = 0;
    for (const auto &n : nodes)
    {
      if (!n.children.empty())
        ++count;
    }
    return count;
  }

} // namespace tree

#endif
