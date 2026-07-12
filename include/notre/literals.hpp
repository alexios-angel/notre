#ifndef NOTRE_V2__NOTRE__LITERALS__HPP
#define NOTRE_V2__NOTRE__LITERALS__HPP

#include "../ctll.hpp"
#include "pcre_actions.hpp"
#include "evaluation.hpp"
#include "wrapper.hpp"
#include "id.hpp"

#ifndef __EDG__

namespace notre {

// in C++17 (clang & gcc with gnu extension) we need translate character pack into ctll::fixed_string
// in C++20 we have `class nontype template parameters`

#if !NOTRE_CNTTP_COMPILER_CHECK
template <typename CharT, CharT... input> static inline constexpr auto _fixed_string_reference = ctll::fixed_string< sizeof...(input)>({input...});
#endif	

namespace literals {
	
// clang and GCC <9 supports LITERALS with packs

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
#define NOTRE_ENABLE_LITERALS
#endif
  
#ifdef _MSC_VER
#ifdef _MSVC_LANG
#if _MSVC_LANG >= 202002L
#define NOTRE_ENABLE_LITERALS
#endif
#else
#define NOTRE_ENABLE_LITERALS
#endif
#endif

#ifdef __INTEL_COMPILER
// not enable literals
#elif defined __GNUC__
#if __GNUC__ < 9
#define NOTRE_ENABLE_LITERALS
#elif __GNUC__ >= 10
#if !NOTRE_CNTTP_COMPILER_CHECK 
// newer versions of GCC will give error when trying to use GNU extension
#else
#define NOTRE_ENABLE_LITERALS
#endif	
#endif
#endif

#ifdef NOTRE_ENABLE_LITERALS
	
// add this when we will have concepts
// requires ctll::parser<notre::pcre, _fixed_string_reference<CharT, charpack...>, notre::pcre_actions>::template correct_with<pcre_context<>>

#if !NOTRE_CNTTP_COMPILER_CHECK
template <typename CharT, CharT... charpack> NOTRE_FLATTEN constexpr NOTRE_FORCE_INLINE auto operator""_notre() noexcept {
	constexpr auto & _input = _fixed_string_reference<CharT, charpack...>;
#else
template <ctll::fixed_string input> NOTRE_FLATTEN constexpr NOTRE_FORCE_INLINE auto operator""_notre() noexcept {
	constexpr auto _input = input; // workaround for GCC 9 bug 88092
#endif
	using tmp = typename ctll::parser<notre::pcre, _input, notre::pcre_actions>::template output<pcre_context<>>;
	static_assert(tmp(), "Regular Expression contains syntax error.");
	if constexpr (tmp()) {
		using re = decltype(notre::resolve_subroutines(front(typename tmp::output_type::stack_type())));
		return notre::regular_expression(re());
	} else {
		return notre::regular_expression(reject());
	}
}



// this will need to be fixed with C++20
#if !NOTRE_CNTTP_COMPILER_CHECK
template <typename CharT, CharT... charpack> NOTRE_FLATTEN constexpr NOTRE_FORCE_INLINE auto operator""_notre_id() noexcept {
	return id<charpack...>();
}
#endif

#endif // NOTRE_ENABLE_LITERALS

}

namespace test_literals {
	
#ifdef NOTRE_ENABLE_LITERALS

#if !NOTRE_CNTTP_COMPILER_CHECK
template <typename CharT, CharT... charpack> NOTRE_FLATTEN constexpr inline auto operator""_notre_test() noexcept {
	constexpr auto & _input = _fixed_string_reference<CharT, charpack...>;
#else
template <ctll::fixed_string input> NOTRE_FLATTEN constexpr inline auto operator""_notre_test() noexcept {
	constexpr auto _input = input; // workaround for GCC 9 bug 88092
#endif
	return ctll::parser<notre::pcre, _input>::template correct_with<>;
}

#if !NOTRE_CNTTP_COMPILER_CHECK
template <typename CharT, CharT... charpack> NOTRE_FLATTEN constexpr inline auto operator""_notre_gen() noexcept {
	constexpr auto & _input = _fixed_string_reference<CharT, charpack...>;
#else
template <ctll::fixed_string input> NOTRE_FLATTEN constexpr inline auto operator""_notre_gen() noexcept {
	constexpr auto _input = input; // workaround for GCC 9 bug 88092
#endif
	using tmp = typename ctll::parser<notre::pcre, _input, notre::pcre_actions>::template output<pcre_context<>>;
	static_assert(tmp(), "Regular Expression contains syntax error.");
	return typename tmp::output_type::stack_type();
}


#if !NOTRE_CNTTP_COMPILER_CHECK
template <typename CharT, CharT... charpack> NOTRE_FLATTEN constexpr NOTRE_FORCE_INLINE auto operator""_notre_syntax() noexcept {
	constexpr auto & _input = _fixed_string_reference<CharT, charpack...>;
#else
template <ctll::fixed_string input> NOTRE_FLATTEN constexpr NOTRE_FORCE_INLINE auto operator""_notre_syntax() noexcept {
	constexpr auto _input = input; // workaround for GCC 9 bug 88092
#endif
	return ctll::parser<notre::pcre, _input, notre::pcre_actions>::template correct_with<pcre_context<>>;
}


#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

} // literals

} // notre

#endif

#endif
