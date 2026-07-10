#ifndef CTRE__RESOLVE_SUBROUTINES__HPP
#define CTRE__RESOLVE_SUBROUTINES__HPP

#include "atoms.hpp"
#include "atoms_unicode.hpp"
#include "rotate.hpp"
#include "id.hpp"
#include "utility.hpp"
#ifndef CTRE_IN_A_MODULE
#include <type_traits>
#endif

// Replaces subroutine-call placeholders — (?N), (?&name), \g<name>, ... —
// with an inlined copy of the referenced group, after the pattern has been
// parsed. Matching PCRE semantics, the inlined copy is atomic and does not
// capture: captures set during a call revert when it returns, so the copy
// has its capture groups turned into plain sequences.
//
// A call inside a lookbehind is rotated the same way the surrounding
// lookbehind content was rotated by the parser, and is inlined as a plain
// (non-atomic) sequence, because the evaluator cannot run atomic groups on
// reversed input — the same limitation applies to writing (?>...) inside
// (?<=...) directly.
//
// Recursive calls — a group calling itself, directly or through other
// groups — would need an unbounded expansion and are rejected with a
// static_assert; so are calls to groups that do not exist.

namespace ctre {

struct subroutine_not_found { };

template <size_t> constexpr bool subroutine_id_dependent_false = false;

// how a subroutine call site behaves during resolution
enum class subroutine_mode {
	normal,           // expansion is wrapped as an atomic group
	lookbehind,       // marker sits in parser-rotated content: expansion is
	                  // built in original orientation, then rotated, non-atomic
	lookbehind_inner  // inside an expansion that a lookbehind call started:
	                  // non-atomic, no additional rotation
};

// --- lookup of the referenced group anywhere in the pattern (worklist walk)

template <size_t Id> constexpr auto subroutine_target_by_id(ctll::list<>) noexcept {
	return subroutine_not_found{};
}

template <size_t Id, size_t Id2, typename... Content, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<capture<Id2, Content...>, Tail...>) noexcept {
	if constexpr (Id == Id2) {
		return capture<Id2, Content...>{};
	} else {
		return subroutine_target_by_id<Id>(ctll::list<Content..., Tail...>{});
	}
}

template <size_t Id, size_t Id2, typename Name, typename... Content, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<capture_with_name<Id2, Name, Content...>, Tail...>) noexcept {
	if constexpr (Id == Id2) {
		return capture_with_name<Id2, Name, Content...>{};
	} else {
		return subroutine_target_by_id<Id>(ctll::list<Content..., Tail...>{});
	}
}

template <size_t Id, typename... Content, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<sequence<Content...>, Tail...>) noexcept {
	return subroutine_target_by_id<Id>(ctll::list<Content..., Tail...>{});
}

template <size_t Id, typename... Options, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<select<Options...>, Tail...>) noexcept {
	return subroutine_target_by_id<Id>(ctll::list<Options..., Tail...>{});
}

template <size_t Id, size_t A, size_t B, typename... Content, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<repeat<A, B, Content...>, Tail...>) noexcept {
	return subroutine_target_by_id<Id>(ctll::list<Content..., Tail...>{});
}

template <size_t Id, size_t A, size_t B, typename... Content, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<lazy_repeat<A, B, Content...>, Tail...>) noexcept {
	return subroutine_target_by_id<Id>(ctll::list<Content..., Tail...>{});
}

template <size_t Id, size_t A, size_t B, typename... Content, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<possessive_repeat<A, B, Content...>, Tail...>) noexcept {
	return subroutine_target_by_id<Id>(ctll::list<Content..., Tail...>{});
}

template <size_t Id, typename... Content, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<atomic_group<Content...>, Tail...>) noexcept {
	return subroutine_target_by_id<Id>(ctll::list<Content..., Tail...>{});
}

template <size_t Id, typename... Content, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<lookahead_positive<Content...>, Tail...>) noexcept {
	return subroutine_target_by_id<Id>(ctll::list<Content..., Tail...>{});
}

template <size_t Id, typename... Content, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<lookahead_negative<Content...>, Tail...>) noexcept {
	return subroutine_target_by_id<Id>(ctll::list<Content..., Tail...>{});
}

template <size_t Id, typename... Content, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<lookbehind_positive<Content...>, Tail...>) noexcept {
	return subroutine_target_by_id<Id>(ctll::list<Content..., Tail...>{});
}

template <size_t Id, typename... Content, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<lookbehind_negative<Content...>, Tail...>) noexcept {
	return subroutine_target_by_id<Id>(ctll::list<Content..., Tail...>{});
}

