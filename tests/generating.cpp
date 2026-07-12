#include <notre.hpp>

void empty_symbol() { }

template <typename... T> struct id_type;

using namespace notre::test_literals;

#if !NOTRE_CNTTP_COMPILER_CHECK
#define NOTRE_GEN(pattern) (pattern ## _notre_gen)
#else


template <ctll::fixed_string input> constexpr auto gen() {
	constexpr auto _input = input;
	
	using tmp = typename ctll::parser<notre::pcre, _input, notre::pcre_actions>::template output<notre::pcre_context<>>;
	static_assert(tmp(), "Regular Expression contains syntax error.");
	return typename tmp::output_type::stack_type();
}


#define NOTRE_GEN(pattern) gen<pattern>()

#endif

template <typename A, typename B> constexpr auto same(A,B) -> std::false_type { return {}; }

template <typename A> constexpr auto same(A,A) -> std::true_type { return {}; }

template <typename A, typename B> constexpr bool same_f(A a,B b) {
	return same(ctll::front(a), b);
}

// basics
static_assert(same_f(NOTRE_GEN(""), notre::empty()));
static_assert(same_f(NOTRE_GEN("y"), notre::character<'y'>()));

// unicode
#if __cpp_char8_t
static_assert(same_f(NOTRE_GEN(u8"😍"), notre::character<U'😍'>()));
static_assert(same_f(NOTRE_GEN(u8"[😍]"), notre::set<notre::character<U'😍'>>()));
#endif
static_assert(same_f(NOTRE_GEN(U"😍"), notre::character<U'😍'>()));
static_assert(same_f(NOTRE_GEN(U"[😍]"), notre::set<notre::character<U'😍'>>()));

// stringification
static_assert(ctll::size(NOTRE_GEN("abc")) == 1);
static_assert(same_f(NOTRE_GEN("abc"), notre::string<'a','b','c'>()));
static_assert(same_f(NOTRE_GEN("(?:abc)"), notre::string<'a','b','c'>()));

// support for hexdec
static_assert(same_f(NOTRE_GEN("\\x40"), notre::character<char{0x40}>()));
static_assert(same_f(NOTRE_GEN("\\x7F"), notre::character<char{0x7F}>()));
// only characters with value < 128 are char otherwise they are internally char32_t
constexpr unsigned char_length = (std::numeric_limits<char>::max)();
constexpr bool char_is_unsigned = (char_length == 255);
// I wish I could have operator implication here :(
using expected_type = std::conditional_t<char_is_unsigned, char, char32_t>;
static_assert(same_f(NOTRE_GEN("\\x80"), notre::character<expected_type{0x80}>()));
static_assert(same_f(NOTRE_GEN("\\xFF"), notre::character<expected_type{0xFF}>()));
static_assert(same_f(NOTRE_GEN("\\x{FF}"), notre::character<expected_type{0xFF}>()));

static_assert(same_f(NOTRE_GEN("\\x{FFF}"), notre::character<char32_t{0xFFF}>()));
static_assert(same_f(NOTRE_GEN("\\x{ABCD}"), notre::character<char32_t{0xABCD}>()));

// anything
static_assert(same_f(NOTRE_GEN("."), notre::any()));

// sequence
static_assert(same_f(NOTRE_GEN("x.ab"), notre::sequence<notre::character<'x'>,notre::any,notre::string<'a','b'>>()));

// character class
static_assert(same_f(NOTRE_GEN("\\d"), notre::set<notre::digit_chars>()));
static_assert(same_f(NOTRE_GEN("\\D"), notre::negative_set<notre::digit_chars>()));
static_assert(same_f(NOTRE_GEN("\\n"), notre::character<'\n'>()));
static_assert(same_f(NOTRE_GEN("\\N"), notre::negative_set<notre::character<'\n'>>()));
static_assert(same_f(NOTRE_GEN("\\w"), notre::set<notre::word_chars>()));
static_assert(same_f(NOTRE_GEN("\\W"), notre::negative_set<notre::word_chars>()));
static_assert(same_f(NOTRE_GEN("\\s"), notre::set<notre::space_chars>()));
static_assert(same_f(NOTRE_GEN("\\S"), notre::negative_set<notre::space_chars>()));

