# ifndef BINARY_SEARCH_TREE
# define BINARY_SEARCH_TREE

# include <concepts>
# include <cstddef>
# include <functional>
# include <initializer_list>
# include <iterator>
# include <ostream>
# include <type_traits>
# include <utility>

# include <tree/config.hpp>

# include <tree/binary_tree_node.hpp>

namespace tree {

namespace detail {

template <typename T, typename Comparator>
requires std::strict_weak_order <Comparator, T, T>
struct equal_to {
	static constexpr Comparator comparator {};

	bool operator () (const T & first, const T & second) const {
		return
			   false == comparator (first, second)
			&& false == comparator (second, first)
		;
	}
};

template <typename T, typename U>
requires std::convertible_to <T, U>
struct equal_to <T, std::less <U>> : std::equal_to <U> {};

template <typename T, typename U>
requires std::convertible_to <T, U>
struct equal_to <T, std::greater <U>> : std::equal_to <U> {};

template <typename T, typename Comparator, bool KeepInvariant_, bool RemovePreserveLeft_>
requires std::strict_weak_order <Comparator, T, T>
struct binary_search_tree_ {
	using equal_to = equal_to <T, Comparator>;
	using node = binary_tree_node <T, equal_to>;

	template <typename T2>
	static constexpr bool ValueComparable = requires (T t, T2 t2, const Comparator c) {
		{ c (t, t2) } -> std::convertible_to <bool>;
		{ c (t2, t) } -> std::convertible_to <bool>;
	};

	// cppcheck-suppress unusedStructMember
	static constexpr bool KeepInvariant = KeepInvariant_;
	// cppcheck-suppress unusedStructMember
	static constexpr bool RemovePreserveLeft = RemovePreserveLeft_;

	template <typename U>
	static constexpr bool is_node_link_v = node::template is_node_link_v <U>;
};

template <typename T, typename = void>
struct is_tree : std::false_type {};

template <template <typename ...> typename C, typename T>
struct is_tree <C <T>, std::void_t <
	typename C <T>::node,
	typename C <T>::node_link,
	typename C <T>::const_node_link,
	std::enable_if_t <std::strict_weak_order <decltype (C <T>::less_than), T, T>>,

	typename C <T>::iterator,
	typename C <T>::const_iterator,

	std::enable_if_t <
		std::is_same_v <typename C <T>::iterator, decltype (std::declval <C <T>> ().begin ())> &&
		std::is_same_v <typename C <T>::const_iterator, decltype (std::declval <const C <T>> ().begin ())> &&
		std::is_same_v <typename C <T>::iterator, decltype (std::declval <C <T>> ().end ())> &&
		std::is_same_v <typename C <T>::const_iterator, decltype (std::declval <const C <T>> ().end ())>
	>,

	std::enable_if_t <
		std::is_same_v <typename C <T>::const_iterator, decltype (std::declval <C <T>> ().cbegin ())> &&
		std::is_same_v <typename C <T>::const_iterator, decltype (std::declval <const C <T>> ().cbegin ())> &&
		std::is_same_v <typename C <T>::const_iterator, decltype (std::declval <C <T>> ().cend ())> &&
		std::is_same_v <typename C <T>::const_iterator, decltype (std::declval <const C <T>> ().cend ())>
	>,

	std::enable_if_t <
		std::is_copy_constructible_v <C <T>> == std::is_copy_constructible_v <T> &&
		std::is_copy_assignable_v <C <T>> == std::is_copy_assignable_v <T> &&
		std::is_nothrow_move_constructible_v <C <T>> &&
		std::is_nothrow_move_assignable_v <C <T>> &&
		std::is_nothrow_destructible_v <C <T>> &&
		std::is_default_constructible_v <C <T>>
	>,

	std::enable_if_t <
		std::is_convertible_v <decltype (std::declval <const C <T>> ().operator== (std::declval <const C <T>> ())), bool> &&
		std::is_convertible_v <const C <T>, vector <T>>
		&& std::is_same_v <decltype (std::declval <std::ostream &> () << std::declval <const C <T>> ()), std::ostream &>
	>,

