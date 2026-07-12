#ifndef NOTRE_V2__NOTRE__ITERATOR__HPP
#define NOTRE_V2__NOTRE__ITERATOR__HPP

#include "literals.hpp"
#include "wrapper.hpp"
#ifndef NOTRE_IN_A_MODULE
#include <cstddef>
#endif

namespace notre {

// TODO make proper iterator traits here

struct regex_end_iterator {
	constexpr regex_end_iterator() noexcept { }
};

template <typename BeginIterator, typename EndIterator, typename RE, typename ResultIterator = BeginIterator> struct regex_iterator {
	using value_type = decltype(RE::template exec_with_result_iterator<ResultIterator>(std::declval<BeginIterator>(), std::declval<EndIterator>()));
	using iterator_category = std::forward_iterator_tag;
	using pointer = void;
	using reference = const value_type &;
	using difference_type = int;
	
	BeginIterator orig_begin{};
	BeginIterator current{};
	EndIterator end{};
	value_type current_match{};
	
	constexpr NOTRE_FORCE_INLINE regex_iterator() noexcept = default;
	constexpr NOTRE_FORCE_INLINE regex_iterator(const regex_iterator &) noexcept = default;
	constexpr NOTRE_FORCE_INLINE regex_iterator(regex_iterator &&) noexcept = default;

	constexpr NOTRE_FORCE_INLINE regex_iterator(BeginIterator begin, EndIterator last) noexcept: orig_begin{begin}, current{begin}, end{last}, current_match{RE::template exec_with_result_iterator<ResultIterator>(current, last)} {
		if (current_match) {
			current = current_match.template get<0>().end();
		}
	}
	
	constexpr NOTRE_FORCE_INLINE regex_iterator & operator=(const regex_iterator &) noexcept = default;
	constexpr NOTRE_FORCE_INLINE regex_iterator & operator=(regex_iterator &&) noexcept = default;
	
	constexpr NOTRE_FORCE_INLINE const value_type & operator*() const noexcept {
		return current_match;
	}
	constexpr NOTRE_FORCE_INLINE regex_iterator & operator++() noexcept {
		if (current == end) {
			current_match = decltype(current_match){};
			return *this;
		}
		
		current_match = RE::template exec_with_result_iterator<ResultIterator>(orig_begin, current, end);
		
		if (current_match) {
			current = current_match.template get<0>().end();
		}
		return *this;
	}
	constexpr NOTRE_FORCE_INLINE regex_iterator operator++(int) noexcept {
		auto previous = *this;
		this->operator++();
		return previous;
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator==(const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return left.current == right.current;
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator!=(const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return !(left.current == right.current);
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator<(const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return left.current < right.current;
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator>(const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return left.current > right.current;
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator<=(const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return left.current <= right.current;
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator>=(const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return left.current >= right.current;
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator==(const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, regex_end_iterator) noexcept {
		return !bool(left.current_match);
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator==(regex_end_iterator, const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return !bool(right.current_match);
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator!=(const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, regex_end_iterator) noexcept {
		return bool(left.current_match);
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator!=(regex_end_iterator, const regex_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return bool(right.current_match);
	}
};

template <typename BeginIterator, typename EndIterator, typename RE, typename ResultIterator = BeginIterator> struct regex_split_iterator {
	using value_type = decltype(RE::template exec_with_result_iterator<ResultIterator>(std::declval<BeginIterator>(), std::declval<EndIterator>()));
	using iterator_category = std::forward_iterator_tag;
	using pointer = void;
	using reference = const value_type &;
	using difference_type = int;

	BeginIterator orig_begin{};
	BeginIterator current{};
	EndIterator end{};
	value_type current_match{};
	bool last_match{false};

	constexpr NOTRE_FORCE_INLINE void modify_match() {
		auto tmp = current_match.template get<0>().end();
		current_match.set_end_mark(current_match.template get<0>().begin());
		current_match.set_start_mark(current);
		current = tmp;
	}
	
	constexpr NOTRE_FORCE_INLINE void match_rest() {
		// the end is there set by search_method
		current_match.set_start_mark(current);
		current_match.matched();
		current = current_match.template get<0>().end();
		last_match = true;
	}

	constexpr NOTRE_FORCE_INLINE regex_split_iterator() noexcept = default;
	constexpr NOTRE_FORCE_INLINE regex_split_iterator(const regex_split_iterator &) noexcept = default;
	constexpr NOTRE_FORCE_INLINE regex_split_iterator(regex_split_iterator &&) noexcept = default;

	constexpr NOTRE_FORCE_INLINE regex_split_iterator(BeginIterator begin, EndIterator last) noexcept: orig_begin{begin}, current{begin}, end{last}, current_match{RE::template exec_with_result_iterator<ResultIterator>(current, last)} {
		if (current_match) {
			modify_match();
		} else {
			match_rest();
		}
	}
	
	constexpr NOTRE_FORCE_INLINE regex_split_iterator & operator=(const regex_split_iterator &) noexcept = default;
	constexpr NOTRE_FORCE_INLINE regex_split_iterator & operator=(regex_split_iterator &&) noexcept = default;
	
	constexpr NOTRE_FORCE_INLINE const value_type & operator*() const noexcept {
		return current_match;
	}
	constexpr NOTRE_FORCE_INLINE regex_split_iterator & operator++() noexcept {
		if (current == end && last_match) {
			current_match = decltype(current_match){};
			return *this;
		}
	
		current_match = RE::template exec_with_result_iterator<ResultIterator>(orig_begin, current, end);
	
		if (current_match) {
			modify_match();
		} else {
			match_rest();
		}
		return *this;
	}
	constexpr NOTRE_FORCE_INLINE regex_split_iterator operator++(int) noexcept {
		auto previous = *this;
		this->operator++();
		return previous;
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator==(const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return left.current == right.current;
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator!=(const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return !(left.current == right.current);
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator<(const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return left.current < right.current;
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator>(const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return left.current > right.current;
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator<=(const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return left.current <= right.current;
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator>=(const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return left.current >= right.current;
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator==(const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, regex_end_iterator) noexcept {
		return !bool(left.current_match);
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator==(regex_end_iterator, const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return !bool(right.current_match);
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator!=(const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & left, regex_end_iterator) noexcept {
		return bool(left.current_match);
	}
	friend constexpr NOTRE_FORCE_INLINE bool operator!=(regex_end_iterator, const regex_split_iterator<BeginIterator, EndIterator, RE, ResultIterator> & right) noexcept {
		return bool(right.current_match);
	}
};

} // notre

#endif