// set support
static_assert(same_f(NOTRE_GEN("[a]"), notre::set<notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("[^a]"), notre::negative_set<notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("[[:digit:]]"), notre::set<notre::digit_chars>()));
static_assert(same_f(NOTRE_GEN("[ab]"), notre::set<notre::character<'a'>,notre::character<'b'>>()));
static_assert(same_f(NOTRE_GEN("[^ab]"), notre::negative_set<notre::character<'a'>,notre::character<'b'>>()));
static_assert(same_f(NOTRE_GEN("[[:digit:][:digit:]]"), notre::set<notre::digit_chars>()));
static_assert(same_f(NOTRE_GEN("[^[:punct:]]"), notre::negative_set<notre::punct_chars>()));
static_assert(same_f(NOTRE_GEN("[[:^digit:]]"), notre::set<notre::negate<notre::digit_chars>>()));
static_assert(same_f(NOTRE_GEN("[[:^digit:][:^alpha:]]"), notre::set<notre::negate<notre::digit_chars>, notre::negate<notre::alpha_chars>>()));
static_assert(same_f(NOTRE_GEN("[[:digit:][:alpha:]]"), notre::set<notre::digit_chars, notre::alpha_chars>()));
static_assert(same_f(NOTRE_GEN("[[:digit:][:^alpha:]]"), notre::set<notre::digit_chars, notre::negate<notre::alpha_chars>>()));
static_assert(same_f(NOTRE_GEN("[a-z]"), notre::set<notre::char_range<'a','z'>>()));
static_assert(same_f(NOTRE_GEN("[a-z0-9]"), notre::set<notre::char_range<'a','z'>,notre::char_range<'0','9'>>()));
static_assert(same_f(NOTRE_GEN("[^a-z]"), notre::negative_set<notre::char_range<'a','z'>>()));
static_assert(same_f(NOTRE_GEN("[^a-z0-9]"), notre::negative_set<notre::char_range<'a','z'>,notre::char_range<'0','9'>>()));
static_assert(same_f(NOTRE_GEN("[a-z[:digit:]]"), notre::set<notre::char_range<'a','z'>,notre::digit_chars>()));
static_assert(same_f(NOTRE_GEN("[a-z98]"), notre::set<notre::char_range<'a','z'>,notre::character<'9'>,notre::character<'8'>>()));
static_assert(same_f(NOTRE_GEN("[\\w]"), notre::set<notre::set<notre::word_chars>>()));
static_assert(same_f(NOTRE_GEN("[\\x30-\\x39]"), notre::set<notre::char_range<'\x30','\x39'>>()));

// alternation
static_assert(same_f(NOTRE_GEN("(?:abc|def)"), notre::select<notre::string<'a','b','c'>,notre::string<'d','e','f'>>()));
static_assert(same_f(NOTRE_GEN("(?:abc|def|ghi)"), notre::select<notre::string<'a','b','c'>,notre::string<'d','e','f'>,notre::string<'g','h','i'>>()));
static_assert(same_f(NOTRE_GEN("(?:a|b|c|d)"), notre::select<notre::character<'a'>,notre::character<'b'>,notre::character<'c'>,notre::character<'d'>>()));
static_assert(same_f(NOTRE_GEN("(?:a|b|c|)"), notre::select<notre::character<'a'>,notre::character<'b'>,notre::character<'c'>,notre::empty>()));
static_assert(same_f(NOTRE_GEN("(?:a|)"), notre::select<notre::character<'a'>, notre::empty>()));
static_assert(same_f(NOTRE_GEN("(?:|a)"), notre::select<notre::empty, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("(?:|)"), notre::select<notre::empty, notre::empty>()));
static_assert(same_f(NOTRE_GEN("(x|)"), notre::capture<1, notre::select<notre::character<'x'>, notre::empty>>()));
static_assert(same_f(NOTRE_GEN("(|x)"), notre::capture<1, notre::select<notre::empty, notre::character<'x'>>>()));
static_assert(same_f(NOTRE_GEN("(|)"), notre::capture<1, notre::select<notre::empty, notre::empty>>()));


