#include <notre.hpp>

void empty_symbol() { }

using namespace notre::test_literals;

#if !NOTRE_CNTTP_COMPILER_CHECK
#define NOTRE_TEST(pattern) (pattern ## _notre_test)
#else

template <ctll::fixed_string input> constexpr bool test() {
	constexpr auto _input = input;
	return ctll::parser<notre::pcre, _input>::template correct_with<>;
}

#define NOTRE_TEST(pattern) test<pattern>()
#endif


// basics
static_assert(NOTRE_TEST(""));
static_assert(NOTRE_TEST("."));
static_assert(NOTRE_TEST("a"));
static_assert(NOTRE_TEST("ab"));
static_assert(NOTRE_TEST("a|b"));
static_assert(NOTRE_TEST("a|"));
static_assert(NOTRE_TEST("|b"));
static_assert(NOTRE_TEST("|"));
static_assert(NOTRE_TEST("n"));

// repetation
static_assert(NOTRE_TEST("x?")); // optional
static_assert(NOTRE_TEST("x??")); // optional lazy
static_assert(NOTRE_TEST("x?+")); // optional possessive
static_assert(NOTRE_TEST("x*")); 
static_assert(NOTRE_TEST("x*+"));
static_assert(NOTRE_TEST("x*?"));
static_assert(NOTRE_TEST("x+"));
static_assert(NOTRE_TEST("x++"));
static_assert(NOTRE_TEST("x+?"));
static_assert(NOTRE_TEST("x{1}"));
static_assert(NOTRE_TEST("x{1,4}"));
static_assert(NOTRE_TEST("x{1,4}+"));
static_assert(NOTRE_TEST("x{1,4}?"));
static_assert(NOTRE_TEST("x{1,}"));
static_assert(NOTRE_TEST("x{1,}+"));
static_assert(NOTRE_TEST("x{1,}?"));

// character sets
static_assert(NOTRE_TEST("[abc]")); // [abc]
static_assert(NOTRE_TEST("[a-c]")); // [abc]
static_assert(NOTRE_TEST("[a-ce-g]")); // [abcefg]
static_assert(NOTRE_TEST("[xa-c]")); // [xabc]
static_assert(NOTRE_TEST("[a-cy]")); // [abcy]
static_assert(NOTRE_TEST("\\d")); // digit
static_assert(NOTRE_TEST("\\D")); // non-digit
static_assert(NOTRE_TEST("\\s")); // whitespace
static_assert(NOTRE_TEST("\\S")); // non-whitespace
static_assert(NOTRE_TEST("\\w")); // word character
static_assert(NOTRE_TEST("\\W")); // non-word character

// assertions
static_assert(NOTRE_TEST("^"));
static_assert(NOTRE_TEST("$"));

// special characters
static_assert(NOTRE_TEST("\\n")); // new line
static_assert(NOTRE_TEST("\\r")); // return carriage
static_assert(NOTRE_TEST("\\t")); // tab
static_assert(NOTRE_TEST("\\0")); // NULL
static_assert(NOTRE_TEST("\\xAB"));
static_assert(NOTRE_TEST("\\x{A}"));
static_assert(NOTRE_TEST("\\x{ABCDEF}"));
static_assert(not NOTRE_TEST("\\x{ABCDEFG}"));

// named sets
static_assert(NOTRE_TEST("[[:alpha:][:digit:]]"));
static_assert(NOTRE_TEST("[^[:alpha:][:digit:]]"));
static_assert(NOTRE_TEST("[[:^alpha:][:^digit:]]"));
static_assert(NOTRE_TEST("[[:alpha:][:digit:]]+"));

// captures
static_assert(NOTRE_TEST("<not capture>"));
static_assert(NOTRE_TEST("(nonname)"));
static_assert(NOTRE_TEST("(non<name>)"));
static_assert(NOTRE_TEST("(?:noncapture)"));
static_assert(NOTRE_TEST("(?:non<cap>ture)"));
static_assert(NOTRE_TEST("(?<hello>:named)"));

