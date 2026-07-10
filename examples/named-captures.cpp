// Named capture groups and numeric conversion.
//
// Groups declared as (?<name>...) can be accessed with get<"name">()
// (C++20). Numbered access get<N>() always works too — capture 0 is the
// whole match, explicit groups count from 1.
//
// to_number<R>() converts a capture using std::from_chars.
//
// Build: make named-captures

#include <ctre.hpp>
#include <iostream>
#include <string_view>

using namespace std::literals;

int main() {
	constexpr auto url = "https://compile-time.re:443/index.html"sv;

	if (auto m = ctre::match<"(?<scheme>[a-z]+)://(?<host>[^:/]+)(?::(?<port>[0-9]+))?(?<path>/.*)?">(url)) {
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
}
