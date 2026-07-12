// Callouts: (?Cn) and (?C'name') are zero-width observation points that
// can watch a match in progress and veto positions.
//
// Implementations attach with notre::with_callouts, either as inline
// ctll::callout entries (matched to their callout at compile time) or as
// one handler type with a name->function map (dispatch at match time).
// Returning fail / false vetoes the position and backtracking continues,
// like PCRE's nonzero callout return.
//
// Build: make callouts

#include <notre.hpp>
#include <ctll/callout.hpp>
#include <ctll/function.hpp>
#include <ctll/map.hpp>
#include <iostream>
#include <string_view>

using namespace std::literals;

// --- inline entries: named veto and numbered observer

// the veto makes the greedy + backtrack until at most 2 digits remain
constexpr auto limited = notre::search<"([0-9]+)(?C'limit')", notre::with_callouts<
	ctll::callout<"limit", [](const auto & c) { return c.position - c.match_start <= 2; }>
>>;

static_assert(limited("xx123yy"sv).get<1>() == "12"sv);

// entries may return notre::callout_result, bool, or void (pure observer)
static_assert(notre::match<"a(?C'ok')b", notre::with_callouts<
	ctll::callout<"ok", [](const auto &) { /* just watching */ }>
>>("ab"sv));

// generic lambdas work with any subject character type
static_assert(notre::match<L"a(?C'w')b", notre::with_callouts<
	ctll::callout<"w", [](const auto & c) { return c.subject == L"ab"sv; }>
>>(L"ab"sv));

// --- handler type with a map: dispatch by name at match time

struct tracing_handler {
	static constexpr auto callouts() {
		return ctll::map{
			std::pair{"trace", ctll::function<notre::callout_result(const notre::callout_data<char> &)>(
				[](const notre::callout_data<char> & c) {
					std::cout << "  callout '" << c.name << "' at offset " << c.position
					          << " (attempt started at " << c.match_start << ")\n";
					return notre::callout_result::proceed;
				})},
		};
	}
};

int observer_hits = 0; // entries are captureless, but can reach globals at runtime

int main() {
	std::cout << "veto-limited digits in 'xx12345yy': '"
	          << limited("xx12345yy"sv).get<1>().to_view() << "'\n";

	// numbered observer counting at runtime
	auto counted = notre::search<"[a-z]+(?C1)", notre::with_callouts<
		ctll::callout<1, [](const auto &) { ++observer_hits; }>
	>>("some words here"sv);
	std::cout << "matched '" << counted.to_view() << "', observer saw "
	          << observer_hits << " completed attempt(s)\n";

	// map-based tracing
	std::cout << "tracing ab(?C'trace') over 'xxabyy':\n";
	notre::search<"ab(?C'trace')", notre::with_callouts<tracing_handler>>("xxabyy"sv);

	// a pattern with callouts still works with NO handler at all: no-ops
	std::cout << "without any handler: "
	          << std::boolalpha << bool(notre::match<"a(?C'limit')b">("ab"sv)) << "\n";
}