template <size_t Id, typename Head, typename... Tail> constexpr auto subroutine_target_by_id(ctll::list<Head, Tail...>) noexcept {
	return subroutine_target_by_id<Id>(ctll::list<Tail...>{});
}

// same walk, selecting by group name

template <typename Name> constexpr auto subroutine_target_by_name(ctll::list<>) noexcept {
	return subroutine_not_found{};
}

template <typename Name, size_t Id2, typename Name2, typename... Content, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<capture_with_name<Id2, Name2, Content...>, Tail...>) noexcept {
	if constexpr (std::is_same_v<Name, Name2>) {
		return capture_with_name<Id2, Name2, Content...>{};
	} else {
		return subroutine_target_by_name<Name>(ctll::list<Content..., Tail...>{});
	}
}

template <typename Name, size_t Id2, typename... Content, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<capture<Id2, Content...>, Tail...>) noexcept {
	return subroutine_target_by_name<Name>(ctll::list<Content..., Tail...>{});
}

template <typename Name, typename... Content, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<sequence<Content...>, Tail...>) noexcept {
	return subroutine_target_by_name<Name>(ctll::list<Content..., Tail...>{});
}

template <typename Name, typename... Options, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<select<Options...>, Tail...>) noexcept {
	return subroutine_target_by_name<Name>(ctll::list<Options..., Tail...>{});
}

template <typename Name, size_t A, size_t B, typename... Content, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<repeat<A, B, Content...>, Tail...>) noexcept {
	return subroutine_target_by_name<Name>(ctll::list<Content..., Tail...>{});
}

template <typename Name, size_t A, size_t B, typename... Content, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<lazy_repeat<A, B, Content...>, Tail...>) noexcept {
	return subroutine_target_by_name<Name>(ctll::list<Content..., Tail...>{});
}

template <typename Name, size_t A, size_t B, typename... Content, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<possessive_repeat<A, B, Content...>, Tail...>) noexcept {
	return subroutine_target_by_name<Name>(ctll::list<Content..., Tail...>{});
}

template <typename Name, typename... Content, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<atomic_group<Content...>, Tail...>) noexcept {
	return subroutine_target_by_name<Name>(ctll::list<Content..., Tail...>{});
}

template <typename Name, typename... Content, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<lookahead_positive<Content...>, Tail...>) noexcept {
	return subroutine_target_by_name<Name>(ctll::list<Content..., Tail...>{});
}

template <typename Name, typename... Content, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<lookahead_negative<Content...>, Tail...>) noexcept {
	return subroutine_target_by_name<Name>(ctll::list<Content..., Tail...>{});
}

template <typename Name, typename... Content, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<lookbehind_positive<Content...>, Tail...>) noexcept {
	return subroutine_target_by_name<Name>(ctll::list<Content..., Tail...>{});
}

template <typename Name, typename... Content, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<lookbehind_negative<Content...>, Tail...>) noexcept {
	return subroutine_target_by_name<Name>(ctll::list<Content..., Tail...>{});
}

template <typename Name, typename Head, typename... Tail> constexpr auto subroutine_target_by_name(ctll::list<Head, Tail...>) noexcept {
	return subroutine_target_by_name<Name>(ctll::list<Tail...>{});
}

// --- turning the inlined copy into a non-capturing one

template <typename T> constexpr auto strip_captures(T) noexcept {
	return T{};
}

template <typename... Content> constexpr auto strip_captures(sequence<Content...>) noexcept {
	return sequence<decltype(strip_captures(Content{}))...>{};
}

template <typename... Options> constexpr auto strip_captures(select<Options...>) noexcept {
	return select<decltype(strip_captures(Options{}))...>{};
}

template <size_t A, size_t B, typename... Content> constexpr auto strip_captures(repeat<A, B, Content...>) noexcept {
	return repeat<A, B, decltype(strip_captures(Content{}))...>{};
}

template <size_t A, size_t B, typename... Content> constexpr auto strip_captures(lazy_repeat<A, B, Content...>) noexcept {
	return lazy_repeat<A, B, decltype(strip_captures(Content{}))...>{};
}

template <size_t A, size_t B, typename... Content> constexpr auto strip_captures(possessive_repeat<A, B, Content...>) noexcept {
	return possessive_repeat<A, B, decltype(strip_captures(Content{}))...>{};
}

template <typename... Content> constexpr auto strip_captures(atomic_group<Content...>) noexcept {
	return atomic_group<decltype(strip_captures(Content{}))...>{};
}