// LIBC++ tests
static_assert(NOTRE_TEST(""));
static_assert(NOTRE_TEST("(.*).*"));
static_assert(NOTRE_TEST("(?=^)b")); // positive lookahead
static_assert(NOTRE_TEST("(a*)*"));
static_assert(NOTRE_TEST("(ab)*c"));
static_assert(NOTRE_TEST("(tour|to|t)+"));
static_assert(NOTRE_TEST("(tour|to|tournament)+"));
static_assert(NOTRE_TEST("(tour|to|tournament)+\ntourna"));
static_assert(NOTRE_TEST("-.*,.*-"));

static_assert(NOTRE_TEST("-\\(.*\\),\\1-"));
static_assert(NOTRE_TEST("A[[:lower:]]B"));
static_assert(NOTRE_TEST("A[^[:lower:]0-9]B"));
static_assert(NOTRE_TEST("A[^[:lower:]]B"));
//static_assert(NOTRE_TEST("Jeff(?!s\\b)")); // negative lookahead
//static_assert(NOTRE_TEST("Jeff(?=s\\b)")); // positive lookahead
static_assert(NOTRE_TEST("[\\n]+"));
static_assert(NOTRE_TEST("[^\\0]*"));
static_assert(NOTRE_TEST("[^\\W]"));
//static_assert(NOTRE_TEST("[a[.ch.]z]"));
//static_assert(NOTRE_TEST("[a[.hyphen.]z]"));
//static_assert(NOTRE_TEST("[a[=M=]z]"));
static_assert(NOTRE_TEST("[abfor]*"));
static_assert(NOTRE_TEST("[ace1-9]*"));
static_assert(NOTRE_TEST("[ace1-9]+"));
static_assert(NOTRE_TEST("[ace1-9]\\{1,\\}"));
static_assert(NOTRE_TEST("[ace1-9]{1,}"));
static_assert(NOTRE_TEST("\\(.*\\).*"));
static_assert(NOTRE_TEST("\\(a*\\)*"));
static_assert(NOTRE_TEST("\\(ab\\)*c"));
static_assert(NOTRE_TEST("\\d[\\W]k"));
static_assert(NOTRE_TEST("^[a-f]$"));
static_assert(NOTRE_TEST("^[a]$"));
static_assert(NOTRE_TEST("^[ab]$"));
static_assert(NOTRE_TEST("/"));
static_assert(NOTRE_TEST("[/]"));
static_assert(NOTRE_TEST("[\\[]"));
static_assert(NOTRE_TEST("[\\[-a]"));
static_assert(NOTRE_TEST("[\\[-\\[]"));
static_assert(NOTRE_TEST("[/-/]"));
static_assert(NOTRE_TEST("[!-/]"));
static_assert(NOTRE_TEST("[|]"));
static_assert(NOTRE_TEST("^abc"));
static_assert(NOTRE_TEST("a"));
static_assert(NOTRE_TEST("a(?=^)b")); // positive lookahead
static_assert(NOTRE_TEST("a*"));
static_assert(NOTRE_TEST("a*?"));
static_assert(NOTRE_TEST("a.\\{3,5\\}c"));
static_assert(NOTRE_TEST("a.c"));
static_assert(NOTRE_TEST("a.{3,5}c"));
static_assert(NOTRE_TEST("a\\{100000000000000000\\}"));
static_assert(NOTRE_TEST("ab"));
static_assert(NOTRE_TEST("ab*c"));
static_assert(NOTRE_TEST("ab\\{3,5\\}c"));
static_assert(NOTRE_TEST("abc$"));
static_assert(NOTRE_TEST("ab{3,5}c"));
static_assert(NOTRE_TEST("a{100000000000000000}"));
static_assert(NOTRE_TEST("b*"));
static_assert(NOTRE_TEST("ba"));
static_assert(NOTRE_TEST("bc"));
static_assert(NOTRE_TEST("cd((e)fg)hi"));
static_assert(NOTRE_TEST("cd((e)fg)hi|(z)"));
static_assert(NOTRE_TEST("cd\\(\\(e\\)fg\\)hi"));
static_assert(NOTRE_TEST("cdefghi"));
static_assert(NOTRE_TEST("q[^u]"));
static_assert(NOTRE_TEST("tour\n\ntournament"));
static_assert(NOTRE_TEST("tour\nto\ntournament"));
static_assert(NOTRE_TEST("tour|to|tournament"));
static_assert(NOTRE_TEST("z"));
static_assert(NOTRE_TEST("=!"));
static_assert(NOTRE_TEST("[=!]"));
static_assert(NOTRE_TEST("(=!)"));
static_assert(NOTRE_TEST("(?!test)"));
static_assert(NOTRE_TEST("(?=test)"));
static_assert(NOTRE_TEST("^(?=.*(.)\\g{1}+.*)[a-z]+"));
static_assert(NOTRE_TEST("^(?=.*(a)\\g{1}.*)$"));

