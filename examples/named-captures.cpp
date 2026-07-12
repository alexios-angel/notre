// Named capture groups and numeric conversion.
//
// Groups declared as (?<name>...) — or with the equivalent (?P<name>...)
// and (?'name'...) syntaxes — can be accessed with get<"name">() (C++20).
// Numbered access get<N>() always works too: capture 0 is the whole
// match, explicit groups count from 1.
//
// to_number<R>() converts a capture using std::from_chars.
//
// Build: make named-captures

#include <notre.hpp>
#include <iostream>
#include <string_view>

using namespace std::literals;

int main() {
	constexpr auto url = "https://compile-time.re:443/index.html"sv;

	if (auto m = notre::match<"(?<scheme>[a-z]+)://(?<host>[^:/]+)(?::(?<port>[0-9]+))?(?<path>/.*)?">(url)) {
		std::cout << "scheme: " << m.get<"scheme">() << "\n";
		std::cout << "host:   " << m.get<"host">() << "\n";

		// an optional group knows whether it participated in the match:
		if (auto port = m.get<"port">()) {
			std::cout << "port:   " << port.to_number<int>() << "\n";
		}

		std::cout << "path:   " << m.get<"path">() << "\n";

		// the same groups are also reachable by number:
		std::cout << "get<1>: " << m.get<1>() << "\n";
	}

	// all three declaration syntaxes are equivalent, and \g{name}
	// backreferences work with any of them
	std::cout << std::boolalpha;
	std::cout << "(?P<w>ab) matches:      " << bool(notre::match<"(?P<w>ab)">("ab"sv)) << "\n";
	std::cout << "(?'w'ab)\\g{w} on abab:  " << bool(notre::match<"(?'w'ab)\\g{w}">("abab"sv)) << "\n";
}
