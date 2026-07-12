// Match point reset \K: everything before it is required to match but is
// excluded from the reported match. Where lookbehind needs a fixed-length
// pattern, the part before \K can be anything.
//
// Build: make match-point-reset

#include <notre.hpp>
#include <iostream>
#include <string_view>

using namespace std::literals;

// the prefix is required...
static_assert(notre::search<"foo\\Kbar">("xxfoobarxx"sv));
static_assert(!notre::search<"foo\\Kbar">("xxbarxx"sv));
// ...but not reported
static_assert(notre::search<"foo\\Kbar">("xxfoobarxx"sv).to_view() == "bar"sv);

// a variable-length prefix, which (?<=...) cannot express
static_assert(notre::search<"[0-9]+\\K[a-z]+">("123abc"sv).to_view() == "abc"sv);

// captures taken before \K still work
static_assert(notre::match<"(a)\\K(b)">("ab"sv).get<1>() == "a"sv);
static_assert(notre::match<"(a)\\K(b)">("ab"sv).to_view() == "b"sv);

int main() {
	// extract values after a key without reporting the key
	constexpr auto input = "width=800 height=600 depth=32"sv;
	std::cout << "values:";
	for (auto m : notre::search_all<"[a-z]+=\\K[0-9]+">(input)) {
		std::cout << " " << m.to_view();
	}
	std::cout << "\n";

	// only in the taken alternation branch does \K apply
	std::cout << "a\\Kb|cd on 'cd' reports: '"
	          << notre::match<"(?:a\\Kb|cd)">("cd"sv).to_view() << "'\n";
	std::cout << "a\\Kb|cd on 'ab' reports: '"
	          << notre::match<"(?:a\\Kb|cd)">("ab"sv).to_view() << "'\n";
}