	decltype (std::declval <C <T>> ().insert (std::declval <T> ())),
	std::enable_if_t <std::is_convertible_v <decltype (std::declval <const C <T>> ().contains (std::declval <T> ())), bool>>,
	decltype (std::declval <C <T>> ().remove (std::declval <T> ())),
	decltype (std::declval <const C <T>> ().dump_sorted (std::declval <std::back_insert_iterator <vector <T>>> ())),
	decltype (std::declval <const C <T>> ().dump_invariant (std::declval <std::back_insert_iterator <vector <T>>> ())),
	decltype (std::declval <C <T>> ().make_empty ()),
	std::enable_if_t <std::is_same_v <decltype (std::declval <const C <T>> ().size ()), std::size_t>>,
	std::enable_if_t <std::is_same_v <decltype (std::declval <const C <T>> ().internal_path_length ()), std::size_t>>,
	std::enable_if_t <std::is_same_v <decltype (std::declval <const C <T>> ().find (std::declval <T> ())), typename C <T>::const_iterator>>,
	// std::enable_if_t <std::is_same_v <decltype (std::declval <C <T>> ().find (std::declval <T> ())), typename C <T>::iterator>>,

	// do we need find_min (), find_max () and root () ?
	void
>>
	: std::true_type {};

} // end namespace detail

template <typename T>
constexpr inline bool is_tree_v = detail::is_tree <T>::value;

template <typename T, typename Comparator = std::less <T>, bool KeepInvariant = false, bool RemovePreserveLeft = false>
requires std::strict_weak_order <Comparator, T, T>
class binary_search_tree {
protected:
	using detail = detail::binary_search_tree_ <T, Comparator, KeepInvariant, RemovePreserveLeft>;

public:
	using node = detail::node;
	using node_link = node::link;
	using const_node_link = node::const_link;
	static constexpr Comparator less_than {};
	template <typename T2>
	static constexpr bool ValueComparable = detail::template ValueComparable <T2>;

	using iterator = node::const_iterator;
	using const_iterator = node::const_iterator;

	iterator begin () const { return const_iterator::begin (m_root); }
	iterator end () const { return const_iterator::end (m_root); }
	const_iterator cbegin () const { return const_iterator::begin (m_root); }
	const_iterator cend () const { return const_iterator::end (m_root); }

	const_iterator find (const T & value) const {
		stack <const_node_link> path = get_link_stack <const_node_link> (value);
		if (nullptr != path.top ()) {
			return const_iterator (& m_root, path);
		}
		else {
			return cend ();
		}
	}

public:
	binary_search_tree ()
		: m_root (nullptr)
		, m_size (0)
	{}

	template <std::convertible_to <T> U>
	binary_search_tree (std::initializer_list <U> list)
		: m_root (nullptr)
		, m_size (0)
	{
		for (const U & e : list) {
			insert (e);
		}
	}

	binary_search_tree (const binary_search_tree & other)
		// requires (std::is_copy_constructible_v <T>)
	{
		if (nullptr != other.m_root) {
			m_root = new node (* other.m_root); // NOLINT(cppcoreguidelines-owning-memory)
			m_size = other.m_size;
		}
		else {
			m_root = nullptr;
			m_size = 0;
		}
	}

	binary_search_tree & operator= (const binary_search_tree & other) {
		if (this != & other) {
			delete m_root;

			if (nullptr != other.m_root) {
				m_root = new node (* other.m_root); // NOLINT(cppcoreguidelines-owning-memory)
				m_size = other.m_size;
			}
			else {
				m_root = nullptr;
				m_size = 0;
			}
		}

		return * this;
	}

	binary_search_tree (binary_search_tree && other) noexcept
		: m_root (other.m_root)
		, m_size (other.m_size)
	{
		other.m_root = nullptr;
		other.m_size = 0;
	}

