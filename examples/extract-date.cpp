// Extracting capture groups with structured bindings — at compile time.
//
// regex_results supports structured bindings; the first binding is the
// whole match (capture 0), followed by one binding per capture group.
// Because matching is constexpr, the extraction can be static_asserted.
//
// Build: make extract-date

#include <notre.hpp>
#include <iostream>
#include <optional>
#include <string_view>

using namespace std::literals;

struct date {
	std::string_view year;
	std::string_view month;
	std::string_view day;
};

constexpr std::optional<date> extract_date(std::string_view s) noexcept {
	if (auto [whole, year, month, day] = notre::match<"(\\d{4})/(\\d{1,2})/(\\d{1,2})">(s); whole) {
		return date{year, month, day};
	} else {
		return std::nullopt;
	}
}

// the whole extraction runs during compilation:
static_assert(extract_date("2018/08/27"sv).has_value());
static_assert(extract_date("2018/08/27"sv)->year == "2018"sv);
static_assert(extract_date("2018/08/27"sv)->month == "08"sv);
static_assert(extract_date("2018/08/27"sv)->day == "27"sv);
static_assert(!extract_date("not a date"sv).has_value());

int main() {
	if (auto d = extract_date("2026/07/10"sv)) {
		std::cout << "year:  " << d->year << "\n";
		std::cout << "month: " << d->month << "\n";
		std::cout << "day:   " << d->day << "\n";
	}
}
