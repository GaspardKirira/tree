#ifndef TREE_HPP
#define TREE_HPP

#include <algorithm>
#include <cstddef>
#include <optional>
#include <queue>
#include <unordered_set>
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

  // Root = 0, children = 1..n-1
  inline std::vector<Node> make_star(std::size_t n)
  {
    std::vector<Node> nodes;
    nodes.reserve(n);

    for (std::size_t i = 0; i < n; ++i)
      nodes.push_back(Node{i, {}});

    if (n == 0)
      return nodes;

    nodes[0].children.reserve(n > 0 ? (n - 1) : 0);
    for (std::size_t i = 1; i < n; ++i)
      nodes[0].children.push_back(i);

    return nodes;
  }

  // Heap-like binary tree: children of i are (2i+1) and (2i+2) if in range.
  inline std::vector<Node> make_binary_tree(std::size_t n)
  {
    std::vector<Node> nodes;
    nodes.reserve(n);

    for (std::size_t i = 0; i < n; ++i)
      nodes.push_back(Node{i, {}});

    for (std::size_t i = 0; i < n; ++i)
    {
      const std::size_t l = 2 * i + 1;
      const std::size_t r = 2 * i + 2;
      if (l < n)
        nodes[i].children.push_back(l);
      if (r < n)
        nodes[i].children.push_back(r);
    }

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

  // Index: idx[id] = pointer to node with that id, nullptr if missing.
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

  // parent[id] = parentId if exists, nullopt otherwise.
  // If multiple parents exist, the *last* one wins (still usable for many cases).
  inline std::vector<std::optional<std::size_t>> parent_index(const std::vector<Node> &nodes)
  {
    std::size_t maxId = 0;
    for (const auto &n : nodes)
      maxId = std::max(maxId, n.id);

    std::vector<std::optional<std::size_t>> parent(maxId + 1, std::nullopt);

    for (const auto &n : nodes)
    {
      for (std::size_t cid : n.children)
      {
        if (cid < parent.size())
          parent[cid] = n.id;
      }
    }

    return parent;
  }

  // Returns the unique root id if exactly one node has no parent, else nullopt.
  inline std::optional<std::size_t> find_root(const std::vector<Node> &nodes)
  {
    if (nodes.empty())
      return std::nullopt;

    auto parent = parent_index(nodes);

    // A "root" must exist in nodes. We'll search by iterating nodes and checking parent[n.id].
    std::optional<std::size_t> root = std::nullopt;

    for (const auto &n : nodes)
    {
      const std::size_t id = n.id;
      if (id >= parent.size())
        continue;

      if (!parent[id].has_value())
      {
        if (root.has_value())
          return std::nullopt; // more than one root
        root = id;
      }
    }

    return root;
  }

  // BFS order from rootId (ignores missing ids and avoids infinite loops if graph is not a tree).
  inline std::vector<std::size_t> bfs_order(const std::vector<Node> &nodes, std::size_t rootId)
  {
    std::vector<std::size_t> order;
    if (nodes.empty())
      return order;

    const auto idx = index_by_id(nodes);
    if (rootId >= idx.size() || idx[rootId] == nullptr)
      return order;

    std::queue<std::size_t> q;
    std::unordered_set<std::size_t> seen;
    seen.reserve(nodes.size() * 2);

    q.push(rootId);
    seen.insert(rootId);

    while (!q.empty())
    {
      const std::size_t id = q.front();
      q.pop();
      order.push_back(id);

      const Node *n = (id < idx.size()) ? idx[id] : nullptr;
      if (!n)
        continue;

      for (std::size_t cid : n->children)
      {
        if (cid < idx.size() && idx[cid] != nullptr && seen.insert(cid).second)
          q.push(cid);
      }
    }

    return order;
  }

  // Count reachable nodes from rootId (safe even if not a tree).
  inline std::size_t count_nodes_reachable(const std::vector<Node> &nodes, std::size_t rootId)
  {
    return bfs_order(nodes, rootId).size();
  }

  // Max depth from rootId (levels count: root depth = 1). Safe with visited set.
  inline std::size_t max_depth(const std::vector<Node> &nodes, std::size_t rootId)
  {
    if (nodes.empty())
      return 0;

    const auto idx = index_by_id(nodes);
    if (rootId >= idx.size() || idx[rootId] == nullptr)
      return 0;

    std::queue<std::pair<std::size_t, std::size_t>> q; // (id, depth)
    std::unordered_set<std::size_t> seen;
    seen.reserve(nodes.size() * 2);

    q.push({rootId, 1});
    seen.insert(rootId);

    std::size_t best = 0;

    while (!q.empty())
    {
      const auto [id, depth] = q.front();
      q.pop();
      best = std::max(best, depth);

      const Node *n = (id < idx.size()) ? idx[id] : nullptr;
      if (!n)
        continue;

      for (std::size_t cid : n->children)
      {
        if (cid < idx.size() && idx[cid] != nullptr && seen.insert(cid).second)
          q.push({cid, depth + 1});
      }
    }

    return best;
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

  template <typename NodeT>
  std::size_t count_leaves(const NodeT *root)
  {
    if (!root)
      return 0;
    if (root->children.empty())
      return 1;

    std::size_t leaves = 0;
    for (const NodeT *child : root->children)
      leaves += count_leaves(child);
    return leaves;
  }

  template <typename NodeT, typename Fn>
  void for_each_preorder(const NodeT *root, Fn &&fn)
  {
    if (!root)
      return;
    fn(root);
    for (const NodeT *child : root->children)
      for_each_preorder(child, std::forward<Fn>(fn));
  }

} // namespace tree

#endif
