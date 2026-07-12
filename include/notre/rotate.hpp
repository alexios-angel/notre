#ifndef NOTRE__ROTATE__HPP
#define NOTRE__ROTATE__HPP

#include "atoms.hpp"
#include "atoms_characters.hpp"
#include "atoms_unicode.hpp"

namespace notre {

// helper functions
template <size_t Index, typename... Content> auto convert_to_capture(ctll::list<Content...>) -> capture<Index, Content...>;
template <size_t Index, typename Name, typename... Content> auto convert_to_named_capture(ctll::list<Content...>) -> capture_with_name<Index, Name, Content...>;
template <template <size_t, size_t, typename...> typename CycleType, size_t A, size_t B, typename... Content> auto convert_to_repeat(ctll::list<Content...>) -> CycleType<A, B, Content...>;
template <template <typename...> typename ListType, typename... Content> auto convert_to_basic_list(ctll::list<Content...>) -> ListType<Content...>;

template <auto V> struct rotate_value {
	template <auto... Vs> friend constexpr auto operator+(string<Vs...>, rotate_value<V>) noexcept -> string<V, Vs...> { return {}; }
};

struct rotate_for_lookbehind {

// from atoms_characters.hpp
template <auto V> static auto rotate(character<V>) -> character<V>;
template <typename... Content> static auto rotate(negative_set<Content...>) -> negative_set<Content...>;
template <typename... Content> static auto rotate(set<Content...>) -> set<Content...>;
template <auto... Cs> static auto rotate(enumeration<Cs...>) -> enumeration<Cs...>;
template <typename... Content> static auto rotate(negate<Content...>) -> negate<Content...>;
template <auto A, auto B> static auto rotate(char_range<A,B>) -> char_range<A,B>;

// from atoms_unicode.hpp
template <auto... Str> static auto rotate(property_name<Str...>) -> property_name<Str...>;
template <auto... Str> static auto rotate(property_value<Str...>) -> property_value<Str...>;
template <typename T, T Type> static auto rotate(binary_property<T, Type>) -> binary_property<T, Type>;
template <typename T, T Type, auto Value> static auto rotate(property<T, Type, Value>) -> property<T, Type, Value>;

// from atoms.hpp
static auto rotate(accept) -> accept;
static auto rotate(reject) -> reject;
static auto rotate(start_mark) -> start_mark;
static auto rotate(end_mark) -> end_mark;
static auto rotate(end_cycle_mark) -> end_cycle_mark;
static auto rotate(end_lookahead_mark) -> end_lookahead_mark;
static auto rotate(end_lookbehind_mark) -> end_lookbehind_mark;
template <size_t Id> static auto rotate(numeric_mark<Id>) -> numeric_mark<Id>;
static auto rotate(any) -> any;

static auto rotate(empty) -> empty;

// select rotates only insides of selection, not select itself
template <typename... Content> static auto rotate(select<Content...>) {
  return select<decltype(rotate(Content{}))...>{};
}


template <size_t a, size_t b, typename... Content> static auto rotate(repeat<a,b,Content...>) -> decltype(notre::convert_to_repeat<repeat, a, b>(ctll::rotate(ctll::list<decltype(rotate(Content{}))...>{})));
template <size_t a, size_t b, typename... Content> static auto rotate(lazy_repeat<a,b,Content...>) -> decltype(notre::convert_to_repeat<lazy_repeat, a, b>(ctll::rotate(ctll::list<decltype(rotate(Content{}))...>{})));
template <size_t a, size_t b, typename... Content> static auto rotate(possessive_repeat<a,b,Content...>) -> decltype(notre::convert_to_repeat<possessive_repeat, a, b>(ctll::rotate(ctll::list<decltype(rotate(Content{}))...>{})));

template <size_t Index, typename... Content> static auto rotate(capture<Index, Content...>) {
	return notre::convert_to_capture<Index>(ctll::rotate(ctll::list<decltype(rotate(Content{}))...>{}));
}

template <size_t Index, typename Name, typename... Content> static auto rotate(capture_with_name<Index, Name, Content...>) {
	return notre::convert_to_named_capture<Index, Name>(ctll::rotate(ctll::list<decltype(rotate(Content{}))...>{}));
}

template <size_t Index> static auto rotate(back_reference<Index>) -> back_reference<Index>;
template <typename Name> static auto rotate(back_reference_with_name<Name>) -> back_reference_with_name<Name>;

// subroutine-call markers are leaves here; the expansion is rotated as a
// whole when a call inside a lookbehind is resolved
template <size_t Index> static auto rotate(subroutine_call<Index>) -> subroutine_call<Index>;
template <typename Name> static auto rotate(subroutine_call_with_name<Name>) -> subroutine_call_with_name<Name>;

// callouts are zero-width leaves
template <size_t Number> static auto rotate(callout_numbered<Number>) -> callout_numbered<Number>;
template <typename Name> static auto rotate(callout_named<Name>) -> callout_named<Name>;
template <typename Handler, typename Name, size_t Number> static auto rotate(callout<Handler, Name, Number>) -> callout<Handler, Name, Number>;

// conditional branches rotate (bodies, not trailing return types, so the
// recursive rotate calls see members declared later in this class);
// a DEFINE body is never evaluated
template <size_t Index, typename Yes, typename No> static auto rotate(condition_capture<Index, Yes, No>) {
	return condition_capture<Index, decltype(rotate(Yes{})), decltype(rotate(No{}))>{};
}
template <typename Name, typename Yes, typename No> static auto rotate(condition_capture_with_name<Name, Yes, No>) {
	return condition_capture_with_name<Name, decltype(rotate(Yes{})), decltype(rotate(No{}))>{};
}
template <typename... Content> static auto rotate(define_group<Content...>) -> define_group<Content...>;

template <typename... Content> static auto rotate(look_start<Content...>) -> look_start<Content...>;

template <auto... Str> static auto rotate(string<Str...>) -> decltype((string<>{} + ... + rotate_value<Str>{}));

template <typename... Content> static auto rotate(sequence<Content...>) {
	return notre::convert_to_basic_list<sequence>(ctll::rotate(ctll::list<decltype(rotate(Content{}))...>{}));
}

// we don't rotate lookaheads
template <typename... Content> static auto rotate(lookahead_positive<Content...>) -> lookahead_positive<Content...>;
template <typename... Content> static auto rotate(lookahead_negative<Content...>) -> lookahead_negative<Content...>;
template <typename... Content> static auto rotate(lookbehind_positive<Content...>) -> lookbehind_positive<Content...>;
template <typename... Content> static auto rotate(lookbehind_negative<Content...>) -> lookbehind_negative<Content...>;

static auto rotate(atomic_start) -> atomic_start;

template <typename... Content> static auto rotate(atomic_group<Content...>) {
	return notre::convert_to_basic_list<atomic_group>(ctll::rotate(ctll::list<decltype(rotate(Content{}))...>{}));
}

template <typename... Content> static auto rotate(boundary<Content...>) -> boundary<Content...>;
template <typename... Content> static auto rotate(not_boundary<Content...>) -> not_boundary<Content...>;

static auto rotate(assert_subject_begin) -> assert_subject_begin;
static auto rotate(assert_subject_end) -> assert_subject_end;
static auto rotate(assert_subject_end_line) -> assert_subject_end_line;
static auto rotate(assert_line_begin) -> assert_line_begin;
static auto rotate(assert_line_end) -> assert_line_end;

// parse must survive \K in a lookbehind so the post-parse check can
// reject it with a readable message (resolve_subroutines.hpp)
static auto rotate(match_point_reset) -> match_point_reset;

};

}

#endif
