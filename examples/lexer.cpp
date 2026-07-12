// A tiny lexer built from one alternation pattern.
//
// Each alternative gets its own capture group; whichever group
// participated in the match tells you the token type. Combined with
// notre::tokenize this turns into a whole tokenizer loop.
//
// Build: make lexer

#include <notre.hpp>
#include <iostream>
#include <optional>
#include <string_view>

using namespace std::literals;

enum class type {
	unknown, identifier, number
};

struct lex_item {
	type t;
	std::string_view c;
};

constexpr std::optional<lex_item> lexer(std::string_view v) noexcept {
	if (auto [m, id, num] = notre::match<"([a-z]+)|([0-9]+)">(v); m) {
		if (id) {
			return lex_item{type::identifier, id};
		} else if (num) {
			return lex_item{type::number, num};
		}
	}
	return std::nullopt;
}

static_assert(lexer("abc"sv)->t == type::identifier);
static_assert(lexer("123"sv)->t == type::number);
static_assert(!lexer("!?"sv).has_value());

constexpr std::string_view name_of(type t) noexcept {
	switch (t) {
		case type::identifier: return "identifier";
		case type::number:     return "number";
		default:               return "unknown";
	}
}

int main() {
	for (auto input : {"hello"sv, "42"sv, "#!"sv}) {
		if (auto item = lexer(input)) {
			std::cout << input << " -> " << name_of(item->t) << " (" << item->c << ")\n";
		} else {
			std::cout << input << " -> no token\n";
		}
	}
}