static_assert(NOTRE_TEST("\\b"));
static_assert(NOTRE_TEST("\\A"));
static_assert(!NOTRE_TEST("[\\A]"));
static_assert(!NOTRE_TEST("\\i")); // this is not existing backslash
static_assert(NOTRE_TEST("(.*)\\1"));

// comments
static_assert(NOTRE_TEST("(?#comment)"));
static_assert(NOTRE_TEST("(?#)"));
static_assert(NOTRE_TEST("a(?#comment)b"));
static_assert(NOTRE_TEST("(?#anything but a close paren: [{(^$.*+?|\\)"));
static_assert(!NOTRE_TEST("(?#unterminated"));
static_assert(!NOTRE_TEST("(?#x))")); // ends at the first close paren; stray paren is an error

// named capture syntaxes (issue #325)
static_assert(NOTRE_TEST("(?<name>a)"));
static_assert(NOTRE_TEST("(?P<name>a)"));
static_assert(NOTRE_TEST("(?'name'a)"));
static_assert(!NOTRE_TEST("(?P[a])")); // P requires <name> or >name
static_assert(!NOTRE_TEST("(?P'name'a)")); // P only takes the angle form
static_assert(!NOTRE_TEST("(?'name)")); // unterminated name quote
static_assert(!NOTRE_TEST("(?''a)")); // empty name
static_assert(!NOTRE_TEST("(?P<name'a)")); // mismatched delimiters

// subroutine call syntaxes
static_assert(NOTRE_TEST("(?1)"));
static_assert(NOTRE_TEST("(?12)"));
static_assert(NOTRE_TEST("(?-1)"));
static_assert(NOTRE_TEST("(?+1)"));
static_assert(NOTRE_TEST("(?&name)"));
static_assert(NOTRE_TEST("(?P>name)"));
static_assert(NOTRE_TEST("\\g<name>"));
static_assert(NOTRE_TEST("\\g'name'"));
static_assert(NOTRE_TEST("\\g<1>"));
static_assert(NOTRE_TEST("\\g'1'"));
static_assert(!NOTRE_TEST("(?R)")); // whole-pattern recursion is not supported
static_assert(!NOTRE_TEST("(?&)")); // missing name
static_assert(!NOTRE_TEST("\\g<name")); // unterminated
static_assert(!NOTRE_TEST("\\g<>")); // empty
static_assert(!NOTRE_TEST("(?P>name")); // unterminated

// action-level rejects (need the semantic actions, not just the grammar)
#if !NOTRE_CNTTP_COMPILER_CHECK
#define NOTRE_SYNTAX(pattern) (pattern ## _notre_syntax)
#else
template <ctll::fixed_string input> constexpr bool syntax_test() {
	constexpr auto _input = input;
	return ctll::parser<notre::pcre, _input, notre::pcre_actions>::template correct_with<notre::pcre_context<>>;
}
#define NOTRE_SYNTAX(pattern) syntax_test<pattern>()
#endif

static_assert(!NOTRE_SYNTAX("(?0)")); // recursion of the whole pattern
static_assert(!NOTRE_SYNTAX("(?-1)")); // no group defined before the call
static_assert(!NOTRE_SYNTAX("(a)(?-2)")); // reaches before the first group
static_assert(NOTRE_SYNTAX("(a)(?-1)"));
static_assert(NOTRE_SYNTAX("(a)(?+1)(b)"));

