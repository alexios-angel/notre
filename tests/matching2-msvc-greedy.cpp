#define NOTRE_MSVC_GREEDY_WORKAROUND

#include <notre.hpp>
#include <string_view>

void empty_symbol() { }

#if !NOTRE_CNTTP_COMPILER_CHECK
#define NOTRE_CREATE(pattern) (pattern ## _notre)
#define NOTRE_SYNTAX(pattern) (pattern ## _notre_syntax)
#else

template <ctll::fixed_string input> constexpr auto create() {
	constexpr auto _input = input;
	
	using tmp = typename ctll::parser<notre::pcre, _input, notre::pcre_actions>::template output<notre::pcre_context<>>;
	static_assert(tmp(), "Regular Expression contains syntax error.");
	using re = decltype(front(typename tmp::output_type::stack_type()));
	return notre::regular_expression(re());
}

template <ctll::fixed_string input> constexpr bool syntax() {
	constexpr auto _input = input;
	
	return ctll::parser<notre::pcre, _input, notre::pcre_actions>::template correct_with<notre::pcre_context<>>;
}


#define NOTRE_CREATE(pattern) create<pattern>()
#define NOTRE_SYNTAX(pattern) syntax<pattern>()

#endif

using namespace notre::literals;
using namespace notre::test_literals;
using namespace std::string_view_literals;

static_assert(NOTRE_CREATE("").search("abc"sv));
static_assert(NOTRE_CREATE("abc").match("abc"sv));

static_assert(NOTRE_CREATE("a").match("a"sv));
static_assert(NOTRE_CREATE("a").search("abc"sv));
static_assert(NOTRE_CREATE("b").search("abc"sv));
static_assert(!NOTRE_CREATE("^b").match("abc"sv));
static_assert(!NOTRE_CREATE("b").match("a"sv));
static_assert(NOTRE_CREATE(".").match("a"sv));
static_assert(NOTRE_CREATE(".").search("abc"sv));
static_assert(NOTRE_CREATE("[\\-]").match("-"sv));
static_assert(NOTRE_CREATE("[\\\\]").match("\\"sv));
static_assert(NOTRE_CREATE("[a-z]").match("a"sv));
static_assert(NOTRE_CREATE("[a-z]").match("f"sv));
static_assert(NOTRE_CREATE("[a-z]").match("z"sv));
static_assert(!NOTRE_CREATE("[a-z]").match("Z"sv));
static_assert(NOTRE_CREATE("\\u0050").match("P"sv));
static_assert(NOTRE_CREATE("[\\u0050-\\u0051]").match("Q"sv));
static_assert(NOTRE_CREATE("\u0050").match("P"sv)); // be aware!

static_assert(NOTRE_CREATE("^[\\x30-\\x39]+?$").match("123456789"sv));
static_assert(NOTRE_CREATE("[a-z0-9]").match("0"sv));
static_assert(!NOTRE_CREATE("[a-z0-9]").match("A"sv));
static_assert(NOTRE_CREATE("[[:xdigit:]]").match("0"sv));
static_assert(NOTRE_CREATE("[[:xdigit:]]").match("9"sv));
static_assert(NOTRE_CREATE("[[:xdigit:]]").match("a"sv));
static_assert(NOTRE_CREATE("[[:xdigit:]]").match("A"sv));
static_assert(NOTRE_CREATE("[[:xdigit:]]").match("f"sv));
static_assert(!NOTRE_CREATE("[[:xdigit:]]").match("g"sv));
static_assert(NOTRE_CREATE("abcdef").match("abcdef"sv));
static_assert(!NOTRE_CREATE("abcdef").match("abcGef"sv));
static_assert(NOTRE_CREATE("").match(""sv));
static_assert(NOTRE_CREATE("(?:a|b|c)").match("a"sv));
static_assert(NOTRE_CREATE("(?:a|b|c)").match("b"sv));
static_assert(NOTRE_CREATE("(?:a|b|c)").match("c"sv));
static_assert(!NOTRE_CREATE("(?:a|b|c)").match("d"sv));
static_assert(NOTRE_CREATE("(?:xy)?").match("xy"sv));
static_assert(NOTRE_CREATE("(?:xy)?").match(""sv));
static_assert(NOTRE_CREATE("(?:xy)?").search("zxy"sv));
static_assert(NOTRE_CREATE("(?:xy)?$").search("zxy"sv));
static_assert(!NOTRE_CREATE("~(?:xy)?$").match("zxy"sv));

static_assert(NOTRE_CREATE("^abc").match("abc"sv));
static_assert(NOTRE_CREATE("^def$").match("def"sv));
static_assert(!NOTRE_CREATE("a^").match("a"sv));
static_assert(!NOTRE_CREATE("$a").match("a"sv));

