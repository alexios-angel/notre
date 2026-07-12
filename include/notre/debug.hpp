#ifndef NOTRE__DEBUG__HPP
#define NOTRE__DEBUG__HPP

#include "diagnostics.hpp"
#include "atoms.hpp"
#include "atoms_characters.hpp"
#include "atoms_unicode.hpp"
#include "resolve_subroutines.hpp"
#include "id.hpp"
#ifndef NOTRE_IN_A_MODULE
#include <cstddef>
#include <string_view>
#endif

// Tools for debugging a regular expression at compile time.
//
//   notre::debug::dump_ast<"a(b|c)+\\d">()
//
// renders the COMPILED form of the pattern - the atom tree the matcher
// actually walks, after desugaring, quantifier lowering, alternation
// and subroutine resolution - as an indented, static-storage string:
//
//   sequence
//     character 'a'
//     + (one or more) greedy
//       capture #1
//         alternation
//           character 'b'
//           character 'c'
//     set [...]
//       range '0'-'9'
//
// This is the regex analogue of a "show me what my pattern compiled to"
// tool: the single most useful thing when a pattern does not behave the
// way you read it. Matching itself already runs both at compile time
// and at runtime (that is notre's whole design), so to step through the
// matcher in a debugger you just call it on a runtime string in a normal
// build - no separate tracing facility is needed. See NOTRE_DEBUG /
// NOTRE_CONSTEXPR_ASSERT in assert.hpp for internal invariant checks.

namespace notre::debug {

namespace detail {

using notre::detail::diag_count;
using notre::detail::diag_fill;
using notre::detail::put_uint;

// --- the parsed atom tree of a pattern (subroutines resolved), matching
// what regex_builder<pattern>::type produces, but without the assert

template <NOTRE_PATTERN_COPY input> struct ast_of {
	using probe = notre::detail::parse_probe<input>;
	using parsed = ctll::conditional<probe::ok,
	    decltype(ctll::front(typename probe::result::output_type::stack_type())), ctll::list<notre::reject>>;
	using type = decltype(notre::resolve_subroutines(parsed{}));
};

// --- rendering primitives

template <typename Sink> constexpr void indent(Sink & s, size_t n) noexcept {
	for (size_t i = 0; i < n; ++i) { s.put(" "); }
}

// one readable code point. `delim` is the surrounding quote character to
// escape (' for character<>/ranges, " for string<>, 0 for bare names);
// only that delimiter is escaped, so the output is never malformed and
// never spuriously escapes the other quote.
template <typename Sink> constexpr void put_cp(Sink & s, char32_t v, char delim = '\'') noexcept {
	switch (v) {
		case U'\n': s.put("\\n"); return;
		case U'\t': s.put("\\t"); return;
		case U'\r': s.put("\\r"); return;
		case U'\f': s.put("\\f"); return;
		case U'\v': s.put("\\v"); return;
		case U'\0': s.put("\\0"); return;
		case U'\\': s.put("\\\\"); return;
		default: break;
	}
	if (delim != 0 && v == static_cast<char32_t>(static_cast<unsigned char>(delim))) {
		const char d = delim;
		s.put("\\");
		s.put(std::string_view{&d, 1});
		return;
	}
	if (v >= 0x20 && v < 0x7f) {
		const char c = static_cast<char>(v);
		s.put(std::string_view{&c, 1});
		return;
	}
	// non-printable / non-ASCII: \xHH or \u{HHHH}
	s.put(v < 0x100 ? std::string_view{"\\x"} : std::string_view{"\\u{"});
	const int digits = v < 0x100 ? 2 : (v < 0x10000 ? 4 : 6);
	for (int i = digits - 1; i >= 0; --i) {
		const unsigned nib = (static_cast<unsigned>(v) >> (4 * i)) & 0xF;
		const char hc = static_cast<char>(nib < 10 ? '0' + nib : 'a' + (nib - 10));
		s.put(std::string_view{&hc, 1});
	}
	if (v >= 0x100) { s.put("}"); }
}

// a name captured as id<chars...>
template <typename Sink, typename Name> constexpr void put_name(Sink & s, Name) noexcept;
template <typename Sink, auto... Cs> constexpr void put_name(Sink & s, notre::id<Cs...>) noexcept {
	(put_cp(s, static_cast<char32_t>(Cs), '\0'), ...); // bare name, no quote to escape
}
template <typename Sink, typename Name> constexpr void put_name(Sink & s, Name) noexcept {
	s.put("?");
}

// --- the node renderer: one dispatch per atom kind

template <typename Node> struct ast_node;

// the free dispatch is named differently from the members' render() to
// avoid unqualified-name collisions when a member calls it recursively
template <typename Node, typename Sink> constexpr void emit(Sink & s, size_t ind) noexcept {
	ast_node<Node>::render(s, ind);
}
template <typename Sink, typename... Cs> constexpr void render_children(Sink & s, size_t ind) noexcept {
	(emit<Cs>(s, ind), ...);
}

// a labelled line: <indent><label>\n
template <typename Sink> constexpr void line(Sink & s, size_t ind, std::string_view label) noexcept {
	indent(s, ind);
	s.put(label);
	s.put("\n");
}

// the quantifier label for repeat<A,B,...>: b==0 means unbounded
template <typename Sink> constexpr void quant_label(Sink & s, size_t a, size_t b) noexcept {
	if (a == 0 && b == 0) {
		s.put("* (zero or more)");
	} else if (a == 1 && b == 0) {
		s.put("+ (one or more)");
	} else if (a == 0 && b == 1) {
		s.put("? (optional)");
	} else if (b == 0) {
		s.put("{");
		put_uint(s, a);
		s.put(",} (at least ");
		put_uint(s, a);
		s.put(")");
	} else if (a == b) {
		s.put("{");
		put_uint(s, a);
		s.put("} (exactly ");
		put_uint(s, a);
		s.put(")");
	} else {
		s.put("{");
		put_uint(s, a);
		s.put(",");
		put_uint(s, b);
		s.put("}");
	}
}

template <typename Sink> constexpr void render_repeat(Sink & s, size_t ind, size_t a, size_t b,
                                                      std::string_view kind) noexcept {
	indent(s, ind);
	quant_label(s, a, b);
	s.put(" ");
	s.put(kind);
	s.put("\n");
}

// fallback: an atom we do not have a specialization for
template <typename Node> struct ast_node {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "<atom>");
	}
};

