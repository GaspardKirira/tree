# ifndef AVL_TREE_H
# define AVL_TREE_H

# include <concepts>
# include <cstddef>
# include <functional>
# include <iterator>
# include <ostream>
# include <type_traits>
# include <utility>

# include <tree/config.hpp>

# include <tree/binary_search_tree.hpp>

namespace tree::detail {

template <typename T, typename = void>
inline constexpr bool is_tuple_like_ = false;

template <typename T>
inline constexpr bool is_tuple_like_ <T, std::void_t <
	decltype (std::tuple_size <T>::value)> // NOLINT(modernize-type-traits)
> = true;

template <typename T>
struct avl_tree_node_data_  {
	// static_assert (false == requires (Data d) { d.value; }, "the 'value' member of Data is reserved");
	// static_assert (
	// 	false == requires (Data d) { d.height_plus_one; },
	// 	"the 'height_plus_one' member of Data is reserved"
	// );

	T value;
	std::size_t height_plus_one; // TODO: replace height with diff of subtree heights, which is always -1, 0 or 1

	// cppcheck-suppress-begin noExplicitConstructor
	explicit (false) avl_tree_node_data_ (const T & value, std::size_t height_plus_one = 0)
		: value (value), height_plus_one (height_plus_one) {}

	explicit (false) avl_tree_node_data_ (T && value, std::size_t height_plus_one = 0)
		: value (std::move (value)), height_plus_one (height_plus_one) {}
	// cppcheck-suppress-end noExplicitConstructor

	friend std::ostream & operator<< (std::ostream & os, const avl_tree_node_data_ & data) {
		os << data.value;
		return os;
	}

	operator const T & () const {
		return value;
	}

	operator T & () {
		return value;
	}
};

template <typename T, typename C>
struct avl_tree_ {
	using node_data = avl_tree_node_data_ <T>;
	using tree = binary_search_tree <node_data, C, false>;
};

} // end namespace tree::detail



namespace std {

template <typename T>
requires tree::detail::is_tuple_like_ <T>
// NOLINTNEXTLINE(bugprone-std-namespace-modification,cert-dcl58-cpp)
struct tuple_size <tree::detail::avl_tree_node_data_ <T>>
	: tuple_size <T> {};

template <std::size_t I, typename T>
requires tree::detail::is_tuple_like_ <T>
// NOLINTNEXTLINE(bugprone-std-namespace-modification,cert-dcl58-cpp)
struct tuple_element <I, tree::detail::avl_tree_node_data_ <T>>
	: tuple_element <I, T> {};

} // end namespace std



namespace tree {

template <typename T, typename Comparator = std::less <T>>
requires std::strict_weak_order <Comparator, T, T>
class avl_tree : protected detail::avl_tree_ <T, Comparator>::tree {
protected:
	using detail = detail::avl_tree_ <T, Comparator>;

public:
	using tree = detail::tree;
	using node = tree::node;
	using node_link = tree::node_link;
	using const_node_link = tree::const_node_link;
	using node_data = detail::node_data;
	using value_type = T;
	using tree::less_than;
	using iterator = tree::iterator;
	using const_iterator = tree::const_iterator;

public: // binary_search_tree interface
	using tree::contains;
	using tree::dump_invariant;
	using tree::dump_sorted;
	using tree::make_empty;
	using tree::root;
	using tree::size;
	using tree::empty;
	using tree::internal_path_length;
	using tree::begin;
	using tree::cbegin;
	using tree::end;
	using tree::cend;
	using tree::find;

	bool operator== (const avl_tree & other) const {
		return
			   static_cast <const tree &> (* this)
			== static_cast <const tree &> (other)
		;
	}

	operator vector <T> () const {
		vector <T> v;
		v.reserve (size ());

		dump_sorted (std::back_inserter (v));

		return v;
	}

