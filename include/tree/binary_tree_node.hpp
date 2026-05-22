# ifndef BINARY_TREE_NODE_H
# define BINARY_TREE_NODE_H

# include <concepts>
# include <cstddef>
# include <cstdint>
# include <functional>
# include <iostream>
# include <iterator>
# include <ostream>
# include <type_traits>
# include <utility>

# include <tree/config.hpp>

namespace tree {

template <typename T, std::equivalence_relation <T, T> EqualTo = std::equal_to <T>>
struct binary_tree_node {
public:
	using link = binary_tree_node **;
	using const_link = const binary_tree_node * const *;
	using nptr = binary_tree_node *;
	using const_nptr = const binary_tree_node *;

	using node = binary_tree_node;

	template <typename U>
	static constexpr bool is_node_link_v =
		std::is_same_v <U, link> ||
		std::is_same_v <U, const_link>
	;
	template <typename U>
	static constexpr bool is_node_ptr_v =
		std::is_same_v <U, nptr> || std::is_same_v <U, const_nptr>
	;

	enum class iterator_algorithm : std::uint8_t {
		preorder, inorder, postorder, levelorder
	};

	template <typename NPT, iterator_algorithm Alg = iterator_algorithm::inorder>
	requires is_node_ptr_v <NPT>
	// FIXME: CRITICAL: 1. clang-tidy: class 'iterator_base' defines a copy constructor but does not define a destructor, a copy assignment operator, a move constructor or a move assignment operator [cppcoreguidelines-special-member-functions,hicpp-special-member-functions]
	class iterator_base {
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using iterator_category = std::bidirectional_iterator_tag;
		// cppcheck-suppress unusedStructMember
		static constexpr iterator_algorithm algorithm = Alg;

	protected:
		using _reference_t = std::conditional_t <std::is_same_v <NPT, const_nptr>, const T &, T &>;

	public:
		friend class iterator_base <nptr>;
		friend class iterator_base <const_nptr>;

		static iterator_base begin (NPT root) {
			stack <NPT> path;
			for (NPT n = root; nullptr != n; n = n->left) { // NOLINT(altera-id-dependent-backward-branch)
				path.push (n);
			}

			return iterator_base (root, std::move (path));
		}

		static iterator_base end (NPT root) {
			return iterator_base (root, {});
		}

		explicit iterator_base (NPT root, const stack <NPT> & link_stack)
			: m_root (root)
			, m_path (link_stack)
		{
			initialize_from_path ();
		}

		explicit iterator_base (NPT root, stack <NPT> && link_stack)
			: m_root (root)
			, m_path (std::move (link_stack))
		{
			initialize_from_path ();
		}

		iterator_base (const iterator_base & other)
			: m_root (other.m_root)
			, m_node (other.m_node)
			, m_path (other.m_path)
		{}

		template <
			typename NPT2,
			typename = std::enable_if_t < // NOLINT(modernize-use-constraints)
				false == std::is_same_v <NPT2, NPT> && ( // disable copy constructor hijacking
					true == std::is_same_v <NPT2, const_nptr> ||
					true == std::is_same_v <NPT2, nptr>
				)
			>
		>
		explicit iterator_base (const iterator_base <NPT2> & other)
			: m_root (other.m_root)
			, m_path (other.m_path)
		{
			m_path.push (other.m_node);
			initialize_from_path ();
		}

		_reference_t & operator* () {
			return m_node->data;
		}

		const _reference_t & operator* () const {
			return m_node->data;
		}

		template <
			typename It2,
			typename = std::enable_if_t < // NOLINT(modernize-use-constraints)
				true == std::is_same_v <It2, iterator_base <nptr>> ||
				true == std::is_same_v <It2, iterator_base <const_nptr>>
			>
		>
		bool operator== (const It2 & other) const {
			return m_node == other.m_node;
		}

		iterator_base & operator++ () {
			if constexpr (iterator_algorithm::inorder == Alg) {
				if (nullptr != m_node->right) { // don't increment at end
					m_path.push (m_node);
					m_node = m_node->right;

					while (nullptr != m_node->left) { // NOLINT(altera-id-dependent-backward-branch)
						m_path.push (m_node);
						m_node = m_node->left;
					}
				}
				else {
					bool found = false;
					const std::size_t itC = m_path.size ();

					for (std::size_t i = 0; i < itC; i++) { // NOLINT(altera-id-dependent-backward-branch)
						const NPT l = m_node;
						m_node = m_path.top ();
						m_path.pop ();

						if (l == m_node->left) {
							found = true;
							break;
						}
					}

					if (false == found) { // incremented to end
						m_node = nullptr;
						m_path = {};
					}
				}

				return * this;
			}
			else {
				static_assert (false, "not implemented iterator algorithm");
			}
		}

