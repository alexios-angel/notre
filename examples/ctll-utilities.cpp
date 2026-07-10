// The ctll toolkit underneath the regex engine is usable on its own:
//
//   ctll::fixed_string  a general purpose compile-time string
//   ctll::function      a constexpr std::function (C++20)
//   ctll::map           a constexpr fixed-size lookup map
//
// Build: make ctll-utilities

#include <ctll/fixed_string.hpp>
#include <ctll/function.hpp>
#include <ctll/map.hpp>
#include <iostream>
#include <string_view>

using namespace std::string_view_literals;

// --- fixed_string: comparisons, search, slicing and concatenation

static constexpr auto greeting = ctll::fixed_string{"hello"} + ", " + ctll::fixed_string{"world"};
static_assert(greeting == "hello, world");
static_assert(greeting.starts_with(ctll::fixed_string{"hello"}));
static_assert(greeting.find(U',') == 5);
static_assert(greeting.substr<7>() == "world");

// --- function: type-erased callables inside constant expressions

constexpr int apply_twice(const ctll::function<int(int)> & f, int x) {
	return f(f(x));
}
static_assert(apply_twice([](int v) { return v + 20; }, 2) == 42);

// --- map: literal keys compare by content, values can be functions

constexpr auto color_of = [](std::string_view name) {
	constexpr ctll::map colors{
		std::pair{"red", 0xff0000},
		std::pair{"green", 0x00ff00},
		std::pair{"blue", 0x0000ff},
	};
	static_assert(colors.has_unique_keys());
	const auto it = colors.find(name);
	return it != colors.end() ? it->second : -1;
};
static_assert(color_of("green") == 0x00ff00);
static_assert(color_of("mauve") == -1);

int main() {
	std::cout << "greeting: " << greeting << "\n"; // utf-8 encoding ostream support

	// name -> function dispatch, the callout-registry shape
	ctll::map ops{
		std::pair{"double", ctll::function<int(int)>([](int x) { return 2 * x; })},
		std::pair{"square", ctll::function<int(int)>([](int x) { return x * x; })},
	};
	for (auto name : {"double"sv, "square"sv}) {
		std::cout << name << "(21) = " << ops.at(name)(21) << "\n";
	}

	std::cout << "color_of(\"blue\") = #" << std::hex << color_of("blue") << "\n";
}