template <typename... Content> constexpr auto strip_captures(lookahead_positive<Content...>) noexcept {
	return lookahead_positive<decltype(strip_captures(Content{}))...>{};
}

template <typename... Content> constexpr auto strip_captures(lookahead_negative<Content...>) noexcept {
	return lookahead_negative<decltype(strip_captures(Content{}))...>{};
}

template <typename... Content> constexpr auto strip_captures(lookbehind_positive<Content...>) noexcept {
	return lookbehind_positive<decltype(strip_captures(Content{}))...>{};
}

template <typename... Content> constexpr auto strip_captures(lookbehind_negative<Content...>) noexcept {
	return lookbehind_negative<decltype(strip_captures(Content{}))...>{};
}

template <size_t Id, typename... Content> constexpr auto strip_captures(capture<Id, Content...>) noexcept {
	return sequence<decltype(strip_captures(Content{}))...>{};
}

template <size_t Id, typename Name, typename... Content> constexpr auto strip_captures(capture_with_name<Id, Name, Content...>) noexcept {
	return sequence<decltype(strip_captures(Content{}))...>{};
}

// --- the resolver itself
//
// Whole is the complete parsed pattern (for lookups), the ctll::list of
// numeric_mark<Id> carries the groups currently being expanded (for cycle
// detection), and the subroutine_mode says how a call site must expand.

