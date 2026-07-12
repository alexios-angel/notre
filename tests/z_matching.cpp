#include <notre.hpp>
#include <string_view>

void empty_symbol() { }

template <typename Pattern> constexpr bool match(std::string_view input, Pattern) {
	return bool(notre::regular_expression<Pattern>::match(input.begin(), input.end()));
}

template <typename Pattern> constexpr bool search(std::string_view input, Pattern) {
	return bool(notre::regular_expression<Pattern>::search(input.begin(), input.end()));
}

template <typename Pattern> constexpr bool starts_with(std::string_view input, Pattern) {
	return bool(notre::regular_expression<Pattern>::starts_with(input.begin(), input.end()));
}

using namespace std::string_view_literals;

// issue #131
static_assert(match("x"sv, notre::sequence<notre::optional<notre::lazy_star<notre::any>>, notre::character<'x'>>()));
static_assert(match("1x"sv, notre::sequence<notre::lazy_optional<notre::lazy_star<notre::any>>, notre::character<'x'>>()));
static_assert(match("1x"sv, notre::sequence<notre::optional<notre::lazy_star<notre::any>>, notre::character<'x'>>()));
static_assert(match("1x"sv, notre::sequence<notre::optional<notre::lazy_star<notre::character<'1'> > >, notre::character<'x'>>()));
static_assert(match("1yx"sv, notre::sequence<notre::optional<notre::lazy_star<notre::character<'1'> >, notre::character<'y'> >, notre::character<'x'>>()));
static_assert(match("aaab"sv, notre::sequence<notre::possessive_plus<notre::optional<notre::character<'a'>>>, notre::character<'b'>>()));
static_assert(match("aaa"sv, notre::possessive_plus<notre::select<notre::empty, notre::character<'a'>>>()));
static_assert(match("aaa"sv, notre::lazy_plus<notre::select<notre::empty, notre::character<'a'>>>()));
static_assert(match("aaaa"sv, notre::possessive_plus<notre::lazy_star<notre::character<'a'>>>()));
static_assert(match(""sv, notre::lazy_star<notre::character<'b'>>()));
static_assert(match("aaaa"sv, notre::possessive_plus<notre::lazy_star<notre::character<'a'>>, notre::lazy_star<notre::character<'b'>>>()));
static_assert(match("aaaab"sv, notre::possessive_plus<notre::lazy_star<notre::character<'a'>>, notre::lazy_star<notre::character<'b'>>>()));

