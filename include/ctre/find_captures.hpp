#ifndef CTRE__FIND_CAPTURES__HPP
#define CTRE__FIND_CAPTURES__HPP

#include "atoms_characters.hpp"
#include "atoms_unicode.hpp"
#include "utility.hpp"
#include "return_type.hpp"

namespace ctre {

CTRE_EXPORT template <typename Pattern> constexpr auto find_captures(Pattern) noexcept {
	return find_captures(ctll::list<Pattern>(), ctll::list<>());
}

CTRE_EXPORT template <typename... Output> constexpr auto find_captures(ctll::list<>, ctll::list<Output...> output) noexcept {
	return output;
}



CTRE_EXPORT template <auto... String, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<string<String...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Tail...>(), output);
}

CTRE_EXPORT template <typename... Options, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<select<Options...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Options..., Tail...>(), output);
}

CTRE_EXPORT template <typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<optional<Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}

CTRE_EXPORT template <typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<lazy_optional<Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}

CTRE_EXPORT template <typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<sequence<Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}

CTRE_EXPORT template <typename... Tail, typename Output> constexpr auto find_captures(ctll::list<empty, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Tail...>(), output);
}


CTRE_EXPORT template <typename... Tail, typename Output> constexpr auto find_captures(ctll::list<assert_subject_begin, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Tail...>(), output);
}


CTRE_EXPORT template <typename... Tail, typename Output> constexpr auto find_captures(ctll::list<assert_subject_end, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Tail...>(), output);
}


// , typename = std::enable_if_t<(MatchesCharacter<CharacterLike>::template value<char>)
CTRE_EXPORT template <typename CharacterLike, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<CharacterLike, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Tail...>(), output);
}


CTRE_EXPORT template <typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<plus<Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}

CTRE_EXPORT template <typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<star<Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}

CTRE_EXPORT template <size_t A, size_t B, typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<repeat<A,B,Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}


CTRE_EXPORT template <typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<lazy_plus<Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}

CTRE_EXPORT template <typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<lazy_star<Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}

CTRE_EXPORT template <size_t A, size_t B, typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<lazy_repeat<A,B,Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}


CTRE_EXPORT template <typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<possessive_plus<Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}

CTRE_EXPORT template <typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<possessive_star<Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}

CTRE_EXPORT template <size_t A, size_t B, typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<possessive_repeat<A,B,Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}



CTRE_EXPORT template <typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<lookahead_positive<Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}


CTRE_EXPORT template <typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<lookahead_negative<Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), output);
}




// captures in either conditional branch are real; a DEFINE body is never
// evaluated, so its groups get no result slot (they can only be reached
// through subroutine calls, which do not capture)
CTRE_EXPORT template <size_t Id, typename Yes, typename No, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<condition_capture<Id, Yes, No>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Yes, No, Tail...>(), output);
}

CTRE_EXPORT template <typename Name, typename Yes, typename No, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<condition_capture_with_name<Name, Yes, No>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Yes, No, Tail...>(), output);
}

CTRE_EXPORT template <typename... Content, typename... Tail, typename Output> constexpr auto find_captures(ctll::list<define_group<Content...>, Tail...>, Output output) noexcept {
	return find_captures(ctll::list<Tail...>(), output);
}

CTRE_EXPORT template <size_t Id, typename... Content, typename... Tail, typename... Output> constexpr auto find_captures(ctll::list<capture<Id,Content...>, Tail...>, ctll::list<Output...>) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), ctll::list<Output..., captured_content<Id>>());
}


CTRE_EXPORT template <size_t Id, typename Name, typename... Content, typename... Tail, typename... Output> constexpr auto find_captures(ctll::list<capture_with_name<Id,Name,Content...>, Tail...>, ctll::list<Output...>) noexcept {
	return find_captures(ctll::list<Content..., Tail...>(), ctll::list<Output..., captured_content<Id, Name>>());
}

}

#endif
