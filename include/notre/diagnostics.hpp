#ifndef NOTRE__DIAGNOSTICS__HPP
#define NOTRE__DIAGNOSTICS__HPP

#include "pcre.hpp"
#include "pcre_actions.hpp"
#include "utility.hpp"
#ifndef NOTRE_IN_A_MODULE
#include <cstddef>
#include <string_view>
#endif

// Included as a leaf at the end of notre.hpp: it relies on ctll (parser,
// list, fixed_string) already being available from the core headers,
// exactly as the rest of this library's headers do, so it adds no new
// edges to the amalgamated single-header include graph.

// Queryable diagnostics for a regular expression that fails to compile.
//
// notre::match<"...">() and friends turn a malformed pattern into a hard
// static_assert. These queries never hard-error: they run the same CTLL
// parse without the assert and report WHERE the pattern broke.
//
//   static_assert(notre::valid<"[a-z]+">);            // a bool, never an error
//   constexpr auto e = notre::error_info<"a(b|c">();  // .position/.line/.column
//   constexpr auto m = notre::error_message<"a(b|c">();
//   //   notre: regex syntax error at position 5 (line 1, column 6)
//   //     a(b|c
//   //          ^
//
// error_message() renders into static storage, so it can be
// static_assert-ed, printed at runtime, or shown in your own tooling.
// Define NOTRE_VERBOSE_ERRORS to also surface the position, line and
// column directly in the compiler's backtrace for a failed match<>().