// optional
static_assert(same_f(NOTRE_GEN("xx?"), notre::sequence<notre::character<'x'>,notre::optional<notre::character<'x'>>>()));
static_assert(same_f(NOTRE_GEN("xx?+"), notre::sequence<notre::character<'x'>,notre::possessive_optional<notre::character<'x'>>>()));
static_assert(same_f(NOTRE_GEN("xx??"), notre::sequence<notre::character<'x'>,notre::lazy_optional<notre::character<'x'>>>()));
static_assert(same_f(NOTRE_GEN("(?:abc)?"), notre::optional<notre::string<'a','b','c'>>()));
static_assert(same_f(NOTRE_GEN("(?:x)?"), notre::optional<notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("(?:x?)?"), notre::optional<notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("(?:x?)??"), notre::lazy_optional<notre::character<'x'>>()));
// the string is split due trigraph warning
static_assert(same_f(NOTRE_GEN("(?:x??" ")?"), notre::lazy_optional<notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("(?:x??" ")??"), notre::lazy_optional<notre::character<'x'>>()));

// repeat
static_assert(same_f(NOTRE_GEN("x+"), notre::plus<notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("(?:abc)+"), notre::plus<notre::string<'a','b','c'>>()));
static_assert(same_f(NOTRE_GEN("x*"), notre::star<notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("(?:abc)*"), notre::star<notre::string<'a','b','c'>>()));
static_assert(same_f(NOTRE_GEN("x{1}"), notre::repeat<1,1,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x{1,}"), notre::repeat<1,0,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x{198}"), notre::repeat<198,198,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x{198,}"), notre::repeat<198,0,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x{1,2}"), notre::repeat<1,2,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x{1,1234}"), notre::repeat<1,1234,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x{567,1234}"), notre::repeat<567,1234,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("[x]{567,1234}"), notre::repeat<567,1234,notre::set<notre::character<'x'>>>()));
static_assert(same_f(NOTRE_GEN("[^x]{567,1234}"), notre::repeat<567,1234,notre::negative_set<notre::character<'x'>>>()));
static_assert(same_f(NOTRE_GEN("(?:abc){3,42}"), notre::repeat<3,42,notre::string<'a','b','c'>>()));

// combine repeat
static_assert(same_f(NOTRE_GEN("x*x*"), notre::star<notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x+x+"), notre::repeat<2,0,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x+x*"), notre::plus<notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x*x+"), notre::plus<notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x{1,3}x{2,4}"), notre::repeat<3,7,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("a?a?a?"), notre::repeat<0,3,notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a?a?b"), notre::sequence<notre::repeat<0,2,notre::character<'a'>>, notre::character<'b'>>()));
static_assert(same(NOTRE_GEN("a?a?b"), NOTRE_GEN("(?:a?a?)b")));

