#include <notre.hpp>
#include <string_view>

template <typename Subject, typename Pattern> constexpr bool match(Subject input, Pattern) {
	return notre::regular_expression<Pattern>::match(input);
}

template <typename Subject, typename Pattern> constexpr bool search(Subject input, Pattern) {
	return notre::regular_expression<Pattern>::search(input);
}

template <typename Subject, typename Pattern> constexpr bool starts_with(Subject input, Pattern) {
	return notre::regular_expression<Pattern>::starts_with(input);
}

using namespace std::string_view_literals;

static_assert(match("ab"sv, notre::plus<notre::select<notre::string<'a','b'>, notre::character<'a'>>>()), "(ab|a)+");
// this will need fix in different branch as this is a long standing problem
//static_assert(match("ab"sv, notre::plus<notre::select<notre::character<'a'>, notre::string<'a','b'>>>()), "(a|ab)+");

