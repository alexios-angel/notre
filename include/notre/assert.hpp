#ifndef NOTRE__ASSERT__HPP
#define NOTRE__ASSERT__HPP

#ifndef NOTRE_IN_A_MODULE
#include <cstdlib>
#endif

// Internal invariant checks that also work during constant evaluation.
//
// Define NOTRE_DEBUG to enable them. When a check fails while the
// compiler is evaluating a constexpr match, evaluation stops with an
// error pointing at the NOTRE_CONSTEXPR_ASSERT line: the call to
// constexpr_assert_failed is the diagnostic, because that function is
// deliberately not constexpr and the compiler quotes the call - with
// the message literal - when it rejects it in a constant expression.
// The same check running at runtime aborts. Without NOTRE_DEBUG the
// checks compile away entirely.

namespace notre::detail {

// not constexpr, on purpose: reaching this call during constant
// evaluation is what produces the compiler error
[[noreturn]] inline void constexpr_assert_failed(const char * /*msg*/) noexcept {
	std::abort();
}

} // namespace notre::detail

#ifdef NOTRE_DEBUG
#define NOTRE_CONSTEXPR_ASSERT(cond, msg) \
	do { \
		if (!(cond)) { ::notre::detail::constexpr_assert_failed(msg); } \
	} while (false)
#else
#define NOTRE_CONSTEXPR_ASSERT(cond, msg) ((void)0)
#endif

#endif