		iterator_base operator++ (int) {
			iterator_base old = * this;
			++(* this);
			return old;
		}

		iterator_base & operator-- () {
			if constexpr (iterator_algorithm::inorder == Alg) {
				if (nullptr != m_node) { // if not end
					if (nullptr != m_node->left) {
						m_path.push (m_node);
						m_node = m_node->left;

						while (nullptr != m_node->right) { // NOLINT(altera-id-dependent-backward-branch)
							m_path.push (m_node);
							m_node = m_node->right;
						}
					}
					else {
						bool found = false;
						std::size_t itC = m_path.size ();

						for (std::size_t i = 0; i < itC; i++) { // NOLINT(altera-id-dependent-backward-branch)
							const NPT l = m_node;
							m_node = m_path.top ();
							m_path.pop ();

							if (l == m_node->right) {
								found = true;
								break;
							}
						}

						if (false == found) { // decremented past begin. UB. resetting to end. why?
							m_node = nullptr;
							m_path.clear ();
						}
					}
				}
				else { // decrementing end
					m_path.clear ();
					for (NPT n = m_root; nullptr != n; n = n->right) { // NOLINT(altera-id-dependent-backward-branch)
						m_path.push (n);
					}

					initialize_from_path ();
				}

				return * this;
			}
			else {
				static_assert (false, "not implemented iterator algorithm");
			}
		}

		iterator_base operator-- (int) {
			iterator_base old = * this;
			--(* this);
			return old;
		}

		NPT base () {
			return m_node;
		}

	private:
		void initialize_from_path () {
			if (false == m_path.empty ()) {
				m_node = m_path.top ();
				m_path.pop ();
			}
			else {
				m_node = nullptr;
			}
		}

	private:
		const NPT m_root; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
		NPT m_node;
		stack <NPT> m_path;
	};

public:
	using equivalence_relation = EqualTo;
	static constexpr equivalence_relation equal_to {};

	using iterator = iterator_base <nptr>;
	using const_iterator = iterator_base <const_nptr>;

	iterator begin () { return iterator::begin (this); }
	iterator end () { return iterator::end (this); }
	const_iterator cbegin () const { return const_iterator::begin (this); }
	const_iterator cend () const { return const_iterator::end (this); }
	const_iterator begin () const { return const_iterator::begin (this); }
	const_iterator end () const { return const_iterator::end (this); }

public:
	T data;
	binary_tree_node * left;
	binary_tree_node * right;

public:
	explicit binary_tree_node (const T & data, binary_tree_node * left = nullptr, binary_tree_node * right = nullptr)
		: data (data), left (left), right (right)
	{}

	explicit binary_tree_node (T && data, binary_tree_node * left = nullptr, binary_tree_node * right = nullptr)
		: data (std::move (data)), left (left), right (right)
	{}

public:
	binary_tree_node (const binary_tree_node & other)
		: data (other.data)
		, left (nullptr)
		, right (nullptr)
	{
		queue <binary_tree_node *> our;
		our.push (this);
		queue <const binary_tree_node *> their;
		their.push (& other);

		while (false == their.empty ()) {
			binary_tree_node * o = our.front ();
			our.pop ();
			const binary_tree_node * t = their.front ();
			their.pop ();

			if (t->left != nullptr) {
				o->left = new binary_tree_node (t->left->data); // NOLINT(cppcoreguidelines-owning-memory)
				our.push (o->left);
				their.push (t->left);
			}

			if (nullptr != t->right) {
				o->right = new binary_tree_node (t->right->data); // NOLINT(cppcoreguidelines-owning-memory)
				our.push (o->right);
				their.push (t->right);
			}
		}
	}

	binary_tree_node & operator= (const binary_tree_node & other) {
		if (this != & other) {
			binary_tree_node copy (other);
			std::swap (* this, copy);
		}
		return * this;
	}

	binary_tree_node (binary_tree_node && other)
		noexcept (std::is_nothrow_move_constructible_v <T>)
		: data (std::move (other.data))
		, left (other.left)
		, right (other.right)
	{
		other.left = nullptr;
		other.right = nullptr;
	}

	binary_tree_node & operator= (binary_tree_node && other)
		noexcept (std::is_nothrow_swappable_v <T>)
	{
		if (this != & other) {
			std::swap (data, other.data);
			std::swap (left, other.left);
			std::swap (right, other.right);
		}

		return * this;
	}