	binary_search_tree & operator= (binary_search_tree && other) noexcept {
		if (this != & other) {
			std::swap (m_root, other.m_root);
			std::swap (m_size, other.m_size);
		}

		return * this;
	}

	~binary_search_tree () {
		delete m_root;
		m_size = 0;
	}

public:
	bool operator== (const binary_search_tree & other) const {
		if (m_size != other.m_size) {
			return false;
		}

		if (nullptr != m_root && nullptr != other.m_root) {
			return * m_root == * other.m_root;
		}
		else {
			return m_root == other.m_root;
		}
	}

	operator vector <T> () const {
		vector <T> v;
		v.reserve (m_size);

		dump_sorted (std::back_inserter (v));

		return v;
	}

	friend std::ostream & operator<< (std::ostream & os, const binary_search_tree & tree)
		requires requires (std::ostream & os, T t) {
			{ os << t } -> std::convertible_to <std::ostream &>;
		}
	{
		if (nullptr != tree.m_root) {
			os << * tree.m_root;
		}

		return os;
	}

public:
	template <typename U>
	requires std::convertible_to <U, T>
	void insert (U && value) {
		node_link link = get_link (value);
		if (nullptr == * link) {
			insert_at (link, std::forward <U> (value));
		}
	}

	template <typename VC>
	requires ValueComparable <VC>
	bool contains (const VC & value) const {
		const node * n = at (value);

		if (nullptr != n) {
			return true;
		}
		else {
			return false;
		}
	}

	template <typename VC>
	requires ValueComparable <VC>
	void remove (const VC & value) {
		node_link link = get_link (value);

		if (nullptr != * link) {
			remove_at (link);
		}
	}

	void dump_sorted (std::output_iterator <T> auto it) const {
		if (nullptr != m_root) {
			m_root->inorder_traverse (
				[&it] (const node * n, const node *) -> void {
					*it++ = n->data;
				}
			);
		}
	}

	void dump_invariant (std::output_iterator <T> auto it) const {
		if (nullptr != m_root) {
			m_root->level_order_traverse (
				[&it] (const node * n, const node *) -> void {
					*it++ = n->data;
				}
			);
		}
	}

	const node * find_min () const {
		return * get_link_to_leftmost (& m_root);
	}

	const node * find_max () const {
		return * get_link_to_rightmost (& m_root);
	}

	void make_empty () {
		if (nullptr != m_root) {
			delete m_root;
			m_root = nullptr;
			m_size = 0;
		}
	}

	[[nodiscard]] const node * root () const { return m_root; }
	[[nodiscard]] std::size_t size () const { return m_size; }
	[[nodiscard]] bool empty () const { return 0 == m_size; }

	[[nodiscard]] std::size_t internal_path_length () const {
		std::size_t s = 0;

		if (nullptr != m_root) {
			m_root->level_order_traverse ([& s] (const node *, const node *, std::size_t l) -> void {
				s += l;
			});
		}

		return s;
	}

protected:
	template <typename U>
	requires std::convertible_to <U, T>
	void insert_at (node_link link, U && value) {
		* link = new node (std::forward <U> (value)); // NOLINT(cppcoreguidelines-owning-memory)
		m_size++;
	}

	void remove_at (node_link link) {
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
			if constexpr (true == detail::KeepInvariant) {
				if constexpr (true == detail::RemovePreserveLeft) {
					node * left_rightmost = * get_link_to_rightmost (link_left);
					left_rightmost->right = * link_right;
					* link = * link_left;
				}
				else {
					node * right_leftmost = * get_link_to_leftmost (link_right);
					right_leftmost->left = * link_left;
					* link = * link_right;
				}
			}
			else {
				if constexpr (true == detail::RemovePreserveLeft) {
					node_link left_rightmost_link = get_link_to_rightmost (link_left);

					node * left_rightmost = * left_rightmost_link;
					left_rightmost->right = * link_right;

					if (left_rightmost_link != link_left) {
						* left_rightmost_link = left_rightmost->left;
						left_rightmost->left = * link_left;
					}

					* link = left_rightmost;
				}
				else {
					node_link right_leftmost_link = get_link_to_leftmost (link_right);

					node * right_leftmost = * right_leftmost_link;
					right_leftmost->left = * link_left;

					if (right_leftmost_link != link_right) {
						* right_leftmost_link = right_leftmost->right;
						right_leftmost->right = * link_right;
					}

					* link = right_leftmost;
				}
			}
		}

