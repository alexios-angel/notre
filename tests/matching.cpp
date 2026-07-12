#include <notre.hpp>
#include <string_view>

void empty_symbol() { }

template <typename Subject, typename Pattern> constexpr bool match(Subject input, Pattern) {
	return notre::regular_expression<Pattern>::match(input);
}

template <typename Subject, typename Pattern> constexpr bool search(Subject input, Pattern) {
	return notre::regular_expression<Pattern>::search(input);
}

template <typename Subject, typename Pattern> constexpr bool starts_with(Subject input, Pattern) {
	return notre::regular_expression<Pattern>::starts_with(input);
}

template <typename Subject, typename Pattern> constexpr auto multiline_match(Subject input, Pattern) {
	return notre::regular_expression<Pattern>::multiline_match(input);
}

template <typename Subject, typename Pattern> constexpr auto multiline_search(Subject input, Pattern) {
	return notre::regular_expression<Pattern>::multiline_search(input);
}

template <typename Subject, typename Pattern> constexpr auto multiline_starts_with(Subject input, Pattern) {
	return notre::regular_expression<Pattern>::multiline_starts_with(input);
}

using namespace std::string_view_literals;

static_assert(match("a"sv, notre::character<'a'>()));
static_assert(search("abc"sv, notre::character<'a'>()));
static_assert(search("abc"sv, notre::character<'b'>())); 
static_assert(search("abc"sv, notre::character<'c'>())); 
static_assert(starts_with("abc"sv, notre::character<'a'>())); 
static_assert(!starts_with("abc"sv, notre::character<'b'>()));
static_assert(!match("abc"sv, notre::character<'b'>()));
static_assert(!match("a"sv, notre::character<'b'>()));
static_assert(match("a"sv, notre::any()));
static_assert(match("a"sv, notre::set<notre::char_range<'a','z'>>()));
static_assert(match("f"sv, notre::set<notre::char_range<'a','z'>>()));
static_assert(match("z"sv, notre::set<notre::char_range<'a','z'>>()));
static_assert(!match("Z"sv, notre::set<notre::char_range<'a','z'>>()));
static_assert(match("0"sv, notre::set<notre::char_range<'a','z'>, notre::char_range<'0','9'>>()));
static_assert(!match("A"sv, notre::set<notre::char_range<'a','z'>, notre::char_range<'0','9'>>()));
static_assert(match("0"sv, notre::set<notre::xdigit_chars>()));
static_assert(match("9"sv, notre::set<notre::xdigit_chars>()));
static_assert(match("a"sv, notre::set<notre::xdigit_chars>()));
static_assert(match("f"sv, notre::set<notre::xdigit_chars>()));
static_assert(!match("g"sv, notre::set<notre::xdigit_chars>()));
static_assert(match("abcdef"sv, notre::string<'a','b','c','d','e','f'>()));
static_assert(!match("abcgef"sv, notre::string<'a','b','c','d','e','f'>()));
static_assert(match(""sv, notre::string<>()));
static_assert(match("a"sv, notre::select<notre::character<'a'>, notre::character<'b'>, notre::character<'c'>>()));
static_assert(match("b"sv, notre::select<notre::character<'a'>, notre::character<'b'>, notre::character<'c'>>()));
static_assert(match("c"sv, notre::select<notre::character<'a'>, notre::character<'b'>, notre::character<'c'>>()));
static_assert(!match("d"sv, notre::select<notre::character<'a'>, notre::character<'b'>, notre::character<'c'>>()));
static_assert(match("xy"sv, notre::optional<notre::string<'x','y'>>()));
static_assert(match(""sv, notre::optional<notre::string<'x','y'>>()));
static_assert(match("abc"sv, notre::sequence<notre::assert_subject_begin, notre::string<'a','b','c'>>()));
static_assert(match("def"sv, notre::sequence<notre::assert_subject_begin, notre::string<'d','e','f'>, notre::assert_subject_end>()));
static_assert(!match("a"sv, notre::sequence<notre::character<'a'>, notre::assert_subject_begin>()));
static_assert(!match("a"sv, notre::sequence<notre::assert_subject_end, notre::character<'a'>>()));

static_assert(match("aaax"sv, notre::sequence<notre::lazy_plus<notre::character<'a'>>, notre::character<'x'>>()));
static_assert(match("ax"sv, notre::sequence<notre::lazy_plus<notre::character<'a'>>, notre::character<'x'>>()));
static_assert(!match("x"sv, notre::sequence<notre::lazy_plus<notre::character<'a'>>, notre::character<'x'>>()));