namespace notre {

#if NOTRE_CNTTP_COMPILER_CHECK
#define NOTRE_PATTERN_INPUT ctll::fixed_string
#define NOTRE_PATTERN_COPY auto
#else
// C++17: pass a constexpr ctll::fixed_string variable with linkage
#define NOTRE_PATTERN_INPUT const auto &
#define NOTRE_PATTERN_COPY const auto &
#endif

// a regex either compiles or has a syntax error at a position
NOTRE_EXPORT enum class error_kind : unsigned char { none, syntax };

NOTRE_EXPORT constexpr std::string_view to_string(error_kind k) noexcept {
	return k == error_kind::none ? std::string_view{"none"} : std::string_view{"syntax error"};
}

// a position in the pattern resolved to a 1-based line and column
// (patterns are usually one line; (?x) extended mode can span several)
NOTRE_EXPORT struct source_position {
	size_t offset = 0;
	size_t line = 1;
	size_t column = 1;
};

NOTRE_EXPORT constexpr source_position locate(std::string_view text, size_t offset) noexcept {
	source_position p{};
	if (offset > text.size()) { offset = text.size(); }
	p.offset = offset;
	for (size_t i = 0; i < offset; ++i) {
		if (text[i] == '\n') {
			++p.line;
			p.column = 1;
		} else {
			++p.column;
		}
	}
	return p;
}

NOTRE_EXPORT struct error_info_t {
	error_kind kind = error_kind::none;
	size_t position = 0;
	size_t line = 1;
	size_t column = 1;

	constexpr bool ok() const noexcept {
		return kind == error_kind::none;
	}
};

namespace detail {

// run the CTLL parse of a pattern WITHOUT the hard static_assert that
// regex_builder carries, so the outcome can be queried
template <NOTRE_PATTERN_COPY input> struct parse_probe {
	static constexpr auto _input = input;
	using result = typename ctll::parser<notre::pcre, _input, notre::pcre_actions>::template output<pcre_context<>>;
	static constexpr bool ok = result::is_correct;
	// where the parse stopped: end of pattern when correct, else the
	// rejecting position
	static constexpr size_t position = result::is_correct ? _input.size() : result::position;
};

// the pattern's code units as bytes in static storage. In the default
// (byte) build a UTF-8 pattern is preserved verbatim, so a multi-byte
// character reproduces exactly and composes to one glyph; with
// NOTRE_STRING_IS_UTF8 the input is decoded and each code point is
// truncated to one byte here. render_snippet accounts for both when it
// places the caret.
template <NOTRE_PATTERN_COPY input> struct pattern_bytes {
	static constexpr size_t length = input.size();
	struct out_t {
		char content[length + 1]{};
	};
	static constexpr out_t make() noexcept {
		out_t o{};
		for (size_t i = 0; i < length; ++i) { o.content[i] = static_cast<char>(input[i]); }
		return o;
	}
	static constexpr out_t content = make();
	static constexpr std::string_view view{content.content, length};
};

// two sinks driving the size pass and the fill pass of the renderer
struct diag_count {
	size_t at = 0;
	constexpr void put(std::string_view s) noexcept {
		at += s.size();
	}
};
struct diag_fill {
	char * out;
	size_t at = 0;
	constexpr void put(std::string_view s) noexcept {
		for (const char c : s) { out[at++] = c; }
	}
};

template <typename Sink> constexpr void put_uint(Sink & s, size_t v) noexcept {
	char buf[20]{};
	size_t n = 0;
	do {
		buf[n++] = static_cast<char>('0' + v % 10);
		v /= 10;
	} while (v > 0);
	for (size_t i = 0; i < n / 2; ++i) {
		const char t = buf[i];
		buf[i] = buf[n - 1 - i];
		buf[n - 1 - i] = t;
	}
	s.put(std::string_view{buf, n});
}

inline constexpr size_t snippet_width = 72;
inline constexpr size_t snippet_caret_max = 60;

// the pattern line around pos, windowed so the caret is always visible
template <typename Sink>
constexpr void render_snippet(Sink & s, std::string_view text, size_t pos) noexcept {
	if (pos > text.size()) { pos = text.size(); }
	size_t ls = pos;
	while (ls > 0 && text[ls - 1] != '\n') { --ls; }
	size_t le = pos;
	while (le < text.size() && text[le] != '\n') { ++le; }
	size_t ws = ls;
	if (pos - ws > snippet_caret_max) { ws = pos - snippet_caret_max; }
	size_t we = le;
	if (we - ws > snippet_width) { we = ws + snippet_width; }
	s.put("\n  ");
	for (size_t i = ws; i < we; ++i) {
		const char c = text[i];
		s.put((c == '\t' || c == '\r') ? std::string_view{" "} : text.substr(i, 1));
	}
	s.put("\n  ");
	for (size_t i = ws; i < pos; ++i) {
#ifndef NOTRE_STRING_IS_UTF8
		// byte mode: the snippet holds raw UTF-8, so a multi-byte code
		// point renders as one glyph - count one caret cell per lead byte
		// (skip 10xxxxxx continuation bytes) so the caret lands under it
		if ((static_cast<unsigned char>(text[i]) & 0xC0) == 0x80) { continue; }
#endif
		s.put(" ");
	}
	s.put("^");
}

template <NOTRE_PATTERN_COPY input, typename Sink> constexpr void render_error(Sink & s) noexcept {
	using probe = parse_probe<input>;
	if (probe::ok) { return; }
	const std::string_view text = pattern_bytes<input>::view;
	const source_position at = locate(text, probe::position);
	s.put("notre: regex syntax error at position ");
	put_uint(s, probe::position);
	s.put(" (line ");
	put_uint(s, at.line);
	s.put(", column ");
	put_uint(s, at.column);
	s.put(")");
	render_snippet(s, text, probe::position);
}

template <NOTRE_PATTERN_COPY input> struct message_storage {
	static constexpr size_t measure() noexcept {
		diag_count c{};
		render_error<input>(c);
		return c.at;
	}
	static constexpr size_t length = measure();
	struct out_t {
		char content[length + 1]{};
	};
	static constexpr out_t compute() noexcept {
		out_t o{};
		diag_fill s{o.content};
		render_error<input>(s);
		return o;
	}
	static constexpr out_t content = compute();
	static constexpr std::string_view view{content.content, length};
};

} // namespace detail

// does the pattern compile? (a bool, never a compile error - the
// counterpart to match<>() which hard-errors on a bad pattern)
NOTRE_EXPORT template <NOTRE_PATTERN_INPUT pattern> constexpr bool valid = detail::parse_probe<pattern>::ok;

// what failed and where: kind, byte offset, line and column
// (kind == error_kind::none when the pattern is valid). Any arguments
// are ignored - these queries are about the PATTERN, not a subject - so
// you can debug a failing match by swapping match<P>(args...) for
// error_info<P>(args...) / error_message<P>(args...) without touching
// the call.
NOTRE_EXPORT template <NOTRE_PATTERN_INPUT pattern, typename... Subject>
constexpr error_info_t error_info(const Subject &...) noexcept {
	using probe = detail::parse_probe<pattern>;
	error_info_t e{};
	if (!probe::ok) {
		e.kind = error_kind::syntax;
		const source_position at = locate(detail::pattern_bytes<pattern>::view, probe::position);
		e.position = probe::position;
		e.line = at.line;
		e.column = at.column;
	}
	return e;
}

// the rendered diagnostic - position, line/column, and the pattern with
// a caret - as a static string ("" when the pattern is valid). Any
// arguments are ignored (see error_info above).
NOTRE_EXPORT template <NOTRE_PATTERN_INPUT pattern, typename... Subject>
constexpr std::string_view error_message(const Subject &...) noexcept {
	if constexpr (detail::parse_probe<pattern>::ok) {
		return std::string_view{};
	} else {
		return detail::message_storage<pattern>::view;
	}
}

} // namespace notre

#endif
