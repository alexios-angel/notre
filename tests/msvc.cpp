#include <notre.hpp>
#include <string_view>

void empty_symbol() { }

using namespace std::string_view_literals;

static inline constexpr auto pattern1 = ctll::fixed_string{"^[\\x30-\\x39]+?$"};
static inline constexpr auto pattern2 = ctll::fixed_string{""};


static_assert(notre::re<pattern1>().match("123456789"sv));
static_assert(notre::re<pattern2>().match(""sv));

template <auto & ptn> constexpr bool re() {
#if NOTRE_CNTTP_COMPILER_CHECK
	constexpr auto _ptn = ptn;
#else
	constexpr auto & _ptn = ptn;
#endif
	return ctll::parser<notre::pcre, _ptn, notre::pcre_actions>::template correct_with<notre::pcre_context<>>;
}

static_assert(re<pattern2>());

static inline constexpr ctll::fixed_string pat = "hello";

template <auto & ptn> constexpr bool re2() {
#if NOTRE_CNTTP_COMPILER_CHECK
	constexpr auto _ptn = ptn;
#else
	constexpr auto & _ptn = ptn;
#endif
	return ctll::parser<notre::pcre, _ptn, notre::pcre_actions>::template correct_with<notre::pcre_context<>>;
}

static_assert(re<pat>());

static_assert(notre::re<pat>().match("hello"sv));

static_assert(notre::match<pat>("hello"sv));