	friend std::ostream & operator<< (std::ostream & os, const avl_tree & tree)
		requires requires (std::ostream & os, T t) {
			{ os << t } -> std::convertible_to <std::ostream &>;
		}
	{
		// return os << static_cast <const avl_tree::tree &> (tree);

		tree.root ()->preorder_traverse (
			[path = stack <const typename std::decay_t <decltype (tree)>::node *> (), &os]
			(auto * node, auto * parent) mutable -> void {
				if (nullptr != parent) {
					while (path.top () != parent) { // NOLINT(altera-id-dependent-backward-branch)
						path.pop ();
					}
				}
				path.push (node);

				std::size_t depth = path.size () - 1;

				for (std::size_t i = 0; i < depth; i++) { // NOLINT(altera-id-dependent-backward-branch)
					os << "  ";
				}

				if (nullptr != parent) {
					os << "|- ";
				}

				os << node->data;
				os << " (" << (static_cast <int> (node->data.height_plus_one) - 1) << ")";

				if (nullptr != parent) {
					if (nullptr == parent->left || nullptr == parent->right) {
						if (node == parent->left) {
							os << " (L)";
						}
						else {
							os << " (R)";
						}
					}
				}

				os << '\n';
			}
		);

		return os;
	}

public:
	template <typename U>
	requires std::convertible_to <U, T>
	void insert (U && value) {
		stack <node_link> path = get_link_stack (value);
		node_link link = path.top (); // path is always non-empty

		if (nullptr == * link) {
			// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
			* link = new node (node_data (std::forward <U> (value), 1));
			this->m_size++;
			rebalance_after_insert (std::move (path));
		}
	}

	template <typename VC>
	requires tree::template ValueComparable <VC>
	void remove (const VC & value) {
		stack <node_link> path = get_link_stack (value);
		node_link link = path.top ();

		if (nullptr != * link) {
			node_link link_left = & (* link)->left;
			node_link link_right = & (* link)->right;

			node * to_remove = * link;

			if (nullptr == * link_left) {
				path.pop ();
				* link = * link_right;
			}
			else if (nullptr == * link_right) {
				path.pop ();
				* link = * link_left;
			}
			else {
				if ((* link_left)->data.height_plus_one > (* link_right)->data.height_plus_one) {
					node_link left_rightmost_link = get_link_to_rightmost (link_left);

					node * left_rightmost = * left_rightmost_link;
					left_rightmost->right = * link_right;
					left_rightmost->data.height_plus_one = to_remove->data.height_plus_one;

					if (left_rightmost_link != link_left) {
						* left_rightmost_link = left_rightmost->left;
						left_rightmost->left = * link_left;

						node_link n = & left_rightmost->left;

						do {
							path.push (n);
							n = & (* n)->right;
						}
						while (* left_rightmost_link != * n); // NOLINT(altera-id-dependent-backward-branch)
					}

					* link = left_rightmost;
				}
				else {
					node_link right_leftmost_link = get_link_to_leftmost (link_right);

					node * right_leftmost = * right_leftmost_link;
					right_leftmost->left = * link_left;
					right_leftmost->data.height_plus_one = to_remove->data.height_plus_one;

					if (right_leftmost_link != link_right) {
						* right_leftmost_link = right_leftmost->right;
						right_leftmost->right = * link_right;

						node_link n = & right_leftmost->right;

						do {
							path.push (n);
							n = & (* n)->left;
						}
						while (* right_leftmost_link != * n); // NOLINT(altera-id-dependent-backward-branch)
					}

					* link = right_leftmost;
				}
			}

			to_remove->left = nullptr;
			to_remove->right = nullptr;
			delete to_remove; // NOLINT(cppcoreguidelines-owning-memory)
			this->m_size--;
			rebalance_after_remove (std::move (path));
		}
	}

private:
	// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
	void rebalance_after_insert (stack <node_link> && path) {
		node_link child = path.top ();
		path.pop ();

		bool prev_left = false;

		while (false == path.empty ()) {
			node_link link = path.top ();
			path.pop ();
			node * n = * link;
			node * nl = n->left;
			node * nr = n->right;

			bool left = * child == (* link)->left;

			std::size_t lh = get_node_height_plus_one (nl);
			std::size_t rh = get_node_height_plus_one (nr);

			if (lh > rh && lh - rh > 1) {
				if (true == prev_left) {
					node * nlr = nl->right;

					nl->right = n;
					n->left = nlr;

					* link = nl;

					n->data.height_plus_one -= 1;
				}
				else {
					node * nlr = nl->right;
					node * nlrl = nlr->left;
					node * nlrr = nlr->right;

					* link = nlr;

					nlr->left = nl;
					nlr->right = n;
					nl->right = nlrl;
					n->left = nlrr;

					n->data.height_plus_one -= 1;
					nl->data.height_plus_one -= 1;
					nlr->data.height_plus_one += 1;
				}

				break; // since link->data.height_plus_one remains the same
			}
			else if (rh > lh && rh - lh > 1) {
				if (false == prev_left) {
					node * nrl = nr->left;

					* link = nr;

					nr->left = n;
					n->right = nrl;

					n->data.height_plus_one -= 1;
				}
				else {
					node * nrl = nr->left;
					node * nrll = nrl->left;
					node * nrlr = nrl->right;

					* link = nrl;

					nrl->left = n;
					nrl->right = nr;
					n->right = nrll;
					nr->left = nrlr;

					n->data.height_plus_one -= 1;
					nr->data.height_plus_one -= 1;
					nrl->data.height_plus_one += 1;
				}

				break; // since link->data.height_plus_one remains the same
			}
			else {
				const std::size_t new_height = 1 + (lh > rh ? lh : rh);

				if (new_height == n->data.height_plus_one) {
					break;
				}
				else {
					n->data.height_plus_one = new_height;
				}
			}

			child = link;
			prev_left = left;
		}
	}

	// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
	void rebalance_after_remove (stack <node_link> && path) {
		while (false == path.empty ()) {
			node_link link = path.top ();
			path.pop ();
			node * n = * link;
			node * nl = n->left;
			node * nr = n->right;

			std::size_t lh = get_node_height_plus_one (nl);
			std::size_t rh = get_node_height_plus_one (nr);

			if (lh > rh && lh - rh > 1) {
				node * nll = nl->left;
				node * nlr = nl->right;

				std::size_t nllh = get_node_height_plus_one (nll);
				std::size_t nlrh = get_node_height_plus_one (nlr);

				if (nllh >= nlrh) {
					n->left = nlr;
					nl->right = n;
					* link = nl;

					if (nllh == nlrh) {
						n->data.height_plus_one -= 1;
						nl->data.height_plus_one += 1;
						break;
					}
					else {
						n->data.height_plus_one -= 2;
					}
				}
				else {
					node * nlrl = nlr->left;
					node * nlrr = nlr->right;

					n->left = nlrr;
					nl->right = nlrl;
					nlr->left = nl;
					nlr->right = n;

					* link = nlr;

					n->data.height_plus_one -= 2;
					nl->data.height_plus_one -= 1;
					nlr->data.height_plus_one += 1;
				}
			}
			else if (rh > lh && rh - lh > 1) {
				node * nrl = nr->left;
				node * nrr = nr->right;

				std::size_t nrlh = get_node_height_plus_one (nrl);
				std::size_t nrrh = get_node_height_plus_one (nrr);

				if (nrlh <= nrrh) {
					n->right = nrl;
					nr->left = n;
					* link = nr;

					if (nrlh == nrrh) {
						n->data.height_plus_one -= 1;
						nr->data.height_plus_one += 1;
						break;
					}
					else {
						n->data.height_plus_one -= 2;
					}
				}
				else {
					node * nrll = nrl->left;
					node * nrlr = nrl->right;

					n->right = nrll;
					nr->left = nrlr;
					nrl->left = n;
					nrl->right = nr;
					* link = nrl;

					n->data.height_plus_one -= 2;
					nr->data.height_plus_one -= 1;
					nrl->data.height_plus_one += 1;
				}
			}
			else {
				const std::size_t new_height = 1 + (lh > rh ? lh : rh);

				if (new_height == n->data.height_plus_one) {
					break;
				}
				else {
					n->data.height_plus_one = new_height;
				}
			}
		}
	}

	static inline std::size_t get_node_height_plus_one (node * node) {
		if (nullptr == node) {
			return 0;
		}
		else {
			return node->data.height_plus_one;
		}
	}

protected:
	using tree::get_link_stack;
	using tree::get_link_stack_to_leftmost;
	using tree::get_link_stack_to_rightmost;
	using tree::get_link_to_leftmost;
	using tree::get_link_to_rightmost;
};

} // end namespace tree

# endif // AVL_TREE_H
