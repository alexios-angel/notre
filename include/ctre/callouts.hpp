#ifndef CTRE__CALLOUTS__HPP
#define CTRE__CALLOUTS__HPP

#include "../ctll/callout.hpp"
#include "atoms.hpp"
#include "atoms_unicode.hpp"
#include "flags_and_modes.hpp"
#include "id.hpp"
#include "return_type.hpp"
#include "utility.hpp"
#ifndef CTRE_IN_A_MODULE
#include <array>
#include <cstddef>
#include <iterator>
#include <string_view>
#include <type_traits>
#endif

// Regex callouts: (?Cn) and (?C"name") are zero-width observation points
// inside a pattern. Implementations are attached with the
// ctre::with_callouts modifier in one of two forms.
//
// Inline entries (C++20), one ctll::callout per name or number:
//
//   ctre::match<"a(?C'go')b(?C2)", ctre::with_callouts<
//       ctll::callout<"go", [](const auto & c) { return c.position < 4; }>,
//       ctll::callout<2, [](const auto & c) { /* observe only */ }>
//   >>(input);
//
// Each entry is matched to its callout at compile time (numbers by their
// decimal spelling) and its callable is invoked directly - no lookup at
// match time. The callable receives a ctre::callout_data and may return
// ctre::callout_result, bool (true = proceed), or void (pure observer).
// Generic lambdas work with any subject character type.
//
// Handler type with a map, for callouts selected at match time:
//
//   struct handler {
//       static constexpr auto callouts() {
//           return ctll::map{std::pair{"go", ctll::function<
//               ctre::callout_result(const ctre::callout_data<char> &)>(...)}};
//       }
//   };
//   ctre::match<"a(?C'go')b", ctre::with_callouts<handler>>(input);
//
// Handler::callouts() is re-invoked per callout hit and dispatches by
// name through the map ((?C7) uses the key "7"; (?C) is "0").
//
// In both forms, callout_result::fail vetoes the current position and
// normal backtracking continues, like PCRE's nonzero callout return. A
// callout with no matching entry or key - or a pattern used without the
// modifier - is a no-op, like PCRE with no callout function set.
//
// The handler rides the pattern as a type: bind_callouts rewrites the
// parsed AST's callout markers into atoms carrying it (the same
// post-parse rewriting technique as resolve_subroutines).
//
// Restrictions (diagnosed with static_asserts): the subject must be a
// contiguous same-iterator range of a plain character type (no UTF-8
// range adaptor and no zero-terminated pointer subjects), and callouts
// inside lookbehinds are not supported (evaluation runs on reversed
// iterators there).

