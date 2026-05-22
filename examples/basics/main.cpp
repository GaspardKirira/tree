# include <tree/avl_tree.hpp>
# include <tree/binary_search_tree.hpp>
# include <tree/splay_tree.hpp>
# include <iostream>
# include <sstream>
# include <cstddef>
# include <iterator>
# include <string>
# include <vector>

# include <tree/config.hpp>

static void binary_search_tree_example ();
static void avl_tree_example ();
static void splay_tree_example ();

int main () {
	std::cout << R"(
 _     _                                                  _       _
| |__ (_)_ __   __ _ _ __ _   _   ___  ___  __ _ _ __ ___| |__   | |_ _ __ ___  ___
| '_ \| | '_ \ / _` | '__| | | | / __|/ _ \/ _` | '__/ __| '_ \  | __| '__/ _ \/ _ \
| |_) | | | | | (_| | |  | |_| | \__ \  __/ (_| | | | (__| | | | | |_| | |  __/  __/
|_.__/|_|_| |_|\__,_|_|   \__, | |___/\___|\__,_|_|  \___|_| |_|  \__|_|  \___|\___|
                          |___/
	)" << '\n';

	binary_search_tree_example ();

	std::cout << R"(
             _   _
  __ ___   _| | | |_ _ __ ___  ___
 / _` \ \ / / | | __| '__/ _ \/ _ \
| (_| |\ V /| | | |_| | |  __/  __/
 \__,_| \_/ |_|  \__|_|  \___|\___|
	)" << '\n';

	avl_tree_example ();

	std::cout << R"(
           _               _
 ___ _ __ | | __ _ _   _  | |_ _ __ ___  ___
/ __| '_ \| |/ _` | | | | | __| '__/ _ \/ _ \
\__ \ |_) | | (_| | |_| | | |_| | |  __/  __/
|___/ .__/|_|\__,_|\__, |  \__|_|  \___|\___|
    |_|            |___/
	)" << '\n';

	splay_tree_example ();
}

void binary_search_tree_example () {
	tree::binary_search_tree <int> bst;
	const int x = 10;
	std::cout << bst.contains (10)
		<< " " << bst.contains (20)
		<< " " << bst.contains (5)
		<< " " << bst.contains (25)
		<< " " << bst.contains (15) << std::endl;
	bst.insert (x);
	std::cout << bst.contains (10)
		<< " " << bst.contains (20)
		<< " " << bst.contains (5)
		<< " " << bst.contains (25)
		<< " " << bst.contains (15) << std::endl;
	bst.insert (20);
	std::cout << bst.contains (10)
		<< " " << bst.contains (20)
		<< " " << bst.contains (5)
		<< " " << bst.contains (25)
		<< " " << bst.contains (15) << std::endl;
	bst.insert (5);
	std::cout << bst.contains (10)
		<< " " << bst.contains (20)
		<< " " << bst.contains (5)
		<< " " << bst.contains (25)
		<< " " << bst.contains (15) << std::endl;
	bst.insert (25);
	std::cout << bst.contains (10)
		<< " " << bst.contains (20)
		<< " " << bst.contains (5)
		<< " " << bst.contains (25)
		<< " " << bst.contains (15) << std::endl;
	bst.insert (15);
	std::cout << bst.contains (10)
		<< " " << bst.contains (20)
		<< " " << bst.contains (5)
		<< " " << bst.contains (25)
		<< " " << bst.contains (15) << std::endl;
	bst.insert (5);
	std::cout << bst.contains (10)
		<< " " << bst.contains (20)
		<< " " << bst.contains (5)
		<< " " << bst.contains (25)
		<< " " << bst.contains (15) << std::endl;
	bst.insert (2);
	std::cout << bst.contains (10)
		<< " " << bst.contains (20)
		<< " " << bst.contains (5)
		<< " " << bst.contains (25)
		<< " " << bst.contains (15) << std::endl;
	bst.insert (7);
	bst.insert (1);
	bst.insert (3);

	bst.remove (5); // 10, 20, 25, 15, 5
	std::stringstream ss;
	ss << "TREE:\n" << bst << std::endl;
	std::cout << ss.str () << std::endl;
	for (int i : (tree::vector <int>) bst) {
		std::cout << i << ", ";
	}
	std::cout << std::endl;

	std::cout << bst.find_min ()->data << " - " << bst.find_max ()->data << std::endl;
	std::cout << (* bst.root () == * bst.root ()) << std::endl;
	// const int y = 20;
	// bst1.insert(y);
}

