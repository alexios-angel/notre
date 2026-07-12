#include <cstdlib>
#include <cassert>
#include <string_view>
#include <optional>

#include <notre.hpp>

using namespace std::string_view_literals;
using namespace notre::literals;

struct date { std::string_view year; std::string_view month; std::string_view day; };

static constexpr ctll::fixed_string pattern = "^([0-9]{4})/([0-9]{1,2}+)/([0-9]{1,2}+)$";

constexpr std::optional<date> extract_date(std::string_view s) noexcept {
    if (auto [whole, year, month, day] = notre::match<pattern>(s); whole
    ) {
        return date{year.to_view(), month.to_view(), day.to_view()};
    } else {
        return std::nullopt;
    }
}

int main() {
 
    assert(extract_date("2018/08/27"sv).has_value());
    assert(extract_date("2018/08/27"sv)->year == "2018"sv);
    assert(extract_date("2018/08/27"sv)->month == "08"sv);
    assert(extract_date("2018/08/27"sv)->day == "27"sv);

    return EXIT_SUCCESS;
}
