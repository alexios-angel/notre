#include <notre.hpp>

void empty_symbol() { }

// unbound callouts (no with_callouts modifier) are zero-width no-ops;
// this part works in C++17 too
#if NOTRE_CNTTP_COMPILER_CHECK
using namespace std::string_view_literals;

static_assert(notre::match<"a(?C)b">("ab"sv));
static_assert(notre::match<"a(?C255)b">("ab"sv));
static_assert(notre::match<"a(?C'named')b">("ab"sv));
static_assert(notre::match<"a(?C\"named\")b">("ab"sv));
static_assert(notre::match<"(?C1)(?C'x')">(""sv));
static_assert(notre::match<"a(?C'x')*b">("ab"sv));

// handlers need ctll::function/ctll::map, so C++20
#if defined(__cpp_constexpr_dynamic_alloc) && __cpp_constexpr_dynamic_alloc >= 201907L

#include <ctll/function.hpp>
#include <ctll/map.hpp>

using cb_fn = ctll::function<notre::callout_result(const notre::callout_data<char> &)>;

struct always_proceed {
	static constexpr auto callouts() {
		return ctll::map{std::pair{"go", cb_fn([](const auto &) { return notre::callout_result::proceed; })}};
	}
};
struct always_fail {
	static constexpr auto callouts() {
		return ctll::map{std::pair{"stop", cb_fn([](const auto &) { return notre::callout_result::fail; })}};
	}
};

// proceed lets the match through, fail vetoes it
static_assert(notre::match<"a(?C'go')b", notre::with_callouts<always_proceed>>("ab"sv));
static_assert(!notre::match<"a(?C'stop')b", notre::with_callouts<always_fail>>("ab"sv));

// a name missing from the map proceeds, like PCRE with no callout set
static_assert(notre::match<"a(?C'unknown')b", notre::with_callouts<always_fail>>("ab"sv));

// a veto at every search position means no match anywhere
static_assert(!notre::search<"a(?C'stop')", notre::with_callouts<always_fail>>("aaa"sv));

// the callout sees name, number, subject and offsets
struct check_data {
	static constexpr auto callouts() {
		return ctll::map{
			std::pair{"d", cb_fn([](const notre::callout_data<char> & c) {
				const bool ok = c.name == "d"sv && c.number == 0
					&& c.subject == "xxabyy"sv
					&& c.match_start == 2   // search found "ab" at offset 2
					&& c.position == 4;     // the callout sits after "ab"
				return ok ? notre::callout_result::proceed : notre::callout_result::fail;
			})},
		};
	}
};
static_assert(notre::search<"ab(?C'd')", notre::with_callouts<check_data>>("xxabyy"sv));

// numbered callouts dispatch with their decimal spelling as the key
struct numbered {
	static constexpr auto callouts() {
		return ctll::map{
			std::pair{"7", cb_fn([](const auto & c) { return c.number == 7 ? notre::callout_result::proceed : notre::callout_result::fail; })},
			std::pair{"0", cb_fn([](const auto & c) { return c.number == 0 ? notre::callout_result::proceed : notre::callout_result::fail; })},
		};
	}
};
static_assert(notre::match<"a(?C7)b", notre::with_callouts<numbered>>("ab"sv));
static_assert(notre::match<"a(?C)b", notre::with_callouts<numbered>>("ab"sv));

// a veto makes greedy repeats backtrack, like PCRE's nonzero callout
// return (regression: the possessive first-set optimization must stay
// disabled around bound callouts)
struct limit2 {
	static constexpr auto callouts() {
		return ctll::map{std::pair{"lim", cb_fn([](const auto & c) {
			return c.position - c.match_start <= 2 ? notre::callout_result::proceed : notre::callout_result::fail;
		})}};
	}
};
static_assert(notre::match<"(a+)(?C'lim')a*", notre::with_callouts<limit2>>("aaaa"sv).get<1>() == "aa"sv);
static_assert(notre::search<"([0-9]+)(?C'lim')", notre::with_callouts<limit2>>("xx123yy"sv).get<1>() == "12"sv);

// a veto in one alternation branch reroutes to the other
struct no_left {
	static constexpr auto callouts() {
		return ctll::map{std::pair{"L", cb_fn([](const auto &) { return notre::callout_result::fail; })}};
	}
};
static_assert(notre::match<"(?:a(?C'L')|(ab))", notre::with_callouts<no_left>>("ab"sv).get<1>() == "ab"sv);

// combines with other modifiers
static_assert(notre::match<"AB(?C'go')", notre::with_callouts<always_proceed>, notre::case_insensitive>("ab"sv));

#endif // __cpp_constexpr_dynamic_alloc

// inline entries: ctll::callout NTTPs matched to callouts at compile time
#include <ctll/callout.hpp>
#ifdef CTLL_CALLOUT_SUPPORTED

// named and numbered entry packs
static_assert(notre::match<"a(?C'go')b(?C'go2')", notre::with_callouts<
	ctll::callout<"go", [](const auto &) { return notre::callout_result::proceed; }>,
	ctll::callout<"go2", [](const auto &) { return notre::callout_result::proceed; }>
>>("ab"sv));
static_assert(notre::match<"a(?C1)b(?C2)", notre::with_callouts<
	ctll::callout<1, [](const auto &) { return notre::callout_result::proceed; }>,
	ctll::callout<2, [](const auto &) { return notre::callout_result::proceed; }>
>>("ab"sv));

// entries may return callout_result, bool, or void (pure observer)
static_assert(notre::match<"a(?C'y')b", notre::with_callouts<
	ctll::callout<"y", [](const auto &) { return true; }>>>("ab"sv));
static_assert(!notre::match<"a(?C'n')b", notre::with_callouts<
	ctll::callout<"n", [](const auto &) { return false; }>>>("ab"sv));
static_assert(notre::match<"a(?C'o')b", notre::with_callouts<
	ctll::callout<"o", [](const auto &) { }>>>("ab"sv));

// veto makes the greedy repeat backtrack, as with the handler form
static_assert(notre::search<"([0-9]+)(?C'lim')", notre::with_callouts<
	ctll::callout<"lim", [](const auto & c) { return c.position - c.match_start <= 2; }>
>>("xx123yy"sv).get<1>() == "12"sv);

// the entry sees the callout's data
static_assert(notre::match<"a(?C42)b", notre::with_callouts<
	ctll::callout<42, [](const auto & c) { return c.number == 42 && c.name == "42"sv; }>
>>("ab"sv));

// an unmatched callout stays a no-op
static_assert(notre::match<"a(?C'other')b", notre::with_callouts<
	ctll::callout<"go", [](const auto &) { return false; }>>>("ab"sv));

// generic lambdas work with wide subjects
static_assert(notre::match<L"a(?C'w')b", notre::with_callouts<
	ctll::callout<"w", [](const auto & c) { return c.subject == L"ab"sv; }>
>>(L"ab"sv));

// modifier order does not matter
static_assert(notre::match<"AB(?C'x')", notre::case_insensitive, notre::with_callouts<
	ctll::callout<"x", [](const auto &) { return true; }>>>("ab"sv));

#endif // CTLL_CALLOUT_SUPPORTED
#endif // NOTRE_CNTTP_COMPILER_CHECK
