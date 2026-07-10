// Unicode properties and UTF-8 matching.
//
// Include <ctre-unicode.hpp> (or <ctre.hpp> + <unicode-db.hpp>) to
// enable Unicode support. std::u8string_view subjects are decoded as
// UTF-8 during matching, and \p{...} matches Unicode properties
// (general categories, scripts, blocks, age, ...).
//
// Build: make unicode

#include <ctre-unicode.hpp>
#include <iostream>
#include <string_view>

// needed only to print utf-8 bytes to the terminal
std::string_view cast_from_unicode(std::u8string_view input) noexcept {
	return std::string_view(reinterpret_cast<const char *>(input.data()), input.size());
}

int main() {
	using namespace std::literals;
	std::u8string_view original = u8"Tu es un génie"sv;

	// every word, including the accented one:
	for (auto match : ctre::search_all<"\\p{Letter}+">(original)) {
		std::cout << cast_from_unicode(match) << "\n";
	}

	// match by script:
	std::u8string_view mixed = u8"abc απλό δίκαιο xyz"sv;
	std::cout << "greek words:";
	for (auto match : ctre::search_all<"\\p{Script=Greek}+">(mixed)) {
		std::cout << " " << cast_from_unicode(match);
	}
	std::cout << "\n";

	// \X matches one grapheme cluster (a code point plus its combining
	// marks), where . matches a single code point: "e" + U+0301 is two
	// code points but one cluster
	std::u8string_view decomposed = u8"éàu"sv;
	size_t clusters = 0;
	for ([[maybe_unused]] auto match : ctre::search_all<"\\X">(decomposed)) {
		++clusters;
	}
	std::cout << "code units: " << decomposed.size() << ", clusters: " << clusters << "\n";
}
