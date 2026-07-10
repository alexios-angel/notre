#ifndef CTLL__FUNCTION__HPP
#define CTLL__FUNCTION__HPP

#ifndef CTLL_IN_A_MODULE
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#endif

#include "utilities.hpp"

// A constexpr type-erased callable wrapper: ctll::function is to constant
// evaluation what std::function is to runtime. It owns a copy of any
// callable with a matching signature - lambdas with captures, function
// pointers, pointers to members - behind a constexpr-virtual interface,
// so it can be created, copied, invoked and destroyed inside a constant
// expression. This is the type-erasure vehicle intended to carry
// user-supplied hooks (e.g. regex callouts) through the library.
//
// Based on https://stackoverflow.com/a/76776249 (constexpr std::function,
// after Kris Jusiak's C++ Tip of the Week #318), with plain owning
// new/delete instead of std::unique_ptr so C++20 suffices (constexpr
// virtual functions and constexpr allocation) instead of C++23's
// constexpr std::unique_ptr.
//
// Invocation follows INVOKE<R> like std::function: pointers to member
// functions and to data members work, and when R is void the callable's
// result is discarded. A null function/member pointer constructs an empty
// ctll::function, as with std::function.
//
// Differences from std::function: calling an empty ctll::function is
// undefined behaviour instead of throwing (this library is exception
// free; in a constant expression it fails to compile), there is no
// RTTI-based target()/target_type() access (the library must stay usable
// with -fno-rtti), and there is no small-buffer optimization (during
// constant evaluation every allocation is transient anyway).

#if !(defined(__cpp_constexpr_dynamic_alloc) && __cpp_constexpr_dynamic_alloc >= 201907L)
#error "ctll::function requires C++20 (constexpr virtual functions and constexpr dynamic allocation)"
#endif

namespace ctll {

CTLL_EXPORT template <typename> class function;

CTLL_EXPORT template <typename R, typename... Args> class function<R(Args...)> {
	struct interface {
		constexpr virtual R invoke(Args... args) = 0;
		constexpr virtual interface * clone() const = 0;
		constexpr virtual ~interface() = default;
	};

	template <typename F> struct implementation final: interface {
		F callable;

		constexpr explicit implementation(F f) noexcept(std::is_nothrow_move_constructible_v<F>): callable(std::move(f)) { }
		constexpr R invoke(Args... args) override {
			if constexpr (std::is_void_v<R>) {
				std::invoke(callable, std::forward<Args>(args)...);
			} else {
				return std::invoke(callable, std::forward<Args>(args)...);
			}
		}
		constexpr interface * clone() const override {
			return new implementation(callable);
		}
		// explicitly defined so deleting through the base pointer during
		// constant evaluation works in GCC, which rejects the implicitly
		// defaulted constexpr virtual destructor of a class template as
		// "used before its definition"
		constexpr ~implementation() override = default;
	};

	template <typename F> static constexpr bool is_viable_callable =
		!std::is_same_v<std::decay_t<F>, function>
		&& !std::is_same_v<std::decay_t<F>, std::nullptr_t>
		&& std::is_invocable_r_v<R, std::decay_t<F> &, Args...>;

	// a null function pointer or member pointer produces an empty function,
	// matching std::function
	template <typename F> static constexpr interface * make_target(F && f) {
		using D = std::decay_t<F>;
		static_assert(std::is_copy_constructible_v<D>, "ctll::function requires a copy constructible callable (the wrapper itself is copyable)");
		if constexpr (std::is_pointer_v<D> || std::is_member_pointer_v<D>) {
			if (f == nullptr) {
				return nullptr;
			}
		}
		return new implementation<D>(std::forward<F>(f));
	}

	interface * target{nullptr};

public:
	using result_type = R;

	constexpr function() noexcept = default;
	constexpr function(std::nullptr_t) noexcept { }

	// decay turns function references into pointers so they are storable
	template <typename F, typename = std::enable_if_t<is_viable_callable<F>>>
	constexpr function(F && f): target(make_target(std::forward<F>(f))) { }

	constexpr function(const function & other): target(other.target ? other.target->clone() : nullptr) { }

	constexpr function(function && other) noexcept: target(std::exchange(other.target, nullptr)) { }

	constexpr function & operator=(const function & other) {
		if (this != &other) {
			interface * copy = other.target ? other.target->clone() : nullptr;
			delete target;
			target = copy;
		}
		return *this;
	}

	constexpr function & operator=(function && other) noexcept {
		if (this != &other) {
			delete target;
			target = std::exchange(other.target, nullptr);
		}
		return *this;
	}

	constexpr function & operator=(std::nullptr_t) noexcept {
		delete target;
		target = nullptr;
		return *this;
	}

	template <typename F, typename = std::enable_if_t<is_viable_callable<F>>>
	constexpr function & operator=(F && f) {
		function(std::forward<F>(f)).swap(*this);
		return *this;
	}

	constexpr ~function() {
		delete target;
	}

	// calling an empty function is undefined behaviour (in a constant
	// expression it fails to compile, at runtime it dereferences null)
	constexpr R operator()(Args... args) const {
		return target->invoke(std::forward<Args>(args)...);
	}

	constexpr explicit operator bool() const noexcept {
		return target != nullptr;
	}

	constexpr void swap(function & other) noexcept {
		target = std::exchange(other.target, target);
	}

	friend constexpr void swap(function & lhs, function & rhs) noexcept {
		lhs.swap(rhs);
	}

	friend constexpr bool operator==(const function & f, std::nullptr_t) noexcept {
		return f.target == nullptr;
	}
};

// deduction from a plain function pointer
CTLL_EXPORT template <typename R, typename... Args> function(R (*)(Args...)) -> function<R(Args...)>;

// deduction from anything with a non-overloaded operator() (lambdas);
// mirrors std::function's deduction guides (cv, ref and noexcept
// qualified variants)
template <typename> struct callable_signature { };
template <typename R, typename G, typename... Args> struct callable_signature<R (G::*)(Args...)> { using type = R(Args...); };
template <typename R, typename G, typename... Args> struct callable_signature<R (G::*)(Args...) const> { using type = R(Args...); };
template <typename R, typename G, typename... Args> struct callable_signature<R (G::*)(Args...) &> { using type = R(Args...); };
template <typename R, typename G, typename... Args> struct callable_signature<R (G::*)(Args...) const &> { using type = R(Args...); };
template <typename R, typename G, typename... Args> struct callable_signature<R (G::*)(Args...) noexcept> { using type = R(Args...); };
template <typename R, typename G, typename... Args> struct callable_signature<R (G::*)(Args...) const noexcept> { using type = R(Args...); };
template <typename R, typename G, typename... Args> struct callable_signature<R (G::*)(Args...) & noexcept> { using type = R(Args...); };
template <typename R, typename G, typename... Args> struct callable_signature<R (G::*)(Args...) const & noexcept> { using type = R(Args...); };

CTLL_EXPORT template <typename F> function(F) -> function<typename callable_signature<decltype(&F::operator())>::type>;

}

#endif
