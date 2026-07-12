#ifndef NOTRE__UTILITY__HPP
#define NOTRE__UTILITY__HPP

#include "../ctll/utilities.hpp"

#define NOTRE_CNTTP_COMPILER_CHECK CTLL_CNTTP_COMPILER_CHECK

#ifdef NOTRE_IN_A_MODULE
#define NOTRE_EXPORT export
#else
#define NOTRE_EXPORT 
#endif

#if __GNUC__ > 9
#if __has_cpp_attribute(likely)
#define NOTRE_LIKELY [[likely]]
#else
#define NOTRE_LIKELY
#endif

#if __has_cpp_attribute(unlikely)
#define NOTRE_UNLIKELY [[unlikely]]
#else
#define NOTRE_UNLIKELY
#endif
#else
#define NOTRE_LIKELY
#define NOTRE_UNLIKELY
#endif

#ifdef _MSC_VER
#define NOTRE_FORCE_INLINE __forceinline
#if __has_cpp_attribute(msvc::flatten)
#define NOTRE_FLATTEN [[msvc::flatten]]
#elif _MSC_VER >= 1930 && !defined(__clang__)
#define NOTRE_FLATTEN [[msvc::flatten]]
#else
#define NOTRE_FLATTEN
#endif
#else
#define NOTRE_FORCE_INLINE inline __attribute__((always_inline))
#define NOTRE_FLATTEN __attribute__((flatten))
#endif

#endif
