// Callouts: (?Cn) and (?C'name') are zero-width observation points that
// can watch a match in progress and veto positions.
//
// Implementations attach with ctre::with_callouts, either as inline
// ctll::callout entries (matched to their callout at compile time) or as
// one handler type with a name->function map (dispatch at match time).
// Returning fail / false vetoes the position and backtracking continues,
// like PCRE's nonzero callout return.
//
// Build: make callouts

#include <ctre.hpp>
#include <ctll/callout.hpp>
#include <ctll/function.hpp>
#include <ctll/map.hpp>
#include <iostream>
#include <string_view>

using namespace std::literals;

// --- inline entries: named veto and numbered observer

// the veto makes the greedy + backtrack until at most 2 digits remain
constexpr auto limited = ctre::search<"([0-9]+)(?C'limit')", ctre::with_callouts<
	ctll::callout<"limit", [](const auto & c) { return c.position - c.match_start <= 2; }>
>>;

static_assert(limited("xx123yy"sv).get<1>() == "12"sv);

// entries may return ctre::callout_result, bool, or void (pure observer)
static_assert(ctre::match<"a(?C'ok')b", ctre::with_callouts<
	ctll::callout<"ok", [](const auto &) { /* just watching */ }>
>>("ab"sv));

// generic lambdas work with any subject character type
static_assert(ctre::match<L"a(?C'w')b", ctre::with_callouts<
	ctll::callout<"w", [](const auto & c) { return c.subject == L"ab"sv; }>
>>(L"ab"sv));

// --- handler type with a map: dispatch by name at match time

struct tracing_handler {
	static constexpr auto callouts() {
		return ctll::map{
			std::pair{"trace", ctll::function<ctre::callout_result(const ctre::callout_data<char> &)>(
				[](const ctre::callout_data<char> & c) {
					std::cout << "  callout '" << c.name << "' at offset " << c.position
					          << " (attempt started at " << c.match_start << ")\n";
					return ctre::callout_result::proceed;
				})},
		};
	}
};

int observer_hits = 0; // entries are captureless, but can reach globals at runtime

int main() {
	std::cout << "veto-limited digits in 'xx12345yy': '"
	          << limited("xx12345yy"sv).get<1>().to_view() << "'\n";

	// numbered observer counting at runtime
	auto counted = ctre::search<"[a-z]+(?C1)", ctre::with_callouts<
		ctll::callout<1, [](const auto &) { ++observer_hits; }>
	>>("some words here"sv);
	std::cout << "matched '" << counted.to_view() << "', observer saw "
	          << observer_hits << " completed attempt(s)\n";

	// map-based tracing
	std::cout << "tracing ab(?C'trace') over 'xxabyy':\n";
	ctre::search<"ab(?C'trace')", ctre::with_callouts<tracing_handler>>("xxabyy"sv);

	// a pattern with callouts still works with NO handler at all: no-ops
	std::cout << "without any handler: "
	          << std::boolalpha << bool(ctre::match<"a(?C'limit')b">("ab"sv)) << "\n";
}
