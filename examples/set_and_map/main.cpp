# include <algorithm>
# include <functional>
# include <iostream>
# include <iterator>
# include <numeric>
# include <random>
# include <set>
# include <utility>
# include <vector>

# include <tree/avl_tree.hpp>

# include "map.hpp"
# include "set.hpp"


template <typename ... Ts>
// using map_tree = binary_search_tree <Ts ...>;
using map_tree = tree::avl_tree <Ts ...>;
// using map_tree = splay_tree <Ts ...>;

static void map_example ();
static void set_example ();


int main () {
	set_example ();
	map_example ();
}

void set_example () {
	const std::set <int> s1 {1, 2, 3, 4, 3, 2, 1};
	const tree::set <int> s2 {1, 2, 3, 4, 3, 2, 1};
	auto s3 = s2;
	decltype (s3) s4;
	s4 = std::move (s3);

	// NOLINTNEXTLINE(*-use-ranges)
	std::copy (s1.cbegin (), s1.cend (), std::ostream_iterator <int> (std::cout, ", "));
	std::cout << '\n';
	// NOLINTNEXTLINE(*-use-ranges)
	std::copy (s4.cbegin (), s4.cend (), std::ostream_iterator <int> (std::cout, ", "));
	std::cout << '\n';

	for (const int i : s4) {
		std::cout << i << ", ";
	}
	std::cout << '\n';
}

void map_example () {
	tree::map <int, int, std::less <>, map_tree> m; {
		std::vector <int> v (12);
		std::iota (v.begin (), v.end (), 1); // NOLINT(*-use-ranges)
		std::mt19937 rng (std::random_device {} ());
		std::ranges::shuffle (v, rng);
		std::ranges::for_each (v, [& m] (int i) -> void {
			m.insert (i, i + 1);
		});
	}

	std::cout << m.contains (10) << '\n';
	m.remove (10);
	std::cout << m.contains (10) << '\n';

	std::cout << m << '\n';

	m [4] = 44;
	m.at (22) = 222;

	for (auto [k, v] : m) {
		std::cout << "key: " << k << ", value: " << v << '\n';
	}
}
