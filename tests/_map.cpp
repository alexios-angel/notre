#include <ctll/map.hpp>
#include <type_traits>

void empty_symbol() { }

using namespace std::string_view_literals;

// deduction from a flat list of pairs; string literal keys become
// string_views so lookup compares content, not pointers
constexpr auto colors = ctll::map{
	std::pair{"red", 1},
	std::pair{"green", 2},
	std::pair{"blue", 3},
};
static_assert(std::is_same_v<decltype(colors), const ctll::map<std::string_view, int, 3>>);

static_assert(colors.size() == 3);
static_assert(!colors.empty());
static_assert(colors.has_unique_keys());

// lookup is heterogeneous: literals, string_views, anything == comparable
static_assert(colors.at("red") == 1);
static_assert(colors.at("green"sv) == 2);
static_assert(colors["blue"] == 3);
static_assert(colors.contains("red"));
static_assert(!colors.contains("mauve"));
static_assert(colors.count("red") == 1);
static_assert(colors.count("mauve") == 0);
static_assert(colors.find("mauve") == colors.end());
static_assert(colors.find("green")->second == 2);

// iteration
static_assert([] {
	int sum = 0;
	for (const auto & entry : colors) {
		sum += entry.second;
	}
	return sum;
}() == 6);

// values are assignable through find/at/operator[]; keys are not touched
static_assert([] {
	auto m = colors;
	m.at("red") = 10;
	m["green"] = 20;
	m.find("blue")->second = 30;
	return m.at("red") + m.at("green") + m.at("blue");
}() == 60);

// equality compares entries
static_assert([] {
	auto m = colors;
	bool same = (m == colors);
	m.at("red") = 0;
	return same && (m != colors);
}());

// duplicate keys are constructible (find returns the first); the
// validation helper reports them
static_assert(!ctll::map{std::pair{"a", 1}, std::pair{"a", 2}}.has_unique_keys());
static_assert(ctll::map{std::pair{"a", 1}, std::pair{"a", 2}}.at("a") == 1);

// deduction from a prebuilt array of pairs (the reference's usage style)
static constexpr std::array<std::pair<std::string_view, int>, 2> table{{{"on"sv, 1}, {"off"sv, 0}}};
static_assert(ctll::map{table}.at("on") == 1);

// wide keys normalize to the matching string_view
static_assert(ctll::map{std::pair{L"wide", 1}}.contains(L"wide"sv));
static_assert(std::is_same_v<decltype(ctll::map{std::pair{U"u32", 1}})::key_type, std::u32string_view>);

// an empty map works
static_assert(ctll::map<std::string_view, int, 0>{}.empty());
static_assert(!ctll::map<std::string_view, int, 0>{}.contains("x"));

// non-string keys stay as given
static_assert(ctll::map{std::pair{1, 10}, std::pair{2, 20}}.at(2) == 20);

#if defined(__cpp_constexpr_dynamic_alloc) && __cpp_constexpr_dynamic_alloc >= 201907L

// the intended library use: names mapped to ctll::function wrappers,
// looked up and invoked inside a constant expression (C++20)
#include <ctll/function.hpp>

static_assert([] {
	ctll::map callouts{
		std::pair{"double", ctll::function<int(int)>([](int x) { return 2 * x; })},
		std::pair{"negate", ctll::function<int(int)>([](int x) { return -x; })},
	};
	if (!callouts.has_unique_keys()) return 0;
	int r = callouts.at("double")(21) + callouts.at("negate")(-1);
	// implementations can be rebound without changing the keys
	callouts.at("double") = [](int x) { return x; };
	return r + callouts.at("double")(-1) + (callouts.contains("missing") ? 100 : 0);
}() == 42);

#endif
