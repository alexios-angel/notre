// Subroutine calls: re-execute a group's PATTERN (unlike a backreference,
// which re-matches the group's captured text).
//
//   (?1) (?2) ...    call group by number ((?-1)/(?+1) relative)
//   (?&name)         call group by name (also (?P>name), \g<name>, \g'name')
//   (?(DEFINE)...)   a group container that never matches by itself;
//                    its groups exist to be called
//
// Calls are inlined at compile time as atomic, non-capturing copies of
// the called group, so recursion is a compile error - but pattern
// libraries via DEFINE work beautifully.
//
// Build: make subroutines

#include <ctre.hpp>
#include <iostream>
#include <string_view>

using namespace std::literals;

// call by number: the same pattern matched twice, not the same text
static_assert(ctre::match<"([ab]c)(?1)">("acbc"sv));   // "ac" then "bc"
static_assert(!ctre::match<"([ab]c)\\g{1}">("acbc"sv)); // backreference: text must repeat

// captures set inside a call revert when it returns (calls do not capture)
static_assert(ctre::match<"([ab]c)(?1)">("acbc"sv).get<1>() == "ac"sv);

// calls are atomic, like PCRE: no backtracking into a finished call
static_assert(ctre::match<"(a|ab)z(?1)c">("azac"sv));
static_assert(!ctre::match<"(a|ab)z(?1)c">("azabc"sv));

// a DEFINE library: named building blocks, used by calling them
constexpr auto is_date = ctre::match<
	"(?(DEFINE)"
		"(?<d2>[0-9][0-9])"
		"(?<d4>(?&d2)(?&d2))"   // definitions can call each other
	")"
	"(?&d4)-(?&d2)-(?&d2)">;

static_assert(is_date("2026-07-10"sv));
static_assert(!is_date("26-07-10"sv));

int main() {
	std::cout << std::boolalpha;
	std::cout << "([ab]c)(?1) on 'acbc':      " << bool(ctre::match<"([ab]c)(?1)">("acbc"sv)) << "\n";
	std::cout << "([ab]c)\\g{1} on 'acbc':     " << bool(ctre::match<"([ab]c)\\g{1}">("acbc"sv)) << " (backreference needs repeated text)\n";
	std::cout << "is_date('2026-07-10'):      " << bool(is_date("2026-07-10"sv)) << "\n";
	std::cout << "is_date('2026-7-10'):       " << bool(is_date("2026-7-10"sv)) << "\n";

	// named call, all four syntaxes
	std::cout << "(?<w>ab)(?&w) on 'abab':    " << bool(ctre::match<"(?<w>ab)(?&w)">("abab"sv)) << "\n";
	std::cout << "(?<w>ab)\\g<w> on 'abab':    " << bool(ctre::match<"(?<w>ab)\\g<w>">("abab"sv)) << "\n";
}
