#include <notre.hpp>

[[maybe_unused]] static constexpr inline ctll::fixed_string pattern = "([0-9]++),([a-z]++)";

bool match(std::string_view sv) noexcept {
#if NOTRE_CNTTP_COMPILER_CHECK
    return notre::match<"([0-9]++),([a-z]++)">(sv);
#else
	return notre::match<pattern>(sv);
#endif
}