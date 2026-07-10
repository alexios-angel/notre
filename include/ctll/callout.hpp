#ifndef CTLL__CALLOUT__HPP
#define CTLL__CALLOUT__HPP

#ifndef CTLL_IN_A_MODULE
#include <cstddef>
#include <string_view>
#endif

#include "utilities.hpp"

// An inline callout entry: a compile-time key (a name or a number) paired
// with a captureless callable, both non-type template parameters. A pack
// of these goes into ctre::with_callouts to attach implementations to a
// pattern's (?C...) callouts without writing a handler type:
//
//   ctre::match<"a(?C'go')b", ctre::with_callouts<
//       ctll::callout<"go", [](const auto & c) { ... }>>>(subject);
//   ctre::match<"a(?C1)b", ctre::with_callouts<
//       ctll::callout<1, [](const auto & c) { ... }>>>(subject);
//
// A numbered key is normalized to its decimal spelling, so ctll::callout<7,
// f> serves (?C7) exactly like a "7" key would. Requires C++20 class-type
// template parameters (the same compiler support as ctre::match<"...">);
// on older compilers this header is empty and only the handler-type form
// of with_callouts is available.

#if CTLL_CNTTP_COMPILER_CHECK
#define CTLL_CALLOUT_SUPPORTED 1

namespace ctll {

// a structural key holding either a name or a number's decimal spelling
struct callout_key {
	char content[32]{};
	size_t length{0};

	template <size_t N> constexpr callout_key(const char (&name)[N]) noexcept {
		static_assert(N - 1 <= 32, "callout names are limited to 32 characters");
		for (size_t i = 0; i < N - 1; ++i) {
			content[i] = name[i];
		}
		length = N - 1;
	}

	constexpr callout_key(unsigned long long number) noexcept {
		size_t digits = 1;
		for (auto v = number; v >= 10; v /= 10) {
			++digits;
		}
		length = digits;
		for (size_t i = digits; i != 0; --i) {
			content[i - 1] = static_cast<char>('0' + number % 10);
			number /= 10;
		}
	}

	constexpr std::string_view view() const noexcept {
		return std::string_view{content, length};
	}
};

CTLL_EXPORT template <callout_key Key, auto Function> struct callout {
	static constexpr callout_key key = Key;

	template <typename Data> static constexpr decltype(auto) invoke(const Data & data) {
		return Function(data);
	}
};

}

#endif

#endif