static_assert(NOTRE_CREATE("a+?").search("aaax"sv));
static_assert(NOTRE_CREATE("a+?").search("ax"sv));
static_assert(!NOTRE_CREATE("a+?").match("x"sv));

static_assert(NOTRE_CREATE("a++").search("aaax"sv));
static_assert(NOTRE_CREATE("a++").search("ax"sv));
static_assert(!NOTRE_CREATE("a++").match("x"sv));

static_assert(NOTRE_CREATE("a*?x").match("aaax"sv));
static_assert(NOTRE_CREATE("a*?x").match("ax"sv));
static_assert(NOTRE_CREATE("a*?x").match("x"sv));
static_assert(!NOTRE_CREATE("a*?x").match("y"sv));

static_assert(NOTRE_CREATE("a*+x").match("aaax"sv));
static_assert(NOTRE_CREATE("a*+x").match("ax"sv));
static_assert(NOTRE_CREATE("a*+x").match("x"sv));
static_assert(!NOTRE_CREATE("a*+x").match("y"sv));

static_assert(!NOTRE_CREATE("a*+ab").match("aaab"sv));
static_assert(!NOTRE_CREATE("a++ab").match("aaab"sv));
static_assert(!NOTRE_CREATE("a*+ab").match("ab"sv));
static_assert(!NOTRE_CREATE("a++ab").match("aab"sv));

static_assert(NOTRE_CREATE("a*+ba").match("aaba"sv));
static_assert(NOTRE_CREATE("a++ba").match("aaba"sv));
static_assert(NOTRE_CREATE("a*+ba").match("ba"sv));
static_assert(NOTRE_CREATE("a++ba").match("aba"sv));

static_assert(NOTRE_CREATE("a{3,}x").match("aaax"sv));
static_assert(NOTRE_CREATE("a{3,}x").match("aaaax"sv));

static_assert(NOTRE_CREATE("^a{5}").match("aaaaa"sv));
static_assert(NOTRE_CREATE("^a{5}").search("aaaaaa"sv));
static_assert(!NOTRE_CREATE("^a{5}$").match("aaaaaa"sv));

static_assert(NOTRE_CREATE("a*").match("aaa"sv));
static_assert(NOTRE_CREATE("a+").match("aaa"sv));
static_assert(NOTRE_CREATE("a*").match(""sv));
static_assert(NOTRE_CREATE("a+").match("a"sv));

static_assert(NOTRE_CREATE("a*$").match("aaa"sv));
static_assert(NOTRE_CREATE("a+$").match("aaa"sv));
static_assert(NOTRE_CREATE("a*$").match(""sv));
static_assert(NOTRE_CREATE("a+$").match("a"sv));

static_assert(NOTRE_CREATE("a*xb").match("aaxb"sv));
static_assert(NOTRE_CREATE("a+xb").match("aaxb"sv));
static_assert(NOTRE_CREATE("a*xb").match("xb"sv));
static_assert(NOTRE_CREATE("a+xb").match("axb"sv));

static_assert(NOTRE_CREATE("a*ab").match("aaab"sv));
static_assert(NOTRE_CREATE("a+ab").match("aaab"sv));
static_assert(NOTRE_CREATE("a*ab").match("ab"sv));
static_assert(NOTRE_CREATE("a+ab").match("aab"sv));

static_assert(!NOTRE_CREATE("^a{2,5}ab").match("aab"sv));
static_assert(NOTRE_CREATE("^a{2,5}ab").match("aaab"sv));
static_assert(NOTRE_CREATE("^a{2,5}ab").match("aaaab"sv));
static_assert(NOTRE_CREATE("^a{2,5}ab").match("aaaaab"sv));
static_assert(NOTRE_CREATE("^a{2,5}ab").match("aaaaaab"sv));
static_assert(!NOTRE_CREATE("^a{2,5}ab").match("aaaaaaab"sv));

static_assert(NOTRE_CREATE("[a-z]+[^a-z]+").match("abcdef123456"sv));

static_assert(NOTRE_CREATE("(abc)").match("abc"sv));
static_assert(NOTRE_CREATE("(abc)+").match("abc"sv));
static_assert(NOTRE_CREATE("(abc)+").match("abcabc"sv));
static_assert(NOTRE_CREATE("(abc)+").match("abcabcabc"sv));

static_assert(NOTRE_CREATE("(?<name>abc)").match("abc"sv));
static_assert(NOTRE_CREATE("(?<name>abc)+").match("abc"sv));
static_assert(NOTRE_CREATE("(?<name>abc)+").match("abcabc"sv));
static_assert(NOTRE_CREATE("(?<name>abc)+").match("abcabcabc"sv));
static_assert(!NOTRE_CREATE("(?<name>abc)+").match("name"sv));

