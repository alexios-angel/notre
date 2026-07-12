// Conditional patterns: (?(condition)yes|no) picks a branch by whether a
// group participated in the match, or by an assertion.
//
//   (?(1)yes|no)       group 1 matched?  ((?(-1) / (?(+1) relative)
//   (?(<name>)yes|no)  named group ((?('name') and (?(name) work too)
//   (?(?=...)yes|no)   assertion condition
//
// Build: make conditionals

#include <notre.hpp>
#include <iostream>
#include <optional>
#include <string_view>

using namespace std::literals;

// the classic: a value that is quoted only if it starts with a quote
constexpr auto quoted_or_bare = notre::match<"(\")?(?(1)[^\"]*\"|[a-z]+)">;

static_assert(quoted_or_bare("\"hello world\""sv));
static_assert(quoted_or_bare("hello"sv));
static_assert(!quoted_or_bare("\"unterminated"sv)); // quote seen -> closing quote required

// assertion condition: pick the branch by lookahead
static_assert(notre::match<"(?(?=ab)a.|b.)">("ab"sv));
static_assert(notre::match<"(?(?=ab)a.|b.)">("bx"sv));

// captures inside branches are real captures
constexpr std::optional<char> which_branch(std::string_view s) {
	if (auto m = notre::match<"(a)?(?(1)(x)|(y))">(s)) {
		if (m.get<2>()) return 'x';
		if (m.get<3>()) return 'y';
	}
	return std::nullopt;
}
static_assert(which_branch("ax"sv) == 'x');
static_assert(which_branch("y"sv) == 'y');

int main() {
	std::cout << std::boolalpha;
	for (auto s : {"\"hi there\""sv, "hi"sv, "\"oops"sv}) {
		std::cout << "quoted_or_bare(" << s << "): " << bool(quoted_or_bare(s)) << "\n";
	}

	// optional area code, but if present it must be followed by a dash
	constexpr auto phone = notre::match<"([0-9]{3})?(?(1)-)[0-9]{3}-[0-9]{4}">;
	std::cout << "phone '555-123-4567':  " << bool(phone("555-123-4567"sv)) << "\n";
	std::cout << "phone '123-4567':      " << bool(phone("123-4567"sv)) << "\n";
	std::cout << "phone '555123-4567':   " << bool(phone("555123-4567"sv)) << "\n";
}
