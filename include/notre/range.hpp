#ifndef NOTRE_V2__NOTRE__RANGE__HPP
#define NOTRE_V2__NOTRE__RANGE__HPP

#include "iterators.hpp"

namespace notre {

template <typename> constexpr bool is_range = false;

template <typename BeginIterator, typename EndIterator, typename RE, typename ResultIterator = BeginIterator> struct regex_range {
	BeginIterator _begin;
	EndIterator _end;
	
	constexpr NOTRE_FORCE_INLINE regex_range(BeginIterator begin, EndIterator end) noexcept: _begin{begin}, _end{end} { }
	
	constexpr NOTRE_FORCE_INLINE auto begin() const noexcept {
		return regex_iterator<BeginIterator, EndIterator, RE, ResultIterator>(_begin, _end);
	}
	constexpr NOTRE_FORCE_INLINE auto end() const noexcept {
		return regex_end_iterator{};
	}
};

template <typename... Ts> constexpr bool is_range<regex_range<Ts...>> = true;

template <typename BeginIterator, typename EndIterator, typename RE, typename ResultIterator = BeginIterator> struct regex_split_range {
	BeginIterator _begin;
	EndIterator _end;
	
	constexpr NOTRE_FORCE_INLINE regex_split_range(BeginIterator begin, EndIterator end) noexcept: _begin{begin}, _end{end} { }
	
	constexpr NOTRE_FORCE_INLINE auto begin() const noexcept {
		return regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator>(_begin, _end);
	}
	constexpr NOTRE_FORCE_INLINE auto end() const noexcept {
		return regex_end_iterator{};
	}
};

template <typename... Ts> constexpr bool is_range<regex_split_range<Ts...>> = true;

template <typename Range, typename RE> struct multi_subject_range {
	struct end_iterator { };
	
	using first_type = decltype(std::declval<Range>().begin());
	using last_type = decltype(std::declval<Range>().end());
	
	struct iterator {
		using value_type = decltype(RE::exec(std::declval<typename std::iterator_traits<first_type>::value_type>()));
		using iterator_category = std::forward_iterator_tag;
		using reference = const value_type &;
		using pointer = const value_type *;
		using difference_type = int;
		
		first_type first{};
		last_type last{};
		value_type current_result{};
		
		constexpr NOTRE_FORCE_INLINE iterator() noexcept = default;
		constexpr NOTRE_FORCE_INLINE iterator(first_type f, last_type l) noexcept: first{f}, last{l}, current_result{find_first()} { }
		
		constexpr NOTRE_FORCE_INLINE value_type find_first() noexcept {
			while (first != last) {
				if (auto res = RE::exec(*first)) return res;
				else ++first;
			}
			return {};
		}
		
		constexpr NOTRE_FORCE_INLINE reference operator*() const noexcept {
			return current_result;
		}
		
		constexpr NOTRE_FORCE_INLINE pointer operator->() const noexcept {
			return &current_result;
		}
		
		constexpr NOTRE_FORCE_INLINE iterator & operator++() noexcept {
			++first;
			current_result = find_first();
			return *this;
		}
		constexpr NOTRE_FORCE_INLINE iterator operator++(int) noexcept {
			auto previous = *this;
			this->operator++();
			return previous;
		}
		
		friend constexpr NOTRE_FORCE_INLINE bool operator==(const iterator & left, const iterator & right) noexcept {
			return left.first == right.first;
		}
		friend constexpr NOTRE_FORCE_INLINE bool operator!=(const iterator & left, const iterator & right) noexcept {
			return !(left.first == right.first);
		}
		friend constexpr NOTRE_FORCE_INLINE bool operator<(const iterator & left, const iterator & right) noexcept {
			return left.first < right.first;
		}
		friend constexpr NOTRE_FORCE_INLINE bool operator>(const iterator & left, const iterator & right) noexcept {
			return left.first > right.first;
		}
		friend constexpr NOTRE_FORCE_INLINE bool operator<=(const iterator & left, const iterator & right) noexcept {
			return left.first <= right.first;
		}
		friend constexpr NOTRE_FORCE_INLINE bool operator>=(const iterator & left, const iterator & right) noexcept {
			return left.first >= right.first;
		}
		friend constexpr NOTRE_FORCE_INLINE bool operator==(const iterator & left, end_iterator) noexcept {
			return left.first == left.last;
		}
		friend constexpr NOTRE_FORCE_INLINE bool operator==(end_iterator, const iterator & right) noexcept {
			return right.first == right.last;
		}
		friend constexpr NOTRE_FORCE_INLINE bool operator!=(const iterator & left, end_iterator) noexcept {
			return left.first != left.last;
		}
		friend constexpr NOTRE_FORCE_INLINE bool operator!=(end_iterator, const iterator & right) noexcept {
			return right.first == right.last;
		}
	};
	
	Range range{};
	
	constexpr NOTRE_FORCE_INLINE multi_subject_range() noexcept = default;
	constexpr NOTRE_FORCE_INLINE multi_subject_range(Range r) noexcept: range{r} { }
	
	constexpr NOTRE_FORCE_INLINE auto begin() const noexcept {
		return iterator{range.begin(), range.end()};
	}
	constexpr NOTRE_FORCE_INLINE auto end() const noexcept {
		return end_iterator{};
	}
};

// this is not regex range!
template <typename... Ts> constexpr bool is_range<multi_subject_range<Ts...>> = true;


}

#if defined __cpp_lib_ranges && __cpp_lib_ranges >= 201911
namespace std::ranges {

	template <typename... Ts> inline constexpr bool enable_borrowed_range<::notre::regex_range<Ts...>> = true;
	template <typename... Ts> inline constexpr bool enable_borrowed_range<::notre::regex_split_range<Ts...>> = true;
	template <typename Range, typename RE> inline constexpr bool enable_borrowed_range<::notre::multi_subject_range<Range, RE>> = enable_borrowed_range<Range>;
	template <typename Range, typename RE> inline constexpr bool enable_view<::notre::multi_subject_range<Range, RE>> = true;

}
#endif 

#endif