	~binary_tree_node () {
		queue <binary_tree_node *> to_delete;

		if (nullptr != left) {
			to_delete.push (left);
			left = nullptr;
		}
		if (nullptr != right) {
			to_delete.push (right);
			right = nullptr;
		}

		while (false == to_delete.empty ()) {
			binary_tree_node * n = to_delete.front ();
			to_delete.pop ();
			if (nullptr != n->left) {
				to_delete.push (n->left);
				n->left = nullptr;
			}
			if (nullptr != n->right) {
				to_delete.push (n->right);
				n->right = nullptr;
			}

			delete n; // NOLINT(cppcoreguidelines-owning-memory)
		}
	}

public:
	bool operator== (const binary_tree_node & other) const {
		stack <const binary_tree_node *> our;
		our.push (this);
		stack <const binary_tree_node *> their;
		their.push (& other);

		bool equal = true;

		while (false == our.empty ()) {
			const binary_tree_node * o = our.top ();
			our.pop ();
			const binary_tree_node * t = their.top ();
			their.pop ();

			if (false == equal_to (o->data, t->data)) {
				equal = false;
				break;
			}

			const binary_tree_node * o_l = o->left;
			const binary_tree_node * o_r = o->right;
			const binary_tree_node * t_l = t->left;
			const binary_tree_node * t_r = t->right;

			const bool o_l_null = o_l == nullptr;
			const bool o_r_null = o_r == nullptr;
			const bool t_l_null = t_l == nullptr;
			const bool t_r_null = t_r == nullptr;

			if (o_l_null != t_l_null || o_r_null != t_r_null) {
				equal = false;
				break;
			}

			if (false == o_l_null) {
				our.push (o_l);
				their.push (t_l);
			}

			if (false == o_r_null) {
				our.push (o_r);
				their.push (t_r);
			}
		}

		return equal;
	}

