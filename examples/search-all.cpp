// Iterating over every match in an input.
//
// ctre::search_all returns a lazy range of regex_results — matching
// happens as you iterate. It also composes with a pipe syntax, works
// with explicit iterator/sentinel pairs, and a range of *subjects* can
// be piped through a scalar function like ctre::match to classify many
// strings at once.
//
// (ctre::range is a deprecated alias of ctre::search_all.)
//
// Build: make search-all

#include <ctre.hpp>
#include <iostream>
#include <string_view>
#include <vector>

using namespace std::literals;

int main() {
	constexpr auto input = "lengths: 12mm, 7cm, 3000um!"sv;

	// plain loop over every occurrence
	std::cout << "measurements:";
	for (auto m : ctre::search_all<"([0-9]+)(mm|cm|um)">(input)) {
		std::cout << " " << m.to_view();
	}
	std::cout << "\n";

	// same thing, pipe style; captures work as usual
	int sum = 0;
	for (auto m : input | ctre::search_all<"[0-9]+">) {
		sum += m.to_number<int>();
	}
	std::cout << "sum of all numbers: " << sum << "\n";

	// explicit iterator / sentinel form
	std::size_t count = 0;
	for (auto it = ctre::iterator<"[0-9]+">(input); it != ctre::sentinel; ++it) {
		++count;
	}
	std::cout << "number count: " << count << "\n";

	// piping a range of subjects through a scalar matcher yields one
	// result per subject:
	std::vector<std::string_view> lines = {"alpha"sv, "1234"sv, "beta"sv};
	std::cout << "words:";
	for (auto m : lines | ctre::match<"[a-z]+">) {
		if (m) std::cout << " " << m.to_view();
	}
	std::cout << "\n";
}