static_assert(match("aaax"sv, notre::sequence<notre::possessive_plus<notre::character<'a'>>, notre::character<'x'>>()));
static_assert(match("ax"sv, notre::sequence<notre::possessive_plus<notre::character<'a'>>, notre::character<'x'>>()));
static_assert(!match("x"sv, notre::sequence<notre::possessive_plus<notre::character<'a'>>, notre::character<'x'>>()));

static_assert(match("aaax"sv, notre::sequence<notre::lazy_star<notre::character<'a'>>, notre::character<'x'>>()));
static_assert(match("ax"sv, notre::sequence<notre::lazy_star<notre::character<'a'>>, notre::character<'x'>>()));
static_assert(match("x"sv, notre::sequence<notre::lazy_star<notre::character<'a'>>, notre::character<'x'>>()));
static_assert(!match("y"sv, notre::sequence<notre::lazy_star<notre::character<'a'>>, notre::character<'x'>>()));

static_assert(match("aaax"sv, notre::sequence<notre::possessive_star<notre::character<'a'>>, notre::character<'x'>>()));
static_assert(match("ax"sv, notre::sequence<notre::possessive_star<notre::character<'a'>>, notre::character<'x'>>()));
static_assert(match("x"sv, notre::sequence<notre::possessive_star<notre::character<'a'>>, notre::character<'x'>>()));
static_assert(!match("y"sv, notre::sequence<notre::possessive_star<notre::character<'a'>>, notre::character<'x'>>()));

static_assert(!match("aaab"sv, notre::sequence<notre::possessive_star<notre::character<'a'>>, notre::string<'a','b'>>()));
static_assert(!match("aaab"sv, notre::sequence<notre::possessive_plus<notre::character<'a'>>, notre::string<'a','b'>>()));
static_assert(!match("ab"sv, notre::sequence<notre::possessive_star<notre::character<'a'>>, notre::string<'a','b'>>()));
static_assert(!match("aab"sv, notre::sequence<notre::possessive_plus<notre::character<'a'>>, notre::string<'a','b'>>()));

static_assert(match("aaba"sv, notre::sequence<notre::possessive_star<notre::character<'a'>>, notre::string<'b','a'>>()));
static_assert(match("aaba"sv, notre::sequence<notre::possessive_plus<notre::character<'a'>>, notre::string<'b','a'>>()));
static_assert(match("ba"sv, notre::sequence<notre::possessive_star<notre::character<'a'>>, notre::string<'b','a'>>()));
static_assert(match("aba"sv, notre::sequence<notre::possessive_plus<notre::character<'a'>>, notre::string<'b','a'>>()));

static_assert(match("aaax"sv, notre::sequence<notre::lazy_repeat<3,0,notre::character<'a'>>, notre::character<'x'>>()));

static_assert(search("aaaaaa"sv, notre::repeat<0,5,notre::character<'a'>>()));
static_assert(!match("aaaaaa"sv, notre::sequence<notre::repeat<0,5,notre::character<'a'>>, notre::assert_subject_end>()));
static_assert(match("aaaaa"sv, notre::sequence<notre::repeat<0,5,notre::character<'a'>>, notre::assert_subject_end>()));

static_assert(match("aaa"sv, notre::star<notre::character<'a'>>()));
static_assert(match("aaa"sv, notre::plus<notre::character<'a'>>()));
static_assert(match(""sv, notre::star<notre::character<'a'>>()));
static_assert(match("a"sv, notre::plus<notre::character<'a'>>()));

static_assert(match("aaxb"sv, notre::sequence<notre::star<notre::character<'a'>>, notre::string<'x','b'>>()));
static_assert(match("aaxb"sv, notre::sequence<notre::plus<notre::character<'a'>>, notre::string<'x','b'>>()));
static_assert(match("xb"sv, notre::sequence<notre::star<notre::character<'a'>>, notre::string<'x','b'>>()));
static_assert(match("axb"sv, notre::sequence<notre::plus<notre::character<'a'>>, notre::string<'x','b'>>()));

