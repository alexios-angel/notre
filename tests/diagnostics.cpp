// Diagnostics and constexpr-debugging tests. Compilation IS the test:
// every claim is a static_assert, so a clean build is a passing suite.
#include <notre.hpp>
#include <string_view>
#include <cstdio>

using namespace std::string_view_literals;

#if NOTRE_CNTTP_COMPILER_CHECK

// ---------------------------------------------------------------------------
// valid<> : a bool, never a hard error (the counterpart to match<>())
// ---------------------------------------------------------------------------
static_assert(notre::valid<"[a-z]+">);
static_assert(notre::valid<"(a|b)*c">);
static_assert(notre::valid<"\\d{2,4}">);
static_assert(!notre::valid<"a(b|c">);   // unclosed group
static_assert(!notre::valid<"[a-">);     // unclosed class
static_assert(!notre::valid<"a{2,1}b\\">); // trailing backslash

// ---------------------------------------------------------------------------
// error_info<> : kind, byte offset, line and column
// ---------------------------------------------------------------------------
static_assert(notre::error_info<"[a-z]+">().ok());
static_assert(notre::error_info<"[a-z]+">().kind == notre::error_kind::none);

constexpr auto unclosed = notre::error_info<"a(b|c">();
static_assert(!unclosed.ok());
static_assert(unclosed.kind == notre::error_kind::syntax);
static_assert(unclosed.position == 5);
static_assert(unclosed.line == 1 && unclosed.column == 6);
static_assert(notre::to_string(unclosed.kind) == "syntax error"sv);

// a multi-line pattern (extended mode style) resolves line/column
constexpr auto ml = notre::error_info<"abc\n[x-">();
static_assert(ml.kind == notre::error_kind::syntax);
static_assert(ml.line == 2 && ml.column == 4);

// ---------------------------------------------------------------------------
// error_message<> : the rendered diagnostic with a caret ("" when valid)
// ---------------------------------------------------------------------------
static_assert(notre::error_message<"[a-z]+">() == ""sv);
static_assert(notre::error_message<"a(b|c">() ==
              "notre: regex syntax error at position 5 (line 1, column 6)\n"
              "  a(b|c\n"
              "       ^"sv);

// ---------------------------------------------------------------------------
// dump_ast<> : the compiled atom tree (the "what did my regex become" tool)
// ---------------------------------------------------------------------------
static_assert(notre::debug::dump_ast<"a|b">() ==
              "alternation\n"
              "  character 'a'\n"
              "  character 'b'\n"sv);
static_assert(notre::debug::dump_ast<"x+">() ==
              "+ (one or more) greedy\n"
              "  character 'x'\n"sv);

// quantifier flavours are distinguished
static_assert(notre::debug::dump_ast<"a*?">().find("* (zero or more) lazy") != std::string_view::npos);
static_assert(notre::debug::dump_ast<"a++">().find("+ (one or more) possessive") != std::string_view::npos);
static_assert(notre::debug::dump_ast<"a{3}">().find("{3} (exactly 3) greedy") != std::string_view::npos);

// named captures, back-references, anchors, look-around, boundaries
static_assert(notre::debug::dump_ast<"(?<yr>[0-9]+)">().find("capture #1 <yr>") != std::string_view::npos);
static_assert(notre::debug::dump_ast<"(a)\\1">().find("back-reference \\1") != std::string_view::npos);
static_assert(notre::debug::dump_ast<"^ab$">().find("anchor ^ (line begin)") != std::string_view::npos);
static_assert(notre::debug::dump_ast<"^ab$">().find("anchor $ (line end)") != std::string_view::npos);
static_assert(notre::debug::dump_ast<"(?=x)">().find("lookahead (?=...)") != std::string_view::npos);
static_assert(notre::debug::dump_ast<"\\bword\\b">().find("word boundary \\b") != std::string_view::npos);
// \B is boundary<negative_set<word_chars>> - labelled, not dumped as a subtree
static_assert(notre::debug::dump_ast<"\\B">() == "not a word boundary \\B\n"sv);

// quotes are escaped against the ACTIVE delimiter only: " inside a
// double-quoted string<>, ' inside a single-quoted character<>
static_assert(notre::debug::dump_ast<"a\"b'c">() == "string \"a\\\"b'c\"\n"sv);
static_assert(notre::debug::dump_ast<"[\"']">().find("character '\"'") != std::string_view::npos);
static_assert(notre::debug::dump_ast<"[\"']">().find("character '\\''") != std::string_view::npos);

// an invalid pattern dumps the diagnostic instead of a tree
static_assert(notre::debug::dump_ast<"a(b|c">().find("regex syntax error") != std::string_view::npos);

// no pattern in this suite falls through to the generic <atom> label
static_assert(notre::debug::dump_ast<"^(?<n>a|b)+\\d\\1$">().find("<atom>") == std::string_view::npos);

#else

// ---------------------------------------------------------------------------
// C++17: patterns are constexpr fixed_string variables with linkage
// ---------------------------------------------------------------------------
static constexpr auto good = ctll::fixed_string{"[a-z]+"};
static constexpr auto bad = ctll::fixed_string{"a(b|c"};

static_assert(notre::valid<good>);
static_assert(!notre::valid<bad>);
static_assert(notre::error_info<good>().ok());

constexpr auto e = notre::error_info<bad>();
static_assert(e.kind == notre::error_kind::syntax);
static_assert(e.position == 5 && e.line == 1 && e.column == 6);
static_assert(notre::error_message<good>() == std::string_view{});
static_assert(notre::error_message<bad>() == std::string_view{
              "notre: regex syntax error at position 5 (line 1, column 6)\n"
              "  a(b|c\n"
              "       ^"});

static constexpr auto grp = ctll::fixed_string{"a(b|c)+"};
static_assert(notre::debug::dump_ast<grp>().find("alternation") != std::string_view::npos);
static_assert(notre::debug::dump_ast<grp>().find("<atom>") == std::string_view::npos);

#endif

int main() {
	// the queries also work at runtime; exercise one end to end
#if NOTRE_CNTTP_COMPILER_CHECK
	const std::string_view msg = notre::error_message<"a(b|c">();
#else
	static constexpr auto bad2 = ctll::fixed_string{"a(b|c"};
	const std::string_view msg = notre::error_message<bad2>();
#endif
	return msg.find('^') != std::string_view::npos ? 0 : 1;
}
