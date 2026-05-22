# ifndef MAP_H
# define MAP_H

# include <cstddef>

# include <concepts>
# include <functional>
# include <initializer_list>
# include <iterator>
# include <ostream>
# include <stdexcept>
# include <tuple>
# include <type_traits>
# include <utility>

# include <tree/config.hpp>

# include <tree/splay_tree.hpp>

namespace tree { // NOLINT(modernize-concat-nested-namespaces)

namespace detail {
template <
	typename Kt,
	typename Vt
>
struct map_node_data_ {
	// static_assert (false == requires (Data d) { d.key; }, "the 'key' member of Data is reserved");
	// static_assert (false == requires (Data d) { d.value; }, "the 'value' member of Data is reserved");

	Kt key;
	Vt value;

	template <typename Kt_, typename Vt_>
	requires std::is_constructible_v <Kt, Kt_> && std::is_constructible_v <Vt, Vt_>
	explicit map_node_data_ (Kt_ && key, Vt_ && value)
		: key (std::forward <Kt_> (key)), value (std::forward <Vt_> (value))
	{}

	friend std::ostream & operator<< (std::ostream & os, const map_node_data_ & data) {
		os << '{' << data.key << ", " << data.value << '}';
		return os;
	}

	template <std::size_t I>
	requires (2 > I)
	friend constexpr const std::tuple_element_t <I, map_node_data_> &
	get (const map_node_data_ & nd) noexcept
	{
		if constexpr (0 == I) {
			return nd.key;
		}
		else {
			return nd.value;
		}
	}

	template <std::size_t I>
	requires (2 > I)
	friend constexpr std::tuple_element_t <I, map_node_data_> &
	get (map_node_data_ & nd) noexcept
	{
		if constexpr (0 == I) {
			return nd.key;
		}
		else {
			return nd.value;
		}
	}

	template <std::size_t I>
	requires (2 > I)
	friend constexpr std::tuple_element_t <I, map_node_data_> &&
	get (map_node_data_ && nd) noexcept // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
	{
		if constexpr (0 == I) {
			return std::move (nd.key);
		}
		else {
			return std::move (nd.value);
		}
	}
};

template <
	typename Kt,
	typename Vt,
	typename Comparator,
	template <typename, typename> typename Container
>
struct map_detail_ {
	using node_data = map_node_data_ <Kt, Vt>;

	struct less_than {
		static constexpr Comparator comparator {};

		bool operator () (const node_data & first, const node_data & second) const {
			return comparator (first.key, second.key);
		}

		bool operator () (const Kt & first, const Kt & second) const {
			return comparator (first, second);
		}

		bool operator () (const Kt & key, const node_data & node) const {
			return comparator (key, node.key);
		}

		bool operator () (const node_data & node, const Kt & key) const {
			return comparator (node.key, key);
		}
	};

	using tree = Container <node_data, less_than>;

	// static_assert (is_tree_v <tree>, "Container of map isn't a tree");
};

} // end namespace detail

} // end namespace tree



namespace std {

template <typename Kt, typename Vt>
// NOLINTNEXTLINE(bugprone-std-namespace-modification,cert-dcl58-cpp)
struct tuple_size <tree::detail::map_node_data_ <Kt, Vt>>
	: std::integral_constant <std::size_t, 2> {};

template <typename Kt, typename Vt>
// NOLINTNEXTLINE(bugprone-std-namespace-modification,cert-dcl58-cpp)
struct tuple_element <0, tree::detail::map_node_data_ <Kt, Vt>> {
	using type = Kt;
};

template <typename Kt, typename Vt>
// NOLINTNEXTLINE(bugprone-std-namespace-modification,cert-dcl58-cpp)
struct tuple_element <1, tree::detail::map_node_data_ <Kt, Vt>> {
	using type = Vt;
};

} // end namespace std