namespace ctre {

CTRE_EXPORT enum class callout_result {
	proceed,
	fail
};

// everything a callout sees; CharT is the subject's character type
CTRE_EXPORT template <typename CharT> struct callout_data {
	size_t number;                          // n of (?Cn), 0 for named callouts
	std::string_view name;                  // "name" of (?C"name"), decimal spelling for numbered
	std::basic_string_view<CharT> subject;  // the whole searched input
	size_t position;                        // code-unit offset of the matching position
	size_t match_start;                     // code-unit offset where this match attempt began
};

// compile-time rendering of a callout's lookup key

template <typename> struct callout_name_string;

template <auto... Str> struct callout_name_string<id<Str...>> {
	static constexpr char content[sizeof...(Str)]{static_cast<char>(Str)...};
	static constexpr std::string_view view{content, sizeof...(Str)};
};

constexpr size_t decimal_digit_count(size_t value) noexcept {
	size_t digits = 1;
	while (value >= 10) {
		value /= 10;
		++digits;
	}
	return digits;
}

template <size_t Number> struct callout_number_string {
	static constexpr size_t length = decimal_digit_count(Number);
	static constexpr std::array<char, length> compute() noexcept {
		std::array<char, length> out{};
		size_t value = Number;
		for (size_t i = length; i != 0; --i) {
			out[i - 1] = static_cast<char>('0' + value % 10);
			value /= 10;
		}
		return out;
	}
	static constexpr std::array<char, length> content = compute();
	static constexpr std::string_view view{content.data(), length};
};

// the first with_callouts<...> in a modifier pack is the callout token
template <typename... Modifiers> struct callout_token_from {
	using type = void;
};
template <typename... Handler, typename... Rest> struct callout_token_from<with_callouts<Handler...>, Rest...> {
	using type = with_callouts<Handler...>;
};
template <typename Head, typename... Rest> struct callout_token_from<Head, Rest...> {
	using type = typename callout_token_from<Rest...>::type;
};

template <typename... Modifiers> using callout_token_from_t = typename callout_token_from<Modifiers...>::type;

// --- the two shapes a with_callouts can have

template <typename T> struct is_callout_entry: std::false_type { };
#ifdef CTLL_CALLOUT_SUPPORTED
template <ctll::callout_key Key, auto Function> struct is_callout_entry<ctll::callout<Key, Function>>: std::true_type { };
#endif

// the key an atom dispatches under
template <typename Atom> struct callout_atom_key;
template <typename Name> struct callout_atom_key<callout_named<Name>> {
	static constexpr std::string_view view = callout_name_string<Name>::view;
};
template <size_t Number> struct callout_atom_key<callout_numbered<Number>> {
	static constexpr std::string_view view = callout_number_string<Number>::view;
};

#ifdef CTLL_CALLOUT_SUPPORTED
// find the entry whose key matches the atom (void when none does)
template <typename Atom, typename... Entries> struct callout_entry_for {
	using type = void;
};
template <typename Atom, typename Entry, typename... Rest> struct callout_entry_for<Atom, Entry, Rest...> {
	using type = std::conditional_t<Entry::key.view() == callout_atom_key<Atom>::view, Entry, typename callout_entry_for<Atom, Rest...>::type>;
};
#endif

template <typename Token> struct callout_dispatch;

template <typename... Handler> struct callout_dispatch<with_callouts<Handler...>> {
	static constexpr bool all_entries = (is_callout_entry<Handler>::value && ...);
	static constexpr bool legacy_handler = sizeof...(Handler) == 1 && !all_entries;
	static_assert(all_entries || legacy_handler, "ctre::with_callouts takes either one handler type (with a static callouts() map) or a list of ctll::callout entries");

	template <typename Atom> static constexpr auto bound_for() noexcept {
		if constexpr (legacy_handler) {
			return bound_as<Handler...>(Atom{});
		} else {
#ifdef CTLL_CALLOUT_SUPPORTED
			using entry = typename callout_entry_for<Atom, Handler...>::type;
			if constexpr (std::is_void_v<entry>) {
				return Atom{}; // no entry: the callout stays a no-op
			} else {
				return bound_as<entry>(Atom{});
			}
#else
			return Atom{};
#endif
		}
	}

	template <typename H, typename Name> static constexpr auto bound_as(callout_named<Name>) noexcept {
		return callout<H, Name, 0>{};
	}
	template <typename H, size_t Number> static constexpr auto bound_as(callout_numbered<Number>) noexcept {
		return callout<H, void, Number>{};
	}
};

// --- binding: rewrite callout markers to carry their implementation

template <typename Token> struct callout_binder {
	// leaves stay what they are
	template <typename T> static constexpr auto bind(T) noexcept {
		return T{};
	}

	template <typename Name> static constexpr auto bind(callout_named<Name>) noexcept {
		return callout_dispatch<Token>::template bound_for<callout_named<Name>>();
	}
	template <size_t Number> static constexpr auto bind(callout_numbered<Number>) noexcept {
		return callout_dispatch<Token>::template bound_for<callout_numbered<Number>>();
	}

	template <typename... Content> static constexpr auto bind(sequence<Content...>) noexcept {
		return sequence<decltype(bind(Content{}))...>{};
	}
	template <typename... Options> static constexpr auto bind(select<Options...>) noexcept {
		return select<decltype(bind(Options{}))...>{};
	}
	template <size_t A, size_t B, typename... Content> static constexpr auto bind(repeat<A, B, Content...>) noexcept {
		return repeat<A, B, decltype(bind(Content{}))...>{};
	}
	template <size_t A, size_t B, typename... Content> static constexpr auto bind(lazy_repeat<A, B, Content...>) noexcept {
		return lazy_repeat<A, B, decltype(bind(Content{}))...>{};
	}
	template <size_t A, size_t B, typename... Content> static constexpr auto bind(possessive_repeat<A, B, Content...>) noexcept {
		return possessive_repeat<A, B, decltype(bind(Content{}))...>{};
	}
	template <typename... Content> static constexpr auto bind(atomic_group<Content...>) noexcept {
		return atomic_group<decltype(bind(Content{}))...>{};
	}
	template <size_t Id, typename... Content> static constexpr auto bind(capture<Id, Content...>) noexcept {
		return capture<Id, decltype(bind(Content{}))...>{};
	}
	template <size_t Id, typename Name, typename... Content> static constexpr auto bind(capture_with_name<Id, Name, Content...>) noexcept {
		return capture_with_name<Id, Name, decltype(bind(Content{}))...>{};
	}
	template <typename... Content> static constexpr auto bind(lookahead_positive<Content...>) noexcept {
		return lookahead_positive<decltype(bind(Content{}))...>{};
	}
	template <typename... Content> static constexpr auto bind(lookahead_negative<Content...>) noexcept {
		return lookahead_negative<decltype(bind(Content{}))...>{};
	}
	// callouts inside lookbehinds are rejected at evaluation; binding is
	// still performed so the static_assert there fires with a clear message
	template <typename... Content> static constexpr auto bind(lookbehind_positive<Content...>) noexcept {
		return lookbehind_positive<decltype(bind(Content{}))...>{};
	}
	template <typename... Content> static constexpr auto bind(lookbehind_negative<Content...>) noexcept {
		return lookbehind_negative<decltype(bind(Content{}))...>{};
	}
	template <size_t Id, typename Yes, typename No> static constexpr auto bind(condition_capture<Id, Yes, No>) noexcept {
		return condition_capture<Id, decltype(bind(Yes{})), decltype(bind(No{}))>{};
	}
	template <typename Name, typename Yes, typename No> static constexpr auto bind(condition_capture_with_name<Name, Yes, No>) noexcept {
		return condition_capture_with_name<Name, decltype(bind(Yes{})), decltype(bind(No{}))>{};
	}
	template <typename... Content> static constexpr auto bind(define_group<Content...>) noexcept {
		return define_group<decltype(bind(Content{}))...>{};
	}
};

// entry point: no with_callouts modifier means no rewrite (markers
// evaluate as no-ops)
template <typename Token, typename RE> constexpr auto bind_callouts(RE re) noexcept {
	if constexpr (std::is_void_v<Token>) {
		return re;
	} else {
		return callout_binder<Token>::bind(re);
	}
}

template <typename... Modifiers> struct callout_binding {
	template <typename RE> using bound = decltype(bind_callouts<callout_token_from_t<Modifiers...>>(RE{}));
};

// --- invocation, from the evaluation engine

template <typename Handler, typename = void> struct callout_handler_has_map: std::false_type { };
template <typename Handler> struct callout_handler_has_map<Handler, std::void_t<decltype(Handler::callouts())>>: std::true_type { };

template <typename Handler, typename Name, size_t Number, typename R, typename BeginIterator, typename Iterator, typename EndIterator>
constexpr CTRE_FORCE_INLINE callout_result invoke_callout(BeginIterator begin, Iterator current, EndIterator last, const R & captures) noexcept {
	constexpr bool same_iterators = std::is_same_v<BeginIterator, Iterator> && std::is_same_v<Iterator, EndIterator>;
	static_assert(same_iterators, "callouts need a subject given as a plain range (not a zero-terminated pointer, not a utf-8 range) and are not supported inside lookbehinds");

	if constexpr (same_iterators) {
		using char_type = typename std::iterator_traits<Iterator>::value_type;
		constexpr bool contiguous = is_random_accessible<typename std::iterator_traits<Iterator>::iterator_category> && !is_reverse_iterator<Iterator>;
		static_assert(contiguous, "callouts need a contiguous, non-reverse subject range (callouts inside lookbehinds are not supported)");

		if constexpr (contiguous) {
			constexpr std::string_view key = [] {
				if constexpr (std::is_void_v<Name>) {
					return callout_number_string<Number>::view;
				} else {
					return callout_name_string<Name>::view;
				}
			}();

			const auto subject_size = static_cast<size_t>(std::distance(begin, last));
			const callout_data<char_type> data{
				Number,
				key,
				subject_size != 0 ? std::basic_string_view<char_type>(&*begin, subject_size) : std::basic_string_view<char_type>(),
				static_cast<size_t>(std::distance(begin, current)),
				static_cast<size_t>(std::distance(begin, Iterator(captures.template get<0>().begin())))
			};

			if constexpr (callout_handler_has_map<Handler>::value) {
				// handler type: dispatch by key through its map
				const auto table = Handler::callouts();
				using table_type = std::remove_cv_t<decltype(table)>;
				static_assert(std::is_invocable_r_v<callout_result, const typename table_type::mapped_type &, const callout_data<char_type> &>,
					"callout handler map values must be callable as ctre::callout_result(const ctre::callout_data<CharT> &) with the subject's character type");

				const auto it = table.find(key);
				if (it == table.end()) {
					return callout_result::proceed;
				}
				return it->second(data);
			} else {
				// inline entry: bound at compile time, invoked directly;
				// the result may be callout_result, bool or void
				using entry_result = decltype(Handler::invoke(data));
				if constexpr (std::is_void_v<entry_result>) {
					Handler::invoke(data);
					return callout_result::proceed;
				} else if constexpr (std::is_same_v<entry_result, bool>) {
					return Handler::invoke(data) ? callout_result::proceed : callout_result::fail;
				} else {
					static_assert(std::is_same_v<std::remove_cv_t<std::remove_reference_t<entry_result>>, callout_result>,
						"a ctll::callout callable must return ctre::callout_result, bool or void");
					return Handler::invoke(data);
				}
			}
		} else {
			return callout_result::fail;
		}
	} else {
		return callout_result::fail;
	}
}

} // namespace ctre

#endif