// match point reset
static_assert(NOTRE_TEST("a\\Kb"));
static_assert(NOTRE_TEST("\\K"));
static_assert(!NOTRE_TEST("[\\K]")); // not an escape inside a class

// conditional patterns
static_assert(NOTRE_TEST("(?(1)a|b)"));
static_assert(NOTRE_TEST("(?(1)a)"));
static_assert(NOTRE_TEST("(?(1)|b)"));
static_assert(NOTRE_TEST("(?(-1)a|b)"));
static_assert(NOTRE_TEST("(?(+1)a|b)"));
static_assert(NOTRE_TEST("(?(<name>)a|b)"));
static_assert(NOTRE_TEST("(?('name')a|b)"));
static_assert(NOTRE_TEST("(?(name)a|b)"));
static_assert(NOTRE_TEST("(?(?=x)a|b)"));
static_assert(NOTRE_TEST("(?(?!x)a|b)"));
static_assert(NOTRE_TEST("(?(?<=x)a|b)"));
static_assert(NOTRE_TEST("(?(?<!x)a|b)"));
static_assert(NOTRE_TEST("(?(DEFINE)(?<d>x))"));
static_assert(!NOTRE_TEST("(?()a)")); // empty condition
static_assert(!NOTRE_TEST("(?(1)a|b|c)")); // more than two branches
static_assert(!NOTRE_TEST("(?(1)a|b")); // unterminated

// conditional action-level rejects
static_assert(!NOTRE_SYNTAX("(?(0)a)")); // no group zero
static_assert(!NOTRE_SYNTAX("(?(2)a)")); // group must already exist
static_assert(!NOTRE_SYNTAX("(?(-1)a)")); // nothing behind
static_assert(!NOTRE_SYNTAX("(?(R)a|b)")); // recursion condition unsupported
static_assert(!NOTRE_SYNTAX("(?(DEFINE)a|b)")); // DEFINE cannot have an alternative
static_assert(NOTRE_SYNTAX("(a)(?(1)b|c)"));
static_assert(NOTRE_SYNTAX("(a)(?(+1)x)(b)"));

// octal escapes
static_assert(NOTRE_TEST("\\o{101}"));
static_assert(NOTRE_TEST("\\0"));
static_assert(NOTRE_TEST("\\07"));
static_assert(NOTRE_TEST("\\077"));
static_assert(NOTRE_TEST("[\\o{40}-\\o{176}]"));
static_assert(!NOTRE_TEST("\\o{}")); // empty
static_assert(!NOTRE_TEST("\\o{18}")); // 8 is not an octal digit
static_assert(!NOTRE_TEST("\\o101")); // braces are required

// control characters
static_assert(NOTRE_TEST("\\cA"));
static_assert(NOTRE_TEST("\\cz"));
static_assert(NOTRE_TEST("\\c["));
static_assert(NOTRE_TEST("\\c\\"));
static_assert(NOTRE_TEST("\\c?"));
static_assert(NOTRE_TEST("[\\cA-\\cZ]"));
static_assert(!NOTRE_TEST("\\c")); // needs a following character
static_assert(!NOTRE_TEST("\\c5")); // digits are not control names

// grapheme cluster
static_assert(NOTRE_TEST("\\X"));
static_assert(NOTRE_TEST("\\X+"));
static_assert(!NOTRE_TEST("[\\X]")); // not valid inside a class

// callouts
static_assert(NOTRE_TEST("(?C)"));
static_assert(NOTRE_TEST("(?C0)"));
static_assert(NOTRE_TEST("(?C255)"));
static_assert(NOTRE_TEST("(?C'name')"));
static_assert(NOTRE_TEST("(?C\"name\")"));
static_assert(NOTRE_TEST("(?C'a b c!')")); // any characters except the delimiter
static_assert(!NOTRE_TEST("(?C'')")); // empty name
static_assert(!NOTRE_TEST("(?Cx)")); // neither number nor delimited name
static_assert(!NOTRE_TEST("(?C'unterminated"));
static_assert(!NOTRE_TEST("(?C7"));
static_assert(!NOTRE_TEST("(?C'mixed\")")); // mismatched delimiters