static_assert(match("aaab"sv, notre::sequence<notre::star<notre::character<'a'>>, notre::string<'a','b'>>()));
static_assert(match("aaab"sv, notre::sequence<notre::plus<notre::character<'a'>>, notre::string<'a','b'>>()));
static_assert(match("ab"sv, notre::sequence<notre::star<notre::character<'a'>>, notre::string<'a','b'>>()));
static_assert(match("aab"sv, notre::sequence<notre::plus<notre::character<'a'>>, notre::string<'a','b'>>()));

static_assert(!match("aab"sv, notre::sequence<notre::repeat<2,5,notre::character<'a'>>, notre::string<'a','b'>>()));
static_assert(match("aaab"sv, notre::sequence<notre::repeat<2,5,notre::character<'a'>>, notre::string<'a','b'>>()));
static_assert(match("aaaab"sv, notre::sequence<notre::repeat<2,5,notre::character<'a'>>, notre::string<'a','b'>>()));
static_assert(match("aaaaab"sv, notre::sequence<notre::repeat<2,5,notre::character<'a'>>, notre::string<'a','b'>>()));
static_assert(match("aaaaaab"sv, notre::sequence<notre::repeat<2,5,notre::character<'a'>>, notre::string<'a','b'>>()));
static_assert(!match("aaaaaaab"sv, notre::sequence<notre::repeat<2,5,notre::character<'a'>>, notre::string<'a','b'>>()));

// issue #131
static_assert(match("x"sv, notre::sequence<notre::optional<notre::lazy_star<notre::character<'1'>>>, notre::character<'x'>>()));

static_assert(match("1x"sv, notre::sequence<notre::optional<notre::lazy_optional<notre::character<'1'>>>, notre::character<'x'>>()));
static_assert(match("1x"sv, notre::sequence<notre::optional<notre::lazy_star<notre::character<'1'>>>, notre::character<'x'>>()));
static_assert(match("1x"sv, notre::sequence<notre::optional<notre::lazy_star<notre::character<'1'> > >, notre::character<'x'>>()));
static_assert(match("aaab"sv, notre::sequence<notre::possessive_plus<notre::optional<notre::character<'a'>>>, notre::character<'b'>>()));

static_assert(match("1yx"sv, notre::sequence<notre::optional<notre::lazy_star<notre::character<'1'> >, notre::character<'y'> >, notre::character<'x'>>()));
static_assert(match("aaa"sv, notre::possessive_plus<notre::select<notre::empty, notre::character<'a'>>>()));

static_assert(match("x"sv, notre::repeat<1,0,notre::repeat<0,0,notre::character<'x'>>>()));
static_assert(match("x"sv, notre::repeat<1,0,notre::lazy_repeat<0,0,notre::character<'x'>>>()));
static_assert(match("x"sv, notre::repeat<1,0,notre::possessive_repeat<0,0,notre::character<'x'>>>()));

static_assert(match("x"sv, notre::lazy_repeat<1,0,notre::repeat<0,0,notre::character<'x'>>>()));
static_assert(match("x"sv, notre::lazy_repeat<1,0,notre::lazy_repeat<0,0,notre::character<'x'>>>()));
static_assert(match("x"sv, notre::lazy_repeat<1,0,notre::possessive_repeat<0,0,notre::character<'x'>>>()));

static_assert(match("x"sv, notre::possessive_repeat<1,0,notre::repeat<0,0,notre::character<'x'>>>()));
static_assert(match("x"sv, notre::possessive_repeat<1,0,notre::lazy_repeat<0,0,notre::character<'x'>>>()));
static_assert(match("x"sv, notre::possessive_repeat<1,0,notre::possessive_repeat<0,0,notre::character<'x'>>>()));

static_assert(match("xy"sv, notre::sequence<notre::repeat<1,0,notre::repeat<0,0,notre::character<'x'>>>, notre::character<'y'>>()));
static_assert(match("xy"sv, notre::sequence<notre::repeat<1,0,notre::lazy_repeat<0,0,notre::character<'x'>>>, notre::character<'y'>>()));
static_assert(match("xy"sv, notre::sequence<notre::repeat<1,0,notre::possessive_repeat<0,0,notre::character<'x'>>>, notre::character<'y'>>()));

static_assert(match("xy"sv, notre::sequence<notre::lazy_repeat<1,0,notre::repeat<0,0,notre::character<'x'>>>, notre::character<'y'>>()));
static_assert(match("xy"sv, notre::sequence<notre::lazy_repeat<1,0,notre::lazy_repeat<0,0,notre::character<'x'>>>, notre::character<'y'>>()));
static_assert(match("xy"sv, notre::sequence<notre::lazy_repeat<1,0,notre::possessive_repeat<0,0,notre::character<'x'>>>, notre::character<'y'>>()));