		to_remove->left = nullptr;
		to_remove->right = nullptr;
		delete to_remove; // NOLINT(cppcoreguidelines-owning-memory)
		m_size--;
	}

protected:
	template <typename VC>
	requires ValueComparable <VC>
	const node * at (const VC & value) const {
		return * get_link (value);
	}

	template <typename VC>
	requires ValueComparable <VC>
	node * at (const VC & value) {
		return * get_link (value);
	}

	template <typename VC>
	requires ValueComparable <VC>
	const_node_link get_link (const VC & value) const {
		const_node_link link = & m_root;

		while (nullptr != * link) { // NOLINT(altera-id-dependent-backward-branch)
			bool lt = less_than (value, (* link)->data);
			bool gt = less_than ((* link)->data, value);

			if (true == lt) {
				link = & (* link)->left;
			}
			else if (true == gt) {
				link = & (* link)->right;
			}
			else {
				break;
			}
		}

		return link;
	}

	template <typename U, typename VC>
	requires detail::template is_node_link_v <U> && ValueComparable <VC>
	static stack <U> get_link_stack (U link, const VC & value) {
		stack <U> link_stack;
		link_stack.push (link);

		while (nullptr != * link) { // NOLINT(altera-id-dependent-backward-branch)
			bool lt = less_than (value, (* link)->data);
			bool gt = less_than ((* link)->data, value);

			if (true == lt) {
				link = & (* link)->left;
				link_stack.push (link);
			}
			else if (true == gt) {
				link = & (* link)->right;
				link_stack.push (link);
			}
			else {
				break;
			}
		}

		return link_stack;
	}

	template <typename VC>
	requires ValueComparable <VC>
	node_link get_link (const VC & value) {
		return const_cast <node_link> (const_cast <const binary_search_tree *> (this)->get_link (value));
	}

	template <typename VC>
	requires ValueComparable <VC>
	stack <const_node_link> get_link_stack (const VC & value) const {
		return get_link_stack <const_node_link> (& m_root, value);
	}

	template <typename VC>
	requires ValueComparable <VC>
	stack <node_link> get_link_stack (const VC & value) {
		return get_link_stack <node_link> (& m_root, value);
	}

	stack <const_node_link> get_link_stack_to_leftmost (const_node_link link) const {
		return node::template get_link_stack_to_leftmost <const_node_link> (link);
	}

	stack <node_link> get_link_stack_to_leftmost (node_link link) {
		return node::template get_link_stack_to_leftmost <node_link> (link);
	}

	stack <const_node_link> get_link_stack_to_rightmost (const_node_link link) const {
		return node::template get_link_stack_to_rightmost <const_node_link> (link);
	}

	stack <node_link> get_link_stack_to_rightmost (node_link link) {
		return node::template get_link_stack_to_rightmost <node_link> (link);
	}

	const_node_link get_link_to_leftmost (const_node_link link) const {
		return node::template get_link_to_leftmost <const_node_link> (link);
	}

	node_link get_link_to_leftmost (node_link link) {
		return node::template get_link_to_leftmost <node_link> (link);
	}

	const_node_link get_link_to_rightmost (const_node_link link) const {
		return node::template get_link_to_rightmost <const_node_link> (link);
	}

	node_link get_link_to_rightmost (node_link link) {
		return node::template get_link_to_rightmost <node_link> (link);
	}

protected:
	node * m_root;
	std::size_t m_size;
};

} // end namespace tree

# endif // BINARY_SEARCH_TREE