// structural
template <typename... Cs> struct ast_node<notre::sequence<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "sequence");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <typename... Cs> struct ast_node<notre::select<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "alternation");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <typename... Cs> struct ast_node<ctll::list<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "list");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <> struct ast_node<notre::empty> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "empty");
	}
};
template <> struct ast_node<notre::accept> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "accept");
	}
};
template <> struct ast_node<notre::reject> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "reject");
	}
};
template <> struct ast_node<notre::any> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, ". (any character)");
	}
};

// literals
template <auto V> struct ast_node<notre::character<V>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("character '");
		put_cp(s, static_cast<char32_t>(V));
		s.put("'\n");
	}
};
template <auto... Str> struct ast_node<notre::string<Str...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("string \"");
		(put_cp(s, static_cast<char32_t>(Str), '"'), ...);
		s.put("\"\n");
	}
};

// quantifiers (plus/star/optional are aliases of these)
template <size_t A, size_t B, typename... Cs> struct ast_node<notre::repeat<A, B, Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		render_repeat(s, ind, A, B, "greedy");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <size_t A, size_t B, typename... Cs> struct ast_node<notre::lazy_repeat<A, B, Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		render_repeat(s, ind, A, B, "lazy");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <size_t A, size_t B, typename... Cs> struct ast_node<notre::possessive_repeat<A, B, Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		render_repeat(s, ind, A, B, "possessive");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};

// captures and references
template <size_t Idx, typename... Cs> struct ast_node<notre::capture<Idx, Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("capture #");
		put_uint(s, Idx);
		s.put("\n");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <size_t Idx, typename Name, typename... Cs> struct ast_node<notre::capture_with_name<Idx, Name, Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("capture #");
		put_uint(s, Idx);
		s.put(" <");
		put_name(s, Name{});
		s.put(">\n");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <size_t Idx> struct ast_node<notre::back_reference<Idx>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("back-reference \\");
		put_uint(s, Idx);
		s.put("\n");
	}
};
template <typename Name> struct ast_node<notre::back_reference_with_name<Name>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("back-reference \\k<");
		put_name(s, Name{});
		s.put(">\n");
	}
};
template <size_t Idx> struct ast_node<notre::subroutine_call<Idx>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("subroutine (?");
		put_uint(s, Idx);
		s.put(")\n");
	}
};
template <typename Name> struct ast_node<notre::subroutine_call_with_name<Name>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("subroutine (?&");
		put_name(s, Name{});
		s.put(")\n");
	}
};
template <size_t Idx, typename Yes, typename No> struct ast_node<notre::condition_capture<Idx, Yes, No>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("conditional on capture #");
		put_uint(s, Idx);
		s.put("\n");
		line(s, ind + 2, "if-matched:");
		emit<Yes>(s, ind + 4);
		line(s, ind + 2, "else:");
		emit<No>(s, ind + 4);
	}
};
template <typename Name, typename Yes, typename No>
struct ast_node<notre::condition_capture_with_name<Name, Yes, No>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("conditional on capture <");
		put_name(s, Name{});
		s.put(">\n");
		line(s, ind + 2, "if-matched:");
		emit<Yes>(s, ind + 4);
		line(s, ind + 2, "else:");
		emit<No>(s, ind + 4);
	}
};