static_assert(std::string_view{NOTRE_CREATE("^([a-z]+)").search("abcdef1234"sv)} == "abcdef"sv);
static_assert(std::string_view{NOTRE_CREATE("^([a-z]+)1234").match("abcdef1234"sv)} == "abcdef1234"sv);
static_assert(std::string_view{NOTRE_CREATE("^([a-z])").search("abcdef1234"sv)} == "a"sv);

static_assert(NOTRE_CREATE("^([0-9]+[a-z]+)+").match("123abc456def"sv));
static_assert(NOTRE_CREATE("^([0-9]+[a-z]+)+").match("123abc456def"sv).template get<1>() == "456def"sv);
static_assert(NOTRE_CREATE("^([0-9]+[a-z]+)+").match("123abc456def"sv).template get<0>() == "123abc456def"sv);

static_assert(NOTRE_CREATE("^([0-9]++[a-z]++)+").match("123abc456def"sv));
static_assert(NOTRE_CREATE("^([0-9]++[a-z]++)+").match("123abc456def"sv).template get<1>() == "456def"sv);
static_assert(NOTRE_CREATE("^([0-9]++[a-z]++)+").match("123abc456def"sv).template get<0>() == "123abc456def"sv);

static_assert(NOTRE_CREATE("^([0-9]+?[a-z]+?)+").search("123abc456def"sv));
static_assert(NOTRE_CREATE("^([0-9]+?[a-z]+?)+").search("123abc456def"sv).template get<1>() == "123a"sv);
static_assert(NOTRE_CREATE("^([0-9]+?[a-z]+?)+").search("123abc456def"sv).template get<0>() == "123a"sv);

static_assert(NOTRE_CREATE("^([0-9]+?[a-z]++)+").match("123abc456def"sv));
static_assert(NOTRE_CREATE("^([0-9]+?[a-z]++)+").match("123abc456def"sv).template get<1>() == "456def"sv);
static_assert(NOTRE_CREATE("^([0-9]+?[a-z]++)+").match("123abc456def"sv).template get<0>() == "123abc456def"sv);

static_assert(NOTRE_CREATE("^([a-z]{2})([a-z]{2})").match("abcd"sv).template get<2>() == "cd"sv);
// FIXME  ID support
//static_assert(NOTRE_CREATE("^([a-z]{2})(?<second>[a-z]{2})").match("abcd"sv).template get<decltype("second")_id)>() == "cd"sv);

static_assert(NOTRE_CREATE("^([a-z]+):\\g{1}$").match("abc:abc"sv));
static_assert(NOTRE_CREATE("^([a-z]+):\\g{1}$").match("abc:abc"sv).template get<1>() == "abc"sv);
static_assert(!NOTRE_CREATE("^([a-z]+):\\g{1}$").match("abc:abce"sv));
static_assert(NOTRE_CREATE("^([a-z]+)\\g{1}$").match("abcabc"sv));
static_assert(!NOTRE_CREATE("^([a-z]+)\\g{1}$").match("abcabcd"sv));
static_assert(NOTRE_SYNTAX("^([a-z]+)\\g{-1}$"));
static_assert(NOTRE_CREATE("^([a-z]+)\\g{-1}$").match("abcabc"sv));
static_assert(!NOTRE_SYNTAX("^([a-z]+)\\g{-2}$"));
// TODO check for existence of named capture too

static_assert(NOTRE_CREATE("^(?<text>[a-z]+):\\g{text}$").match("abc:abc"sv));

static_assert(NOTRE_CREATE("^abc$").match("abc"sv));
static_assert(NOTRE_CREATE("^abc$").match(L"abc"sv));
// static_assert(NOTRE_CREATE("^abc$").match(u8"abc"sv)); // GCC9.0.1 doesn't support a char8_t string_view literals
static_assert(NOTRE_CREATE("^abc$").match(u"abc"sv));
static_assert(NOTRE_CREATE("^abc$").match(U"abc"sv));

static_assert(NOTRE_CREATE(R"(\(\))").match("()"sv));
static_assert(NOTRE_CREATE("\\[\\]").match("[]"sv));
static_assert(NOTRE_CREATE(R"(\[\])").match("[]"sv));

static_assert(NOTRE_CREATE(R"(\[([A-Z]*?)\])").match("[]"sv));
static_assert(NOTRE_CREATE(R"(\[([A-Z]*?)\])").match("[URL]"sv));

static_assert(NOTRE_CREATE(R"(\[([\s\S]*?)\]\(([\s\S]*?)\))").match("[URL](https://cpp.fail/notre)"));

