// Using CTRE with C++17 (no class-type NTTP support).
//
// The pattern is provided as a constexpr ctll::fixed_string variable
// instead of a string literal template argument. Named captures are
// accessed through a fixed_string variable with linkage.
//
// Where the compiler supports the N3599 GNU extension (clang; gcc with
// -DCTRE_ENABLE_LITERALS and without -pedantic), the "..."_ctre user
// defined literal also works.
//
// Build: make cxx17-syntax   (compiled with -std=c++17)

#include <ctre.hpp>
#include <iostream>
#include <string_view>

using namespace std::literals;

// pattern as a constexpr variable
static constexpr auto pattern = ctll::fixed_string{"(?<year>[0-9]{4})/([0-9]{1,2})"};

// capture name as a variable with linkage
static constexpr ctll::fixed_string year_name = "year";

constexpr std::string_view get_year(std::string_view sv) noexcept {
	if (auto m = ctre::match<pattern>(sv)) {
		return m.template get<year_name>();
	}
	return {};
}

static_assert(get_year("2026/07"sv) == "2026"sv);

int main() {
	std::cout << "year of 2026/07: " << get_year("2026/07"sv) << "\n";

#ifdef CTRE_ENABLE_LITERALS
	// UDL syntax, when the compiler supports it
	using namespace ctre::literals;
	std::cout << std::boolalpha;
	std::cout << "\"h.*\"_ctre matches 'hello': " << bool("h.*"_ctre.match("hello"sv)) << "\n";
#endif
}