template <typename Whole> struct subroutine_resolver {

	template <size_t Id, subroutine_mode Mode, typename... Visited> static constexpr auto expand_call(ctll::list<Visited...>) noexcept {
		using target = decltype(subroutine_target_by_id<Id>(ctll::list<Whole>{}));
		static_assert(!std::is_same_v<target, subroutine_not_found> || subroutine_id_dependent_false<Id>, "subroutine call (?N) references a capture group which does not exist in the pattern");
		return expand_target<target, Mode>(ctll::list<Visited...>{});
	}

	template <typename Name, subroutine_mode Mode, typename... Visited> static constexpr auto expand_call_with_name(ctll::list<Visited...>) noexcept {
		using target = decltype(subroutine_target_by_name<Name>(ctll::list<Whole>{}));
		static_assert(!std::is_same_v<target, subroutine_not_found> || subroutine_id_dependent_false<sizeof...(Visited)>, "subroutine call (?&name) references a capture group name which does not exist in the pattern");
		return expand_target<target, Mode>(ctll::list<Visited...>{});
	}

	template <typename Target, subroutine_mode Mode, typename... Visited> static constexpr auto expand_target(ctll::list<Visited...>) noexcept {
		if constexpr (std::is_same_v<Target, subroutine_not_found>) {
			return reject{};
		} else {
			return expand_group<Mode>(Target{}, ctll::list<Visited...>{});
		}
	}

	template <subroutine_mode Mode, size_t Id, typename... Content, typename... Visited> static constexpr auto expand_group(capture<Id, Content...>, ctll::list<Visited...>) noexcept {
		constexpr bool recursive = (std::is_same_v<numeric_mark<Id>, Visited> || ...);
		static_assert(!recursive || subroutine_id_dependent_false<Id>, "recursive subroutine calls are not supported (the pattern is expanded at compile time and recursion would never finish)");
		// nested calls in the expansion keep the original orientation; only
		// the outermost lookbehind call site rotates the whole expansion
		constexpr subroutine_mode inner = (Mode == subroutine_mode::normal) ? subroutine_mode::normal : subroutine_mode::lookbehind_inner;
		if constexpr (recursive) {
			return reject{};
		} else if constexpr (Mode == subroutine_mode::normal) {
			return atomic_group<decltype(strip_captures(resolve<inner>(ctll::list<numeric_mark<Id>, Visited...>{}, Content{})))...>{};
		} else {
			return sequence<decltype(strip_captures(resolve<inner>(ctll::list<numeric_mark<Id>, Visited...>{}, Content{})))...>{};
		}
	}

	template <subroutine_mode Mode, size_t Id, typename Name, typename... Content, typename... Visited> static constexpr auto expand_group(capture_with_name<Id, Name, Content...>, ctll::list<Visited...>) noexcept {
		return expand_group<Mode>(capture<Id, Content...>{}, ctll::list<Visited...>{});
	}

	// leaves stay what they are
	template <subroutine_mode Mode, typename Visited, typename T> static constexpr auto resolve(Visited, T) noexcept {
		return T{};
	}

	// the calls: atomic per PCRE; inside a lookbehind the expansion is
	// rotated once at the outermost call site and inlined non-atomically
	template <subroutine_mode Mode, typename Visited, size_t Id> static constexpr auto resolve(Visited, subroutine_call<Id>) noexcept {
		using expanded = decltype(expand_call<Id, Mode>(Visited{}));
		if constexpr (Mode == subroutine_mode::lookbehind) {
			return decltype(rotate_for_lookbehind::rotate(expanded{})){};
		} else {
			return expanded{};
		}
	}

	template <subroutine_mode Mode, typename Visited, typename Name> static constexpr auto resolve(Visited, subroutine_call_with_name<Name>) noexcept {
		using expanded = decltype(expand_call_with_name<Name, Mode>(Visited{}));
		if constexpr (Mode == subroutine_mode::lookbehind) {
			return decltype(rotate_for_lookbehind::rotate(expanded{})){};
		} else {
			return expanded{};
		}
	}

	// compound nodes are rebuilt with their content resolved
	template <subroutine_mode Mode, typename Visited, typename... Content> static constexpr auto resolve(Visited v, sequence<Content...>) noexcept {
		return sequence<decltype(resolve<Mode>(v, Content{}))...>{};
	}

	template <subroutine_mode Mode, typename Visited, typename... Options> static constexpr auto resolve(Visited v, select<Options...>) noexcept {
		return select<decltype(resolve<Mode>(v, Options{}))...>{};
	}

	template <subroutine_mode Mode, typename Visited, size_t A, size_t B, typename... Content> static constexpr auto resolve(Visited v, repeat<A, B, Content...>) noexcept {
		return repeat<A, B, decltype(resolve<Mode>(v, Content{}))...>{};
	}

	template <subroutine_mode Mode, typename Visited, size_t A, size_t B, typename... Content> static constexpr auto resolve(Visited v, lazy_repeat<A, B, Content...>) noexcept {
		return lazy_repeat<A, B, decltype(resolve<Mode>(v, Content{}))...>{};
	}

	template <subroutine_mode Mode, typename Visited, size_t A, size_t B, typename... Content> static constexpr auto resolve(Visited v, possessive_repeat<A, B, Content...>) noexcept {
		return possessive_repeat<A, B, decltype(resolve<Mode>(v, Content{}))...>{};
	}

	template <subroutine_mode Mode, typename Visited, typename... Content> static constexpr auto resolve(Visited v, atomic_group<Content...>) noexcept {
		return atomic_group<decltype(resolve<Mode>(v, Content{}))...>{};
	}

	template <subroutine_mode Mode, typename Visited, size_t Id, typename... Content> static constexpr auto resolve(Visited v, capture<Id, Content...>) noexcept {
		return capture<Id, decltype(resolve<Mode>(v, Content{}))...>{};
	}

	template <subroutine_mode Mode, typename Visited, size_t Id, typename Name, typename... Content> static constexpr auto resolve(Visited v, capture_with_name<Id, Name, Content...>) noexcept {
		return capture_with_name<Id, Name, decltype(resolve<Mode>(v, Content{}))...>{};
	}

	template <subroutine_mode Mode, typename Visited, typename... Content> static constexpr auto resolve(Visited v, lookahead_positive<Content...>) noexcept {
		return lookahead_positive<decltype(resolve<Mode>(v, Content{}))...>{};
	}

	template <subroutine_mode Mode, typename Visited, typename... Content> static constexpr auto resolve(Visited v, lookahead_negative<Content...>) noexcept {
		return lookahead_negative<decltype(resolve<Mode>(v, Content{}))...>{};
	}

	// lookbehind content was rotated during parsing, so call sites inside
	// switch to the rotating mode
	template <subroutine_mode Mode, typename Visited, typename... Content> static constexpr auto resolve(Visited v, lookbehind_positive<Content...>) noexcept {
		return lookbehind_positive<decltype(resolve<subroutine_mode::lookbehind>(v, Content{}))...>{};
	}

	template <subroutine_mode Mode, typename Visited, typename... Content> static constexpr auto resolve(Visited v, lookbehind_negative<Content...>) noexcept {
		return lookbehind_negative<decltype(resolve<subroutine_mode::lookbehind>(v, Content{}))...>{};
	}
};

// entry point: resolves every subroutine call in a parsed pattern
CTRE_EXPORT template <typename RE> constexpr auto resolve_subroutines(RE re) noexcept {
	return subroutine_resolver<RE>::template resolve<subroutine_mode::normal>(ctll::list<>{}, re);
}

}

#endif