// combine lazy repeat
static_assert(same_f(NOTRE_GEN("x*?x*?"), notre::lazy_star<notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x+?x+?"), notre::lazy_repeat<2,0,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x+?x*?"), notre::lazy_plus<notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x*?x+?"), notre::lazy_plus<notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("x{1,3}?x{2,4}?"), notre::lazy_repeat<3,7,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("a??a??a??"), notre::lazy_repeat<0,3,notre::character<'a'>>()));

// combine possessive
static_assert(same_f(NOTRE_GEN("a{0,0}+a{0,0}+"), notre::possessive_repeat<0,0, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{1,0}+a{0,0}+"), notre::possessive_repeat<1,0, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{0,0}+a{1,0}+"), notre::sequence<notre::reject, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{1,0}+a{1,0}+"), notre::sequence<notre::reject, notre::character<'a'>>()));

static_assert(same_f(NOTRE_GEN("a{0,1}+a{0,0}+"), notre::possessive_repeat<0,0, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{1,1}+a{0,0}+"), notre::possessive_repeat<1,0, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{0,1}+a{1,0}+"), notre::possessive_repeat<2,0, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{1,1}+a{1,0}+"), notre::possessive_repeat<2,0, notre::character<'a'>>()));

static_assert(same_f(NOTRE_GEN("a{0,0}+a{0,1}+"), notre::possessive_repeat<0,0, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{1,0}+a{0,1}+"), notre::possessive_repeat<1,0, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{0,0}+a{1,1}+"), notre::sequence<notre::reject, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{1,0}+a{1,1}+"), notre::sequence<notre::reject, notre::character<'a'>>()));

static_assert(same_f(NOTRE_GEN("a{0,1}+a{0,1}+"), notre::possessive_repeat<0,2, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{1,1}+a{0,1}+"), notre::possessive_repeat<1,2, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{0,1}+a{1,1}+"), notre::possessive_repeat<2,2, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{1,1}+a{1,1}+"), notre::possessive_repeat<2,2, notre::character<'a'>>()));

static_assert(same_f(NOTRE_GEN("a{1,2}+a{2,0}+"), notre::possessive_repeat<4,0, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{1,2}+a{1,2}+"), notre::possessive_repeat<3,4, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{1,1}+a{2,0}+"), notre::possessive_repeat<3,0, notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("a{1,1}+a{1,2}+"), notre::possessive_repeat<2,3, notre::character<'a'>>()));


// don't combine different types of repeats
static_assert(same_f(NOTRE_GEN("x+?x+"), notre::sequence<notre::lazy_plus<notre::character<'x'>>, notre::plus<notre::character<'x'>>>()));


// possessive/lazy
static_assert(same_f(NOTRE_GEN("c++"), notre::possessive_plus<notre::character<'c'>>()));
static_assert(same_f(NOTRE_GEN("c+?"), notre::lazy_plus<notre::character<'c'>>()));
static_assert(same_f(NOTRE_GEN("g*+"), notre::possessive_star<notre::character<'g'>>()));
static_assert(same_f(NOTRE_GEN("g*?"), notre::lazy_star<notre::character<'g'>>()));
static_assert(same_f(NOTRE_GEN("i??"), notre::lazy_optional<notre::character<'i'>>()));
static_assert(same_f(NOTRE_GEN("l{1,2}+"), notre::possessive_repeat<1,2,notre::character<'l'>>()));
static_assert(same_f(NOTRE_GEN("l{1,2}?"), notre::lazy_repeat<1,2,notre::character<'l'>>()));
static_assert(same_f(NOTRE_GEN("q{4,4}+"), notre::possessive_repeat<4,4,notre::character<'q'>>()));
static_assert(same_f(NOTRE_GEN("q{4,4}?"), notre::lazy_repeat<4,4,notre::character<'q'>>()));
// note: there is no possessive/lazy evaluationg for fixed number of repeats

// captures
static_assert(same_f(NOTRE_GEN("(a)"), notre::capture<1,notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("(x[cd])"), notre::capture<1,notre::character<'x'>, notre::set<notre::character<'c'>, notre::character<'d'>>>())); 
static_assert(same_f(NOTRE_GEN("(x[cd])(ab)"), notre::sequence<notre::capture<1,notre::character<'x'>, notre::set<notre::character<'c'>, notre::character<'d'>>>,notre::capture<2,notre::string<'a','b'>>>())); 
static_assert(same_f(NOTRE_GEN("(x[cd])(ab)+"), notre::sequence<notre::capture<1,notre::character<'x'>, notre::set<notre::character<'c'>, notre::character<'d'>>>,notre::plus<notre::capture<2,notre::string<'a','b'>>>>())); 
static_assert(same_f(NOTRE_GEN("(?<n>x)"), notre::capture_with_name<1,notre::id<'n'>,notre::character<'x'>>())); 
static_assert(same_f(NOTRE_GEN("(?<name>x)"), notre::capture_with_name<1,notre::id<'n','a','m','e'>,notre::character<'x'>>())); 
static_assert(same_f(NOTRE_GEN("(?<name>xy)"), notre::capture_with_name<1,notre::id<'n','a','m','e'>,notre::string<'x','y'>>())); 
static_assert(same_f(NOTRE_GEN("(?<name>x|y)"), notre::capture_with_name<1,notre::id<'n','a','m','e'>,notre::select<notre::character<'x'>,notre::character<'y'>>>())); 
static_assert(same_f(NOTRE_GEN("(?<xy>[x]y)"), notre::capture_with_name<1,notre::id<'x','y'>,notre::set<notre::character<'x'>>,notre::character<'y'>>())); 
static_assert(same_f(NOTRE_GEN("(?<xy>[x]y)(a)"), notre::sequence<notre::capture_with_name<1,notre::id<'x','y'>,notre::set<notre::character<'x'>>,notre::character<'y'>>, notre::capture<2,notre::character<'a'>>>()));

// (?P<name>...) and (?'name'...) build the same AST as (?<name>...)
static_assert(same_f(NOTRE_GEN("(?P<name>x)"), notre::capture_with_name<1,notre::id<'n','a','m','e'>,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("(?'name'x)"), notre::capture_with_name<1,notre::id<'n','a','m','e'>,notre::character<'x'>>()));
static_assert(same_f(NOTRE_GEN("(?P<name>x|y)"), notre::capture_with_name<1,notre::id<'n','a','m','e'>,notre::select<notre::character<'x'>,notre::character<'y'>>>()));
static_assert(same_f(NOTRE_GEN("(?'name'x|y)"), notre::capture_with_name<1,notre::id<'n','a','m','e'>,notre::select<notre::character<'x'>,notre::character<'y'>>>()));

// subroutine calls parse into placeholders (inlined later by resolve_subroutines)
static_assert(same_f(NOTRE_GEN("(x)(?1)"), notre::sequence<notre::capture<1,notre::character<'x'>>,notre::subroutine_call<1>>()));
static_assert(same_f(NOTRE_GEN("(x)(y)(?-2)"), notre::sequence<notre::capture<1,notre::character<'x'>>,notre::capture<2,notre::character<'y'>>,notre::subroutine_call<1>>()));
static_assert(same_f(NOTRE_GEN("(?+1)(x)"), notre::sequence<notre::subroutine_call<1>,notre::capture<1,notre::character<'x'>>>()));
static_assert(same_f(NOTRE_GEN("(?&name)"), notre::subroutine_call_with_name<notre::id<'n','a','m','e'>>()));
static_assert(same_f(NOTRE_GEN("(?P>name)"), notre::subroutine_call_with_name<notre::id<'n','a','m','e'>>()));
static_assert(same_f(NOTRE_GEN("\\g<name>"), notre::subroutine_call_with_name<notre::id<'n','a','m','e'>>()));
static_assert(same_f(NOTRE_GEN("\\g'name'"), notre::subroutine_call_with_name<notre::id<'n','a','m','e'>>()));
static_assert(same_f(NOTRE_GEN("\\g<1>"), notre::subroutine_call<1>()));
static_assert(same_f(NOTRE_GEN("\\g'2'"), notre::subroutine_call<2>()));

// match point reset \K is an ordinary zero-width atom in the AST
static_assert(same_f(NOTRE_GEN("a\\Kb"), notre::sequence<notre::character<'a'>,notre::match_point_reset,notre::character<'b'>>()));

// conditional patterns
static_assert(same_f(NOTRE_GEN("(a)(?(1)b|c)"), notre::sequence<notre::capture<1,notre::character<'a'>>,notre::condition_capture<1,notre::character<'b'>,notre::character<'c'>>>()));
static_assert(same_f(NOTRE_GEN("(a)(?(1)b)"), notre::sequence<notre::capture<1,notre::character<'a'>>,notre::condition_capture<1,notre::character<'b'>,notre::empty>>()));
static_assert(same_f(NOTRE_GEN("(?<w>a)(?(w)b|c)"), notre::sequence<notre::capture_with_name<1,notre::id<'w'>,notre::character<'a'>>,notre::condition_capture_with_name<notre::id<'w'>,notre::character<'b'>,notre::character<'c'>>>()));
// an assertion condition is rewritten into a guarded alternation
static_assert(same_f(NOTRE_GEN("(?(?=x)a|b)"), notre::select<notre::sequence<notre::lookahead_positive<notre::character<'x'>>,notre::character<'a'>>,notre::sequence<notre::lookahead_negative<notre::character<'x'>>,notre::character<'b'>>>()));
// DEFINE keeps its body only for subroutine lookups
static_assert(same_f(NOTRE_GEN("(?(DEFINE)(?<d>x))"), notre::define_group<notre::capture_with_name<1,notre::id<'d'>,notre::character<'x'>>>()));

// callouts parse into unbound zero-width markers
static_assert(same_f(NOTRE_GEN("(?C)"), notre::callout_numbered<0>()));
static_assert(same_f(NOTRE_GEN("(?C42)"), notre::callout_numbered<42>()));
static_assert(same_f(NOTRE_GEN("(?C'ab')"), notre::callout_named<notre::id<'a','b'>>()));
static_assert(same_f(NOTRE_GEN("(?C\"ab\")"), notre::callout_named<notre::id<'a','b'>>()));
static_assert(same_f(NOTRE_GEN("x(?C1)y"), notre::sequence<notre::character<'x'>,notre::callout_numbered<1>,notre::character<'y'>>()));

// octal and control character escapes produce plain characters
static_assert(same_f(NOTRE_GEN("\\o{101}"), notre::character<'A'>()));
static_assert(same_f(NOTRE_GEN("\\077"), notre::character<'?'>()));
static_assert(same_f(NOTRE_GEN("\\0"), notre::character<'\0'>()));
static_assert(same_f(NOTRE_GEN("\\cA"), notre::character<'\x01'>()));
static_assert(same_f(NOTRE_GEN("\\cj"), notre::character<'\x0A'>()));
static_assert(same_f(NOTRE_GEN("\\c?"), notre::character<'\x7F'>()));

static_assert(same_f(NOTRE_GEN("()"), notre::capture<1,notre::empty>()));
static_assert(same_f(NOTRE_GEN("(a)(b)"), notre::sequence<notre::capture<1,notre::character<'a'>>,notre::capture<2,notre::character<'b'>>>()));
static_assert(same_f(NOTRE_GEN("((a)(b))"), notre::capture<1,notre::capture<2,notre::character<'a'>>,notre::capture<3,notre::character<'b'>>>()));
static_assert(same_f(NOTRE_GEN("(((a)(b)))"),notre::capture<1, notre::capture<2,notre::capture<3,notre::character<'a'>>,notre::capture<4,notre::character<'b'>>>>()));

static_assert(same_f(NOTRE_GEN("((?:a)(b))"), notre::capture<1,notre::character<'a'>,notre::capture<2,notre::character<'b'>>>()));


// backreference
static_assert(same_f(NOTRE_GEN("(a)\\g{1}"), notre::sequence<notre::capture<1,notre::character<'a'>>, notre::back_reference<1>>()));
static_assert(same_f(NOTRE_GEN("(?<ab>a)\\g{ab}"), notre::sequence<notre::capture_with_name<1,notre::id<'a','b'>,notre::character<'a'>>, notre::back_reference_with_name<notre::id<'a','b'>>>()));
static_assert(same_f(NOTRE_GEN("((a)\\g{2})"), notre::capture<1, notre::capture<2,notre::character<'a'>>, notre::back_reference<2>>()));

// asserts
static_assert(same_f(NOTRE_GEN("^"), notre::assert_line_begin()));
static_assert(same_f(NOTRE_GEN("$"), notre::assert_line_end()));
static_assert(same_f(NOTRE_GEN("^$"), notre::sequence<notre::assert_line_begin, notre::assert_line_end>()));
static_assert(same_f(NOTRE_GEN("\\A"), notre::assert_subject_begin()));
static_assert(same_f(NOTRE_GEN("\\Z"), notre::assert_subject_end_line()));
static_assert(same_f(NOTRE_GEN("\\z"), notre::assert_subject_end()));

static_assert(same_f(NOTRE_GEN("^abc$"), notre::sequence<notre::assert_line_begin, notre::string<'a','b','c'>, notre::assert_line_end>()));
static_assert(same_f(NOTRE_GEN("abc?"), notre::sequence<notre::string<'a','b'>, notre::optional<notre::character<'c'>>>()));
static_assert(same_f(NOTRE_GEN("abc."), notre::sequence<notre::string<'a','b','c'>, notre::any>()));
static_assert(same_f(NOTRE_GEN("abc.?"), notre::sequence<notre::string<'a','b','c'>, notre::optional<notre::any>>()));
static_assert(same_f(NOTRE_GEN("abc.def"), notre::sequence<notre::string<'a','b','c'>, notre::any, notre::string<'d','e','f'>>()));

static_assert(same_f(NOTRE_GEN("^a|b$"), notre::select<notre::sequence<notre::assert_line_begin, notre::character<'a'>>, notre::sequence<notre::character<'b'>, notre::assert_line_end>>()));

// atomic group
static_assert(same_f(NOTRE_GEN("(?>a)"), notre::atomic_group<notre::character<'a'>>())); 
static_assert(same_f(NOTRE_GEN("(?>ab)"), notre::atomic_group<notre::string<'a','b'>>())); 
static_assert(same_f(NOTRE_GEN("(?>a+b)"), notre::atomic_group<notre::plus<notre::character<'a'>>,notre::character<'b'>>())); 

// lookahead positive/negative
static_assert(same_f(NOTRE_GEN("(?=a)"), notre::lookahead_positive<notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("(?=ax)"), notre::lookahead_positive<notre::string<'a','x'>>()));
static_assert(same_f(NOTRE_GEN("(?=[a]x)"), notre::lookahead_positive<notre::set<notre::character<'a'>>,notre::character<'x'>>()));

static_assert(same_f(NOTRE_GEN("(?!a)"), notre::lookahead_negative<notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("(?!ax)"), notre::lookahead_negative<notre::string<'a','x'>>()));
static_assert(same_f(NOTRE_GEN("(?![a]x)"), notre::lookahead_negative<notre::set<notre::character<'a'>>,notre::character<'x'>>()));

static_assert(same_f(NOTRE_GEN("^(?=(a))$"), notre::sequence<notre::assert_line_begin, notre::lookahead_positive<notre::capture<1,notre::character<'a'>>>, notre::assert_line_end>()));

static_assert(same_f(NOTRE_GEN("^(?=.*(a))$"), notre::sequence<notre::assert_line_begin, notre::lookahead_positive<notre::star<notre::any>,notre::capture<1,notre::character<'a'>>>, notre::assert_line_end>()));

static_assert(same_f(NOTRE_GEN("^(?=.*(a).*)$"), notre::sequence<notre::assert_line_begin, notre::lookahead_positive<notre::star<notre::any>,notre::capture<1,notre::character<'a'>>, notre::star<notre::any>>, notre::assert_line_end>()));

static_assert(same_f(NOTRE_GEN("^(?=.*(a)\\g{1}.*)$"), notre::sequence<notre::assert_line_begin, notre::lookahead_positive<notre::star<notre::any>,notre::capture<1,notre::character<'a'>>, notre::back_reference<1>, notre::star<notre::any>>, notre::assert_line_end>()));

static_assert(same_f(NOTRE_GEN("^(?=.*(a)\\g{1}.*)[a-z]$"), notre::sequence<notre::assert_line_begin, notre::lookahead_positive<notre::star<notre::any>,notre::capture<1,notre::character<'a'>>, notre::back_reference<1>, notre::star<notre::any>>, notre::set<notre::char_range<'a','z'>>, notre::assert_line_end>()));

// lookbehind positive/negative
static_assert(same_f(NOTRE_GEN("(?<=a)"), notre::lookbehind_positive<notre::character<'a'>>()));
static_assert(same_f(NOTRE_GEN("(?<!a)"), notre::lookbehind_negative<notre::character<'a'>>()));

static_assert(same_f(NOTRE_GEN("(?<=ab)"), notre::lookbehind_positive<notre::string<'b','a'>>()));
static_assert(same_f(NOTRE_GEN("(?<!ab)"), notre::lookbehind_negative<notre::string<'b','a'>>()));

static_assert(same_f(NOTRE_GEN("(?<=ab|cd)"), notre::lookbehind_positive<notre::select<notre::string<'b','a'>, notre::string<'d','c'>>>()));
static_assert(same_f(NOTRE_GEN("(?<!ab|cd)"), notre::lookbehind_negative<notre::select<notre::string<'b','a'>, notre::string<'d','c'>>>()));

static_assert(same_f(NOTRE_GEN("(?<=(ab))"), notre::lookbehind_positive<notre::capture<1, notre::string<'b','a'>>>()));
static_assert(same_f(NOTRE_GEN("(?<!(ab))"), notre::lookbehind_negative<notre::capture<1, notre::string<'b','a'>>>()));
