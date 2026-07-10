// Basic matching: ctre::match, ctre::search, ctre::starts_with.
//
// match       - the whole input must match the pattern
// search      - the pattern may match anywhere inside the input
// starts_with - the input must begin with a match (rest is ignored)
//
// All three return a regex_results object convertible to bool, and all
// work in constexpr context.
//
// Build: make match-basics   (or: clang++ -std=c++20 -I../include match-basics.cpp)

#include <ctre.hpp>
#include <iostream>
#include <string_view>

using namespace std::literals;

// everything is constexpr-friendly, so results can be checked at compile time
static_assert(ctre::match<"[a-z]+[0-9]+">("hello42"sv));
static_assert(!ctre::match<"[a-z]+">("hello42"sv));
static_assert(ctre::search<"[0-9]+">("hello42"sv));
static_assert(ctre::starts_with<"[a-z]+">("hello42"sv));

// the functions are functors, so they can be stored and passed around
constexpr auto is_identifier = ctre::match<"[a-zA-Z_][a-zA-Z0-9_]*">;

int main() {
	std::cout << std::boolalpha;
	std::cout << "match [a-z]+[0-9]+ on 'hello42':  " << bool(ctre::match<"[a-z]+[0-9]+">("hello42"sv)) << "\n";
	std::cout << "search [0-9]+ on 'hello42':       " << bool(ctre::search<"[0-9]+">("hello42"sv)) << "\n";
	std::cout << "starts_with [a-z]+ on 'hello42':  " << bool(ctre::starts_with<"[a-z]+">("hello42"sv)) << "\n";

	// subjects can be string_views, string-like ranges, iterator pairs,
	// or zero-terminated C strings (no strlen needed):
	std::cout << "is_identifier(\"snake_case_2\"):    " << bool(is_identifier("snake_case_2")) << "\n";
	std::cout << "is_identifier(\"2fast\"):           " << bool(is_identifier("2fast")) << "\n";

	// the result of a search is the matched part of the input
	if (auto m = ctre::search<"[0-9]+">("port is 8080, ok?"sv)) {
		std::cout << "searched number:                  " << m.to_view() << "\n";
	}

	// patterns can carry (?#comments), octal escapes and control characters
	std::cout << "with a comment:                   "
	          << bool(ctre::match<"ab(?#matches nothing, documents plenty)c">("abc"sv)) << "\n";
	std::cout << "\\o{101} is 'A':                   " << bool(ctre::match<"\\o{101}">("A"sv)) << "\n";
	std::cout << "\\cI is a tab:                     " << bool(ctre::match<"a\\cIb">("a\tb"sv)) << "\n";
}