// groups and look-arounds
template <typename... Cs> struct ast_node<notre::atomic_group<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "atomic group (?>...)");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <typename... Cs> struct ast_node<notre::lookahead_positive<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "lookahead (?=...)");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <typename... Cs> struct ast_node<notre::lookahead_negative<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "negative lookahead (?!...)");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <typename... Cs> struct ast_node<notre::lookbehind_positive<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "lookbehind (?<=...)");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <typename... Cs> struct ast_node<notre::lookbehind_negative<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "negative lookbehind (?<!...)");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <typename... Cs> struct ast_node<notre::define_group<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "define (?(DEFINE)...)");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};

// anchors and marks
template <> struct ast_node<notre::assert_subject_begin> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "anchor \\A (subject begin)");
	}
};
template <> struct ast_node<notre::assert_subject_end> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "anchor \\z (subject end)");
	}
};
template <> struct ast_node<notre::assert_subject_end_line> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "anchor \\Z (subject end, before a final newline)");
	}
};
template <> struct ast_node<notre::assert_line_begin> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "anchor ^ (line begin)");
	}
};
template <> struct ast_node<notre::assert_line_end> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "anchor $ (line end)");
	}
};
template <> struct ast_node<notre::match_point_reset> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "match reset \\K");
	}
};

// \b is boundary<word_chars>; \B is boundary<negative_set<word_chars>>
// (what the parser actually emits) - label both instead of dumping the
// underlying character-set subtree
template <> struct ast_node<notre::word_boundary> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "word boundary \\b");
	}
};
template <> struct ast_node<notre::boundary<notre::negative_set<notre::word_chars>>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "not a word boundary \\B");
	}
};
template <typename... Cs> struct ast_node<notre::boundary<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "boundary");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <typename... Cs> struct ast_node<notre::not_boundary<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "not-boundary");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};

// character classes
template <typename... Cs> struct ast_node<notre::set<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "set [...]");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <typename... Cs> struct ast_node<notre::negative_set<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "negated set [^...]");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <typename... Cs> struct ast_node<notre::negate<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "negate");
		render_children<Sink, Cs...>(s, ind + 2);
	}
};
template <auto A, auto B> struct ast_node<notre::char_range<A, B>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("range '");
		put_cp(s, static_cast<char32_t>(A));
		s.put("'-'");
		put_cp(s, static_cast<char32_t>(B));
		s.put("'\n");
	}
};
template <auto... Cs> struct ast_node<notre::enumeration<Cs...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("one of '");
		(put_cp(s, static_cast<char32_t>(Cs)), ...);
		s.put("'\n");
	}
};

// inline mode switches, e.g. (?i)
template <typename T> struct ast_node<notre::mode_switch<T>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "mode switch");
	}
};

// callouts (?C1) / (?C'name')
template <size_t N> struct ast_node<notre::callout_numbered<N>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("callout (?C");
		put_uint(s, N);
		s.put(")\n");
	}
};
template <typename Name> struct ast_node<notre::callout_named<Name>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("callout (?C'");
		put_name(s, Name{});
		s.put("')\n");
	}
};
template <typename Handler, typename Name, size_t N> struct ast_node<notre::callout<Handler, Name, N>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "callout (bound)");
	}
};

// unicode properties \p{...} (rendered generically; the property enum
// values do not carry a constexpr-printable name here)
template <auto... S> struct ast_node<notre::property_name<S...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("property name '");
		(put_cp(s, static_cast<char32_t>(S)), ...);
		s.put("'\n");
	}
};
template <auto... S> struct ast_node<notre::property_value<S...>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		indent(s, ind);
		s.put("property value '");
		(put_cp(s, static_cast<char32_t>(S)), ...);
		s.put("'\n");
	}
};
template <typename T, T Type> struct ast_node<notre::binary_property<T, Type>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "unicode property \\p{...}");
	}
};
template <typename T, T Type, auto Value> struct ast_node<notre::property<T, Type, Value>> {
	template <typename Sink> static constexpr void render(Sink & s, size_t ind) noexcept {
		line(s, ind, "unicode property \\p{...=...}");
	}
};

// --- storage: measure then fill

template <NOTRE_PATTERN_COPY input> struct ast_storage {
	using node = typename ast_of<input>::type;
	static constexpr size_t measure() noexcept {
		diag_count c{};
		emit<node>(c, 0);
		return c.at;
	}
	static constexpr size_t length = measure();
	struct out_t {
		char content[length + 1]{};
	};
	static constexpr out_t compute() noexcept {
		out_t o{};
		diag_fill s{o.content};
		emit<node>(s, 0);
		return o;
	}
	static constexpr out_t content = compute();
	static constexpr std::string_view view{content.content, length};
};

} // namespace detail

// the compiled atom tree of a pattern as an indented static string; for
// a pattern that does not compile, the diagnostic from error_message()
NOTRE_EXPORT template <NOTRE_PATTERN_INPUT pattern> constexpr std::string_view dump_ast() noexcept {
	if constexpr (notre::detail::parse_probe<pattern>::ok) {
		return detail::ast_storage<pattern>::view;
	} else {
		return notre::error_message<pattern>();
	}
}

} // namespace notre::debug

#endif
