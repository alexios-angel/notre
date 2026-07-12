#include <notre.hpp>

void empty_symbol() { }

using namespace notre::test_literals;

#if !NOTRE_CNTTP_COMPILER_CHECK
#define NOTRE_TEST(pattern) (pattern ## _notre_test)
#else

template <typename...> struct identify_me;

template <ctll::fixed_string input> constexpr bool test() {
	constexpr auto _input = input;

	using tmp = typename ctll::parser<notre::pcre, _input, notre::pcre_actions>::template output<notre::pcre_context<>>;
	//static_assert(tmp(), "Regular Expression contains syntax error.");
	//using re = decltype(front(typename tmp::output_type::stack_type()));
	//identify_me<typename tmp::output_type::stack_type> i;
	////return notre::regular_expression(re());
	return tmp();
}

#define NOTRE_TEST(pattern) test<pattern>()
#endif


// basics
static_assert(!NOTRE_TEST("(?)"));
static_assert(NOTRE_TEST("(?i)"));
static_assert(NOTRE_TEST("(?ic)"));
static_assert(NOTRE_TEST("(?icsm)"));

// after
static_assert(NOTRE_TEST("(?i)x"));
static_assert(NOTRE_TEST("(?ic)x"));
static_assert(NOTRE_TEST("(?icsm)x"));

// before
static_assert(NOTRE_TEST("y(?i)"));
static_assert(NOTRE_TEST("y(?ic)"));
static_assert(NOTRE_TEST("y(?icsm)"));

static_assert(NOTRE_TEST("(?i)Tom|Sawyer|Huckleberry|Finn"));