	friend std::ostream & operator<< (std::ostream & os, const binary_tree_node & node)
		requires requires (std::ostream os, T t) {
			{ os << t } -> std::convertible_to <std::ostream &>;
		}
	{
		node.preorder_traverse (
			[path = stack <const binary_tree_node *> (), &os]
			(const binary_tree_node * node, const binary_tree_node * parent) mutable -> void {
				if (nullptr != parent) {
					while (path.top () != parent) {
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
	template <typename F>
	requires std::invocable <F &, binary_tree_node *, binary_tree_node *>
	void preorder_traverse (F && func) {
		preorder_traverse (this, std::forward <F> (func));
	}

	template <typename F>
	requires std::invocable <F &, binary_tree_node *, binary_tree_node *>
	void inorder_traverse (F && func) {
		inorder_traverse (this, std::forward <F> (func));
	}

	template <typename F>
	requires std::invocable <F &, binary_tree_node *, binary_tree_node *>
	void postorder_traverse (F && func) {
		postorder_traverse (this, std::forward <F> (func));
	}

	template <typename F>
	requires std::invocable <F &, binary_tree_node *, binary_tree_node *>
	void level_order_traverse (F && func) {
		level_order_traverse (this, std::forward <F> (func));
	}

	template <typename F>
	requires std::invocable <F &, binary_tree_node *, binary_tree_node *, std::size_t>
	void level_order_traverse (F && func) {
		level_order_traverse (this, std::forward <F> (func));
	}

	template <typename F>
	requires std::invocable <F &, const binary_tree_node *, const binary_tree_node *>
	void preorder_traverse (F && func) const {
		preorder_traverse (this, std::forward <F> (func));
	}

	template <typename F>
	requires std::invocable <F &, const binary_tree_node *, const binary_tree_node *>
	void inorder_traverse (F && func) const {
		inorder_traverse (this, std::forward <F> (func));
	}

	template <typename F>
	requires std::invocable <F &, const binary_tree_node *, const binary_tree_node *>
	void postorder_traverse (F && func) const {
		postorder_traverse (this, std::forward <F> (func));
	}

	template <typename F>
	requires std::invocable <F &, const binary_tree_node *, const binary_tree_node *>
	void level_order_traverse (F && func) const {
		level_order_traverse (this, std::forward <F> (func));
	}

	template <typename F>
	requires std::invocable <F &, const binary_tree_node *, const binary_tree_node *, std::size_t>
	void level_order_traverse (F && func) const {
		level_order_traverse (this, std::forward <F> (func));
	}

public:
	template <typename U>
	requires is_node_link_v <U>
	static U get_link_to_leftmost (U link) {
		if (nullptr != * link) {
			while (nullptr != (* link)->left) { // NOLINT(altera-id-dependent-backward-branch)
				link = & (* link)->left;
			}
		}

		return link;
	}

	template <typename U>
	requires is_node_link_v <U>
	static U get_link_to_rightmost (U link) {
		if (nullptr != * link) {
			while (nullptr != (* link)->right) { // NOLINT(altera-id-dependent-backward-branch)
				link = & (* link)->right;
			}
		}

		return link;
	}

	template <typename U>
	requires is_node_link_v <U>
	static stack <U> get_link_stack_to_leftmost (U link) {
		stack <U> link_stack;
		link_stack.push (link);

		if (nullptr != * link) {
			while (nullptr != (* link)->left) { // NOLINT(altera-id-dependent-backward-branch)
				link = & (* link)->left;
				link_stack.push (link);
			}
		}

		return link_stack;
	}

	template <typename U>
	requires is_node_link_v <U>
	static stack <U> get_link_stack_to_rightmost (U link) {
		stack <U> link_stack;
		link_stack.push (link);

		if (nullptr != * link) {
			while (nullptr != (* link)->right) { // NOLINT(altera-id-dependent-backward-branch)
				link = & (* link)->right;
				link_stack.push (link);
			}
		}

		return link_stack;
	}

private:
	template <typename U, typename F>
	requires
		   std::same_as <std::remove_cv_t <std::remove_pointer_t <U>>, binary_tree_node>
		&& std::invocable <F &, U, U>
		// NOTE: const F & doesn't accept mutable lambdas
		// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
	static void preorder_traverse (U node, F && func) {
		stack <U> nodes;
		stack <U> parents;
		nodes.push (node);
		parents.push (nullptr);

		while (false == nodes.empty ()) {
			U n = nodes.top ();
			nodes.pop ();
			U p = parents.top ();
			parents.pop ();

			func (n, p);

			if (nullptr != n->right) {
				nodes.push (n->right);
				parents.push (n);
			}
			if (nullptr != n->left) {
				nodes.push (n->left);
				parents.push (n);
			}
		}
	}

	template <typename U, typename F>
	requires
		   std::same_as <std::remove_cv_t <std::remove_pointer_t <U>>, binary_tree_node>
		&& std::invocable <F &, U, U>
		// NOTE: const F & doesn't accept mutable lambdas
		// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
	static void inorder_traverse (U node, F && func, U parent = nullptr) {
		if (nullptr != node->left) {
			inorder_traverse (static_cast <U> (node->left), func, node);
		}

		func (node, parent);

		if (nullptr != node->right) {
			inorder_traverse (static_cast <U> (node->right), func, node);
		}
	}

	template <typename U, typename F>
	requires
		   std::same_as <std::remove_cv_t <std::remove_pointer_t <U>>, binary_tree_node>
		&& std::invocable <F &, U, U>
		// NOTE: const F & doesn't accept mutable lambdas
		// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
	static void postorder_traverse (U node, F && func, U parent = nullptr) {
		if (nullptr != node->left) {
			postorder_traverse (static_cast <U> (node->left), func, node);
		}

		if (nullptr != node->right) {
			postorder_traverse (static_cast <U> (node->right), func, node);
		}

		func (node, parent);
	}

	template <typename U, typename F>
	requires
		   std::same_as <std::remove_cv_t <std::remove_pointer_t <U>>, binary_tree_node>
		&& std::invocable <F &, U, U>
		// NOTE: const F & doesn't accept mutable lambdas
		// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
	static void level_order_traverse (U node, F && func, U parent = nullptr) {
		queue <U> nodes;
		nodes.push (node);
		queue <U> parents;
		parents.push (parent);

		while (false == nodes.empty ()) {
			U n = nodes.front ();
			nodes.pop ();
			U p = parents.front ();
			parents.pop ();

			if (nullptr != n->left) {
				nodes.push (n->left);
				parents.push (n);
			}

			if (nullptr != n->right) {
				nodes.push (n->right);
				parents.push (n);
			}

			func (n, p);
		}
	}

	template <typename U, typename F>
	requires
		   std::same_as <std::remove_cv_t <std::remove_pointer_t <U>>, binary_tree_node>
		&& std::invocable <F &, U, U, std::size_t>
		// NOTE: const F & doesn't accept mutable lambdas
		// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
	static void level_order_traverse (U node, F && func, U parent = nullptr) {
		queue <U> nodes, nodes_next;
		nodes.push (node);
		queue <U> parents, parents_next;
		parents.push (parent);

		std::size_t level = 0;

		while (true) {
			while (false == nodes.empty ()) { // NOLINT(altera-id-dependent-backward-branch)
				U n = nodes.front ();
				nodes.pop ();
				U p = parents.front ();
				parents.pop ();

				if (nullptr != n->left) {
					nodes_next.push (n->left);
					parents_next.push (n);
				}

				if (nullptr != n->right) {
					nodes_next.push (n->right);
					parents_next.push (n);
				}

				func (n, p, level);
			}
			if (false == nodes_next.empty ()) {
				nodes = std::move (nodes_next);
				parents = std::move (parents_next);
				nodes_next = {};
				parents_next = {};
				level++;
			}
			else {
				break;
			}
		}
	}
};

} // end namespace tree

# endif // BINARY_TREE_NODE_H