static_assert(match("xy"sv, notre::sequence<notre::possessive_repeat<1,0,notre::repeat<0,0,notre::character<'x'>>>, notre::character<'y'>>()));
static_assert(match("xy"sv, notre::sequence<notre::possessive_repeat<1,0,notre::lazy_repeat<0,0,notre::character<'x'>>>, notre::character<'y'>>()));
static_assert(match("xy"sv, notre::sequence<notre::possessive_repeat<1,0,notre::possessive_repeat<0,0,notre::character<'x'>>>, notre::character<'y'>>()));


static_assert(match("xy"sv, notre::sequence<notre::repeat<0,0,notre::repeat<1,0,notre::character<'x'>>>, notre::character<'y'>>()));
static_assert(match("xy"sv, notre::sequence<notre::repeat<0,0,notre::lazy_repeat<1,0,notre::character<'x'>>>, notre::character<'y'>>()));
static_assert(match("xy"sv, notre::sequence<notre::repeat<0,0,notre::possessive_repeat<1,0,notre::character<'x'>>>, notre::character<'y'>>()));

static_assert(match("xy"sv, notre::sequence<notre::lazy_repeat<0,0,notre::repeat<1,0,notre::character<'x'>>>, notre::character<'y'>>()));
static_assert(match("xy"sv, notre::sequence<notre::lazy_repeat<0,0,notre::lazy_repeat<1,0,notre::character<'x'>>>, notre::character<'y'>>()));
static_assert(match("xy"sv, notre::sequence<notre::lazy_repeat<0,0,notre::possessive_repeat<1,0,notre::character<'x'>>>, notre::character<'y'>>()));

static_assert(match("xy"sv, notre::sequence<notre::possessive_repeat<0,0,notre::repeat<1,0,notre::character<'x'>>>, notre::character<'y'>>()));
static_assert(match("xy"sv, notre::sequence<notre::possessive_repeat<0,0,notre::lazy_repeat<1,0,notre::character<'x'>>>, notre::character<'y'>>()));
static_assert(match("xy"sv, notre::sequence<notre::possessive_repeat<1,0,notre::possessive_repeat<0,0,notre::character<'x'>>>, notre::character<'y'>>()));

// word boundary
static_assert(starts_with("a "sv, notre::sequence<notre::character<'a'>, notre::boundary<notre::word_chars>>()));
static_assert(starts_with(u8"a "sv, notre::sequence<notre::character<'a'>, notre::boundary<notre::word_chars>>()));
static_assert(starts_with("aaaa "sv, notre::sequence<notre::plus<notre::character<'a'>>, notre::boundary<notre::word_chars>, notre::any>()));
static_assert(starts_with("  aaaa"sv, notre::sequence<notre::plus<notre::any>, notre::boundary<notre::word_chars>>()));
static_assert(starts_with(u8"aaaa "sv, notre::sequence<notre::plus<notre::character<'a'>>, notre::boundary<notre::word_chars>, notre::any>()));
static_assert(starts_with(u8"  aaaa"sv, notre::sequence<notre::plus<notre::any>, notre::boundary<notre::word_chars>>()));
static_assert(match("a ", notre::sequence<notre::character<'a'>, notre::word_boundary, notre::character<' '>>() ));
static_assert(match("ab", notre::sequence<notre::character<'a'>, notre::not_word_boundary, notre::character<'b'>>() ));

// multiline
static_assert(multiline_match("", notre::assert_subject_begin()));
static_assert(multiline_match("", notre::assert_subject_end()));
static_assert(multiline_match("", notre::assert_line_begin()));
static_assert(multiline_match("", notre::assert_line_end()));
static_assert(multiline_match("", notre::assert_subject_end_line()));
static_assert(!multiline_match("\n", notre::any()));
static_assert(multiline_starts_with("aaa\nbbb", notre::plus<notre::any>()));
static_assert(multiline_starts_with("aaa\nbbb", notre::plus<notre::any>()).size() == 3);
static_assert(multiline_starts_with("aaa\nbbb", notre::sequence<notre::plus<notre::any>,notre::assert_line_end,notre::character<'\n'>>()));
static_assert(multiline_starts_with("aaa\nbbb", notre::sequence<notre::plus<notre::any>,notre::assert_line_end,notre::character<'\n'>>()).size() == 4);
