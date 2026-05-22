# ifndef SPLAY_TREE
# define SPLAY_TREE

# include <concepts>
# include <functional>
# include <iterator>
# include <ostream>
# include <utility>

# include <tree/config.hpp>

# include <tree/binary_search_tree.hpp>

namespace tree {

namespace detail {

template <typename T, typename C, bool RemovePreserveLeft_>
struct splay_tree_ {
	// cppcheck-suppress unusedStructMember
	static constexpr bool RemovePreserveLeft = RemovePreserveLeft_;
	using tree = binary_search_tree <T, C, RemovePreserveLeft_>;
};

} // end namespace detail

template <typename T, typename Comparator = std::less <T>, bool RemovePreserveLeft = false>
requires std::strict_weak_order <Comparator, T, T>
class splay_tree : protected detail::splay_tree_ <T, Comparator, RemovePreserveLeft>::tree
{
protected:
	using detail = detail::splay_tree_ <T, Comparator, RemovePreserveLeft>;

public:
	using tree = detail::tree;
	using node = tree::node;
	using node_link = tree::node_link;
	using const_node_link = tree::const_node_link;
	using value_type = T;
	using tree::less_than;
	using iterator = tree::iterator;
	using const_iterator = tree::const_iterator;

public: // binary_search_tree interface
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

	bool operator== (const splay_tree & other) const {
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

	friend std::ostream & operator<< (std::ostream & os, const splay_tree & tree)
		requires requires (std::ostream & os, T t) {
			{ os << t } -> std::convertible_to <std::ostream &>;
		}
	{
		return os << static_cast <const splay_tree::tree &> (tree);
	}

	template <typename U>
	requires std::convertible_to <U, T>
	void insert (U && value) {
		stack <node_link> path = get_link_stack (value);
		if (nullptr == * path.top ()) {
			tree::insert_at (path.top (), std::forward <U> (value));
		}
		splay (std::move (path));
	}

	template <typename VC>
	requires tree::template ValueComparable <VC>
	bool contains (const VC & value) const {
		// return const_cast <splay_tree *> (this)->contains (value);
		return tree::contains (value);
	}

	template <typename VC>
	requires tree::template ValueComparable <VC>
	bool contains (const VC & value) {
		stack <node_link> path = get_link_stack (value);
		bool c = * path.top () != nullptr;

		if (false == c) {
			path.pop ();

			if (false == path.empty ()) {
				splay (std::move (path));
			}
		}
		else {
			splay (std::move (path));
		}

		return c;
	}

	template <typename VC>
	requires tree::template ValueComparable <VC>
	void remove (const VC & value) {
		// stack <node_link> path = get_link_stack (value);
		//
		// if (nullptr != * path.top ()) {
		// 	remove_at (path.top ());
		// }
		//
		// path.pop ();
		//
		// if (false == path.empty ()) {
		// 	splay (std::move (path));
		// }

		stack <node_link> path = get_link_stack (value);

		if (nullptr != * path.top ()) {
			splay (std::move (path));

			node_link link = & this->m_root;
			node_link link_left = & (* link)->left;
			node_link link_right = & (* link)->right;

			node * to_remove = * link;

			if (nullptr == * link_right) {
				* link = * link_left;
			}
			else if (nullptr == * link_left) {
				* link = * link_right;
			}
			else {
				if constexpr (true == detail::RemovePreserveLeft) {
					path = get_link_stack_to_rightmost (link_left);
					splay (std::move (path));
					(* link_left)->right = * link_right;
					* link = * link_left;
				}
				else {
					path = get_link_stack_to_leftmost (link_right);
					splay (std::move (path));
					(* link_right)->left = * link_left;
					* link = * link_right;
				}
			}

			to_remove->left = nullptr;
			to_remove->right = nullptr;
			delete to_remove; // NOLINT(cppcoreguidelines-owning-memory)
			this->m_size--;
		}
		else {
			// path.pop ();
			// if (false == path.empty ()) {
			// 	splay (std::move (path));
			// }
		}
	}

protected:
	void splay (stack <node_link> && path) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
		node_link current = path.top (); // path always contains & m_root
		path.pop ();

		if (nullptr == * current || true == path.empty ()) {
			return;
		}

		node_link parent = path.top ();
		path.pop ();

		node_link grandparent;

		if (false == path.empty ()) {
			grandparent = path.top ();
			path.pop ();
		}
		else {
			grandparent = nullptr;
		}

		bool currentIsParentLeft = * current == (* parent)->left;

		while (true) {
			node * c = * current;
			node * p = * parent;

			if (nullptr == grandparent) {
				if (true == currentIsParentLeft) {
					p->left = c->right;
					c->right = p;
					* parent = c;
				}
				else {
					p->right = c->left;
					c->left = p;
					* parent = c;
				}

				break;
			}
			else {
				bool parentIsGrandParentLeft = * parent == (* grandparent)->left;

				node * g = * grandparent; (void) g;

				if (true == currentIsParentLeft) {
					if (true == parentIsGrandParentLeft) {
						g->left = p->right;
						p->left = c->right;
						p->right = g;
						c->right = p;
						* grandparent = c;
					}
					else {
						g->right = c->left;
						p->left = c->right;
						c->left = g;
						c->right = p;
						* grandparent = c;
					}
				}
				else {
					if (false == parentIsGrandParentLeft) {
						g->right = p->left;
						p->right = c->left;
						p->left = g;
						c->left = p;
						* grandparent = c;
					}
					else {
						g->left = c->right;
						p->right = c->left;
						c->left = p;
						c->right = g;
						* grandparent = c;
					}
				}

				if (false == path.empty ()) {
					current = grandparent;

					parent = path.top ();
					path.pop ();

					if (false == path.empty ()) {
						grandparent = path.top ();
						path.pop ();
					}
					else {
						grandparent = nullptr;
					}

					currentIsParentLeft = * current == (* parent)->left;
				}
				else {
					break;
				}
			}
		}
	}

protected:
	using tree::insert_at;
	using tree::remove_at;
	using tree::at;
	using tree::get_link_stack;
	using tree::get_link_stack_to_leftmost;
	using tree::get_link_stack_to_rightmost;
	using tree::get_link_to_leftmost;
	using tree::get_link_to_rightmost;
};

} // end namespace tree

# endif // SPLAY_TREE
