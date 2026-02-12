# tree

A tiny header-only  example library for the Vix Registry demo.

## Usage
- Include headers from `include/`
- Example in `examples/basic.cpp`
```cpp
#include <iostream>
#include <tree/tree.hpp>

int main()
{
  auto nodes = tree::make_binary_tree(15);

  auto root = tree::find_root(nodes);
  if (!root)
  {
    std::cerr << "no unique root\n";
    return 1;
  }

  std::cout << "nodes=" << nodes.size()
            << " edges=" << tree::count_edges(nodes)
            << " leaves=" << tree::count_leaves(nodes)
            << " depth=" << tree::max_depth(nodes, *root)
            << " reachable=" << tree::count_nodes_reachable(nodes, *root)
            << "\n";

  auto order = tree::bfs_order(nodes, *root);
  std::cout << "bfs:";
  for (auto id : order)
    std::cout << " " << id;
  std::cout << "\n";
}
```
---

## License

MIT © [Gaspard Kirira Authors](https://github.com/Gaspardkirira/tree)