namespace tree {
// NOTE: binary_tree_node and trees are implemented such a way that iterators
// are second class cityzens here and get invalidated after the container gets
// mutated. Instead of iterators all underlying implementation uses node** and
// stack<node**>. To make iterators first class cityzens first nodes must
// carry parrent node pointers. All code using nodes must be carefully rewriten.

template <
	typename Kt,
	typename Vt,
	typename Comparator = std::less <Kt>,
	template <typename, typename> typename Container = splay_tree
>
requires std::strict_weak_order <Comparator, Kt, Kt>
struct map : protected detail::map_detail_ <Kt, Vt, Comparator, Container>::tree {
protected:
	using detail = detail::map_detail_ <Kt, Vt, Comparator, Container>;
public:
	using tree = detail::tree;
	using node = tree::node;
	using node_link = tree::node_link;
	using const_node_link = tree::const_node_link;
	using node_data = detail::node_data;
	using key_type = Kt;
	using value_type = Vt;
	using tree::less_than;
	using iterator = tree::iterator;
	using const_iterator = tree::const_iterator;

public:
	using tree::dump_invariant;
	using tree::dump_sorted;
	using tree::make_empty;
	using tree::root;
	using tree::size;
	using tree::empty;
	using tree::internal_path_length;
	// using tree::insert;
	using tree::remove;
	using tree::contains;

	using tree::begin;
	using tree::cbegin;
	using tree::end;
	using tree::cend;
	using tree::find;

	map () = default;

	template <std::convertible_to <Kt> Kt_, std::convertible_to <Vt> Vt_>
	map (std::initializer_list <std::pair <Kt_, Vt_>> list) {
		for (const auto & [k, v] : list) {
			insert (k, v);
		}
	}

	bool operator== (const map & other) const {
		return
			   static_cast <const tree &> (* this)
			== static_cast <const tree &> (other)
		;
	}

	void dump_sorted (std::output_iterator <std::pair <Kt, Vt>> auto it) const {
		if (nullptr != this->m_root) {
			this->m_root->inorder_traverse (
				[&it] (const node * n, const node *) -> void {
					*it++ = { n->key, n->value };
				}
			);
		}
	}

	operator vector <std::pair <Kt, Vt>> () const {
		vector <std::pair <Kt, Vt>> v;
		v.reserve (size ());

		dump_sorted (std::back_inserter (v));

		return v;
	}

	friend std::ostream & operator<< (std::ostream & os, const map & tree)
		requires requires (std::ostream & os, Kt k, Vt v) {
			{ os << k } -> std::convertible_to <std::ostream &>;
			{ os << v } -> std::convertible_to <std::ostream &>;
		}
	{
		return os << static_cast <const map::tree &> (tree);
	}

public:
	template <typename Kt_, typename Vt_>
	requires std::convertible_to <Kt_, Kt> && std::convertible_to <Vt_, Vt>
	bool insert (Kt_ && key, Vt_ && value) {
		bool inserting = false;

		if (false == contains (key)) {
			this->tree::insert (node_data (std::forward <Kt_> (key), std::forward <Vt_> (value)));
			inserting = true;
		}	// after move constructing value we can't use it to get an iterator,
			// so insert should return one.

		return inserting;
	}

	template <typename Kt_, typename Vt_>
	requires std::convertible_to <Kt_, Kt> && std::convertible_to <Vt_, Vt>
	bool insert (const std::pair <Kt_, Vt_> & value) {
		return insert (value.first, value.second);
	}

	template <typename Kt_, typename Vt_>
	requires std::convertible_to <Kt_, Kt> && std::convertible_to <Vt_, Vt>
	bool insert (std::pair <Kt_, Vt_> && value) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
		return insert (std::move (value.first), std::move (value.second));
	}

	Vt & at (const Kt & key) {
		if (false == this->contains (key)) {
			this->insert (key, Vt {});
		}
		return static_cast <node_data &> ((* tree::get_link (key))->data).value;
	}

	const Vt & at (const Kt & key) const {
		if (false == this->contains (key)) {
			throw std::out_of_range ("map::at");
		}
		return static_cast <const node_data &> ((* tree::get_link (key))->data).value;
	}

	Vt & operator[] (const Kt & key) {
		return at (key);
	}

	const Vt & operator[] (const Kt & key) const {
		return at (key);
	}
};

} // end namespace tree

# endif // MAP_H
