#ifndef CTLL__MAP__HPP
#define CTLL__MAP__HPP

#ifndef CTLL_IN_A_MODULE
#include <array>
#include <cstddef>
#include <string_view>
#include <utility>
#endif

#include "utilities.hpp"

// A constexpr fixed-size lookup map: an aggregate over a std::array of
// key/value pairs with linear search, after the design popularized by
// Jason Turner's C++ Weekly (see
// https://xuhuisun.com/post/c++-weekly-2-constexpr-map/). Intended for
// small compile-time tables - in the library it will map callout names to
// their ctll::function implementations.
//
// The deduction guides normalize character-pointer keys to
// std::basic_string_view, so ctll::map{std::pair{"name", value}, ...}
// compares keys by content; raw pointer keys would compare by address.
//
// All entries exist from construction (size() is the capacity); values
// remain assignable through find/at/operator[], keys do not. Unlike
// std::map, a missing key in at()/operator[] is not an exception (this
// library is exception free) but undefined behaviour - in a constant
// expression it fails to compile. Use find() or contains() when the key
// may be absent. Whether every key is distinct can be checked with
// has_unique_keys() in a static_assert.

namespace ctll {

CTLL_EXPORT template <typename Key, typename Value, size_t Size> struct map {
	using key_type = Key;
	using mapped_type = Value;
	using value_type = std::pair<Key, Value>;
	using iterator = value_type *;
	using const_iterator = const value_type *;

	std::array<value_type, Size> content;

	// element access

	constexpr iterator begin() noexcept {
		return content.data();
	}
	constexpr const_iterator begin() const noexcept {
		return content.data();
	}
	constexpr const_iterator cbegin() const noexcept {
		return begin();
	}
	constexpr iterator end() noexcept {
		return content.data() + Size;
	}
	constexpr const_iterator end() const noexcept {
		return content.data() + Size;
	}
	constexpr const_iterator cend() const noexcept {
		return end();
	}

	static constexpr size_t size() noexcept {
		return Size;
	}
	static constexpr bool empty() noexcept {
		return Size == 0;
	}

	// lookup (heterogeneous: anything equality-comparable with Key)

	template <typename K> constexpr iterator find(const K & key) noexcept {
		for (iterator it = begin(); it != end(); ++it) {
			if (it->first == key) {
				return it;
			}
		}
		return end();
	}
	template <typename K> constexpr const_iterator find(const K & key) const noexcept {
		for (const_iterator it = begin(); it != end(); ++it) {
			if (it->first == key) {
				return it;
			}
		}
		return end();
	}

	template <typename K> constexpr bool contains(const K & key) const noexcept {
		return find(key) != end();
	}

	template <typename K> constexpr size_t count(const K & key) const noexcept {
		size_t n = 0;
		for (const_iterator it = begin(); it != end(); ++it) {
			if (it->first == key) {
				++n;
			}
		}
		return n;
	}

	// a missing key is undefined behaviour (the dereference of end();
	// constant evaluation rejects it) - use find/contains when unsure
	template <typename K> constexpr Value & at(const K & key) noexcept {
		return find(key)->second;
	}
	template <typename K> constexpr const Value & at(const K & key) const noexcept {
		return find(key)->second;
	}

	// lookup only; a fixed-size map cannot insert, so unlike std::map
	// operator[] requires the key to exist (same contract as at)
	template <typename K> constexpr Value & operator[](const K & key) noexcept {
		return at(key);
	}
	template <typename K> constexpr const Value & operator[](const K & key) const noexcept {
		return at(key);
	}

	// registration-table validation, e.g. static_assert(m.has_unique_keys())
	constexpr bool has_unique_keys() const noexcept {
		for (const_iterator a = begin(); a != end(); ++a) {
			for (const_iterator b = a + 1; b != end(); ++b) {
				if (a->first == b->first) {
					return false;
				}
			}
		}
		return true;
	}

	friend constexpr bool operator==(const map & lhs, const map & rhs) {
		for (size_t i = 0; i != Size; ++i) {
			if (!(lhs.content[i] == rhs.content[i])) {
				return false;
			}
		}
		return true;
	}
	friend constexpr bool operator!=(const map & lhs, const map & rhs) {
		return !(lhs == rhs);
	}
};

// character-pointer keys become string views so keys compare by content
template <typename Key> struct map_key_normalize { using type = Key; };
template <> struct map_key_normalize<char *> { using type = std::string_view; };
template <> struct map_key_normalize<const char *> { using type = std::string_view; };
template <> struct map_key_normalize<wchar_t *> { using type = std::wstring_view; };
template <> struct map_key_normalize<const wchar_t *> { using type = std::wstring_view; };
#if defined(__cpp_char8_t)
template <> struct map_key_normalize<char8_t *> { using type = std::u8string_view; };
template <> struct map_key_normalize<const char8_t *> { using type = std::u8string_view; };
#endif
template <> struct map_key_normalize<char16_t *> { using type = std::u16string_view; };
template <> struct map_key_normalize<const char16_t *> { using type = std::u16string_view; };
template <> struct map_key_normalize<char32_t *> { using type = std::u32string_view; };
template <> struct map_key_normalize<const char32_t *> { using type = std::u32string_view; };

template <typename Key> using map_key_normalize_t = typename map_key_normalize<Key>::type;

// deduction from a flat list of pairs:
//   ctll::map{std::pair{"red"sv, 1}, std::pair{"green"sv, 2}}
CTLL_EXPORT template <typename Key, typename Value, typename... Rest> map(std::pair<Key, Value>, Rest...) -> map<map_key_normalize_t<Key>, Value, 1 + sizeof...(Rest)>;

// deduction from a prebuilt array of pairs (the reference's usage style)
CTLL_EXPORT template <typename Key, typename Value, size_t Size> map(std::array<std::pair<Key, Value>, Size>) -> map<Key, Value, Size>;

}

#endif
