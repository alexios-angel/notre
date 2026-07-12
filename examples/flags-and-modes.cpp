// Case-insensitive and multiline matching.
//
// Three equivalent ways to change matching behaviour:
//   1. a modifier passed as an extra template argument
//      (notre::case_insensitive / notre::case_sensitive, aliases ci / cs)
//   2. an inline mode switch in the pattern: (?i) (?c) (?s) (?m)
//   3. the multiline_* function variants, which make ^ and $ match at
//      line boundaries
//
// Build: make flags-and-modes

#include <notre.hpp>
#include <iostream>
#include <string_view>

using namespace std::literals;

// 1. modifier as template argument
static_assert(notre::match<"hello", notre::case_insensitive>("HeLLo"sv));
static_assert(!notre::match<"hello", notre::case_sensitive>("HeLLo"sv));

// 2. inline mode switch inside the pattern ((?c) switches back)
static_assert(notre::match<"(?i)hello">("HELLO"sv));
static_assert(notre::match<"(?i)AB(?c)AB", notre::case_sensitive>("abAB"sv));

// 3. multiline variants: ^ and $ match around \n
static_assert(notre::multiline_search<"^bar$">("foo\nbar"sv));
static_assert(!notre::search<"^bar$">("foo\nbar"sv));

int main() {
	constexpr auto log = "INFO ok\nERROR disk full\nWARN retry"sv;

	// find the error line, however it is capitalized
	for (auto line : notre::multiline_search_all<"^(?i)error\\s+(.+)$">(log)) {
		std::cout << "error message: " << line.get<1>() << "\n";
	}

	std::cout << std::boolalpha;
	std::cout << "case-insensitive 'HeLLo': " << bool(notre::match<"hello", notre::ci>("HeLLo"sv)) << "\n";
}
