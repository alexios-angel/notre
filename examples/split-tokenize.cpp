// Splitting input by a regex, and tokenizing it.
//
// split    - yields the parts of the input *between* matches of the
//            separator pattern; the separator's own capture groups stay
//            accessible on each yielded result.
// tokenize - yields consecutive matches anchored to the end of the
//            previous one, stopping at the first gap.
//
// Build: make split-tokenize

#include <notre.hpp>
#include <iostream>
#include <string_view>

using namespace std::literals;

int main() {
	// split a comma/semicolon separated list
	std::cout << "parts:";
	for (auto part : notre::split<"[,;]">("a,b;c,d"sv)) {
		std::cout << " '" << part.to_view() << "'";
	}
	std::cout << "\n";

	// the separator's capture groups remain readable on each part:
	std::cout << "parts with separators:";
	for (auto part : notre::split<"([,;])">("a,b;c"sv)) {
		std::cout << " '" << part.to_view() << "'";
		if (auto sep = part.get<1>()) {
			std::cout << " (sep '" << sep.to_view() << "')";
		}
	}
	std::cout << "\n";

	// tokenize: matches must follow each other back-to-back;
	// iteration stops at "99;" because it doesn't match [a-z]+;
	std::cout << "tokens:";
	for (auto token : notre::tokenize<"[a-z]+;">("ab;cd;99;ef;"sv)) {
		std::cout << " '" << token.to_view() << "'";
	}
	std::cout << "\n";
}