static_assert(NOTRE_CREATE("abc").match("abc"));
static_assert(NOTRE_CREATE("[_]").match("_"));
static_assert(NOTRE_CREATE("[()]").match("("));
static_assert(NOTRE_CREATE("[$]").match("$"));
static_assert(NOTRE_CREATE("[*]").match("*"));
static_assert(NOTRE_CREATE("[+]").match("+"));
static_assert(NOTRE_CREATE("[?]").match("?"));
static_assert(NOTRE_CREATE("[{}]").match("{"));
static_assert(NOTRE_CREATE("[(-)]").match("("));
static_assert(NOTRE_CREATE("[(-)]").match(")"));

static_assert(NOTRE_CREATE("[A-Z_a-z]").match("a"));
static_assert(NOTRE_CREATE("[A-Z_a-z]").match("_"));
static_assert(NOTRE_CREATE("[A-Z_a-z]").match("Z"));
// FIXME: maybe in future I will allow this again
// static_assert(NOTRE_CREATE("[-]").match("-"));
// static_assert(NOTRE_CREATE("[-x]").match("x"));
// FIXME: due current limitation of LL1 grammar parser I can make this work "[x-]" without significant change in grammar
static_assert(NOTRE_CREATE("<").match("<"));
static_assert(NOTRE_CREATE("(<)").match("<"));
static_assert(NOTRE_CREATE("(<>)").match("<>"));
static_assert(NOTRE_CREATE("(<>?)").match("<"));
static_assert(NOTRE_CREATE("(<?>)").match(">"));
static_assert(NOTRE_CREATE("()").match(""));


static_assert((NOTRE_CREATE("[a-z]") >> NOTRE_CREATE("[0-9]")).match("a9"));
static_assert((NOTRE_CREATE("a") | NOTRE_CREATE("b")).match("a"));
static_assert((NOTRE_CREATE("a") | NOTRE_CREATE("b")).match("b"));
static_assert(!(NOTRE_CREATE("a") | NOTRE_CREATE("b")).match("c"));

static_assert(NOTRE_CREATE("((a)(b))").match("ab"sv).template get<0>() == "ab"sv);
static_assert(NOTRE_CREATE("((a)(b))").match("ab"sv).template get<1>() == "ab"sv);
static_assert(NOTRE_CREATE("((a)(b))").match("ab"sv).template get<2>() == "a"sv);
static_assert(NOTRE_CREATE("((a)(b))").match("ab"sv).template get<3>() == "b"sv);

static_assert(NOTRE_CREATE("^x(?=y)").search("xy"sv).template get<0>() == "x"sv);
static_assert(NOTRE_CREATE("^x(?!a)").search("xy"sv).template get<0>() == "x"sv);

static_assert(NOTRE_CREATE("a(?!3)[0-9]").match("a0"sv));
static_assert(NOTRE_CREATE("a(?!3)[0-9]").match("a9"sv));
static_assert(!NOTRE_CREATE("a(?!3)[0-9]").match("a3"sv));

static_assert(!NOTRE_CREATE(".*(.)\\g{1}.*").match("abcdefghijk"sv));
static_assert(NOTRE_CREATE(".*(.)\\g{1}.*").match("aabcdefghijk"sv));
static_assert(NOTRE_CREATE(".*(.)\\g{1}.*").match("abcdeffghijk"sv));

static_assert(NOTRE_CREATE("(?=.*(.)\\g{1})[a-z]+").match("abcdeffghijk"sv));
static_assert(!NOTRE_CREATE("(?=.*(.)\\g{1}{2})[a-z]+").match("abcddeffghijk"sv));
static_assert(NOTRE_CREATE("(?=.*(.)\\g{1}{2})[a-z]+").match("abcdddeffghijk"sv));

static_assert( NOTRE_CREATE("(?!.*(.)\\g{1})[a-z]+").match("abcdefgh"sv));
static_assert(!NOTRE_CREATE("(?!.*(.)\\g{1})[a-z]+").match("abcdeefgh"sv));

static_assert(NOTRE_CREATE("_").match("_"sv));
static_assert(NOTRE_CREATE("[<]").match("<"sv));
static_assert(NOTRE_CREATE("[>]").match(">"sv));
static_assert(NOTRE_CREATE("[<>]").match("<"sv));
static_assert(NOTRE_CREATE("[<>]+").match("><"sv));

static_assert(NOTRE_CREATE("<[a-z]+>").match("<aloha>"sv));
static_assert(NOTRE_CREATE("(<[a-z]+>)\\g{1}").match("<aloha><aloha>"sv));

// issue #60
static_assert(NOTRE_CREATE("[^\\^]").match("a"sv));
static_assert(NOTRE_CREATE("[^^]").match("a"sv));
static_assert(NOTRE_CREATE("[\\-]").match("-"sv));
static_assert(NOTRE_CREATE("[\\--\\-]").match("-"sv));

// msvc
static_assert(NOTRE_CREATE("[a-z]+abc").match("xxxabc"));
