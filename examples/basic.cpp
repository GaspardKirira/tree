#include <iostream>
#include <tree/tree.hpp>

int main()
{
  auto nodes = tree::make_chain(5);
  std::cout << "nodes=" << nodes.size()
            << " edges=" << tree::count_edges(nodes)
            << "\n";
}