void avl_tree_example () {
	tree::avl_tree <int> avlt;
	avlt.insert (50);
	avlt.insert (25);
	avlt.insert (75);
	avlt.insert (20);
	avlt.insert (30);
	avlt.insert (100);
	avlt.insert (27);
	decltype (avlt) avlt2 = avlt;
	avlt.insert (28);

	std::cout << avlt << std::endl;
	std::cout << (avlt == avlt2) << std::endl;

	std::cout << "====================" << std::endl;

	std::cout << avlt << std::endl;

	int x = 10;
	std::cout << avlt.contains (10)
		<< " " << avlt.contains (20)
		<< " " << avlt.contains (5)
		<< " " << avlt.contains (25)
		<< " " << avlt.contains (15) << std::endl;
	avlt.insert (x);
	std::cout << avlt.contains (10)
		<< " " << avlt.contains (20)
		<< " " << avlt.contains (5)
		<< " " << avlt.contains (25)
		<< " " << avlt.contains (15) << std::endl;
	avlt.insert (20);
	std::cout << avlt.contains (10)
		<< " " << avlt.contains (20)
		<< " " << avlt.contains (5)
		<< " " << avlt.contains (25)
		<< " " << avlt.contains (15) << std::endl;
	avlt.insert (5);
	std::cout << avlt.contains (10)
		<< " " << avlt.contains (20)
		<< " " << avlt.contains (5)
		<< " " << avlt.contains (25)
		<< " " << avlt.contains (15) << std::endl;
	avlt.insert (25);
	std::cout << avlt.contains (10)
		<< " " << avlt.contains (20)
		<< " " << avlt.contains (5)
		<< " " << avlt.contains (25)
		<< " " << avlt.contains (15) << std::endl;
	avlt.insert (15);
	std::cout << avlt.contains (10)
		<< " " << avlt.contains (20)
		<< " " << avlt.contains (5)
		<< " " << avlt.contains (25)
		<< " " << avlt.contains (15) << std::endl;
	avlt.insert (5);
	std::cout << avlt.contains (10)
		<< " " << avlt.contains (20)
		<< " " << avlt.contains (5)
		<< " " << avlt.contains (25)
		<< " " << avlt.contains (15) << std::endl;
	avlt.insert (2);
	std::cout << avlt.contains (10)
		<< " " << avlt.contains (20)
		<< " " << avlt.contains (5)
		<< " " << avlt.contains (25)
		<< " " << avlt.contains (15) << std::endl;
	avlt.insert (7);
	avlt.insert (1);
	avlt.insert (3);

	// avlt.remove (5); // 10, 20, 25, 15, 5
	std::stringstream ss;
	ss << "TREE:\n" << avlt << std::endl;
	std::cout << ss.str () << std::endl;
	std::cout << avlt << std::endl;
	for (int i : (tree::vector <int>) avlt) {
		std::cout << i << ", ";
	}
	std::cout << std::endl;

	std::vector <int> v;
	avlt.dump_sorted (std::back_inserter(v));
	for (int i : v) {
		std::cout << i << ", ";
	}
	std::cout << std::endl;

	std::cout << "\n====================\n" << std::endl;

	struct Traverse {
	public:
		enum class Algorithm {
			preorder, inorder, postorder
		};

	private:
		const Algorithm algorithm;
		std::size_t lpo = 0;

		inline void print_tree (const decltype (avlt)::node * n, std::size_t lpo) {
			for (std::size_t i = 1; i < lpo; i++) {
				std::cout << "    ";
			}
			std::cout << n->data << std::endl;
		};

	public:
		Traverse (Algorithm TT) : algorithm (TT) {}

		void operator () (const decltype (avlt)::node * n) {
			lpo++;

			if (Algorithm::preorder == algorithm) {
				print_tree (n, lpo);
			}

			if (nullptr != n->left) {
				this->operator () (n->left);
			}

			if (Algorithm::inorder == algorithm) {
				print_tree (n, lpo);
			}

			if (nullptr != n->right) {
				this->operator () ( n->right);
			}

			if (Algorithm::postorder == algorithm) {
				print_tree (n, lpo);
			}

			lpo--;
		}
	};

	Traverse t (Traverse::Algorithm::postorder);
	t (avlt.root ());
}

void splay_tree_example () {
	tree::splay_tree <int> spt;
	spt.insert (2);
	spt.insert (3);
	spt.insert (4);
	spt.insert (5);
	spt.insert (6);
	spt.insert (7);
	spt.insert (1);

	std::cout << spt << std::endl;

	spt.make_empty ();

	for (int i = 1; i <= 32; i++) {
		spt.insert (i);
	}

	for (int i = 1; i <= 9; i++) {
		spt.contains (i);
	}

	std::cout << spt << std::endl;
}
