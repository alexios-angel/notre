// ctll::function requires C++20 (constexpr virtuals + constexpr allocation);
// in C++17 suite runs this file compiles empty
#if defined(__cpp_constexpr_dynamic_alloc) && __cpp_constexpr_dynamic_alloc >= 201907L

#include <ctll/function.hpp>

int twice(int x) { return 2 * x; }

static_assert(std::is_same_v<ctll::function<int(int)>::result_type, int>);
static_assert(std::is_invocable_r_v<int, ctll::function<int(int)>, int>);

// capturing lambda through the deduction guide, in a constant expression
constexpr auto capture_local() {
	int i = 42;
	ctll::function f = [&] { return i; };
	return f();
}
static_assert(capture_local() == 42);

// explicit signature with arguments
static_assert([] {
	ctll::function<int(int, int)> f = [](int a, int b) { return a + b; };
	return f(40, 2);
}() == 42);

// copies are deep (clone) and independent of the source's lifetime
static_assert([] {
	ctll::function<int()> g;
	{
		ctll::function<int()> f = [] { return 5; };
		g = f;
	} // f destroyed
	return g();
}() == 5);

// move empties the source
static_assert([] {
	ctll::function<int()> f = [] { return 7; };
	ctll::function<int()> g = std::move(f);
	return g() + (f ? 100 : 0);
}() == 7);

// deduction from a function pointer (invocable at runtime only, since
// twice is not constexpr; construction and emptiness are still constant)
static_assert([] {
	ctll::function f = twice;
	return bool(f);
}());

// the wrapper owns mutable lambda state
static_assert([] {
	ctll::function<int()> counter = [n = 0]() mutable { return ++n; };
	counter();
	counter();
	return counter();
}() == 3);

// member swap, free ADL swap, and emptiness
static_assert([] {
	ctll::function<int()> a = [] { return 1; };
	ctll::function<int()> b;
	a.swap(b);
	swap(a, b);
	a.swap(b);
	return (b ? b() : 0) + (a ? 100 : 0);
}() == 1);

// copy assignment through a reference (self-assignment safe)
static_assert([] {
	ctll::function<int()> f = [] { return 1; };
	ctll::function<int()> g = [] { return 2; };
	g = f;
	auto & self = f;
	f = self;
	return f() * 10 + g();
}() == 11);

// direct assignment from a callable
static_assert([] {
	ctll::function<int()> f;
	f = [] { return 9; };
	return f();
}() == 9);

// nullptr: construction, assignment and comparison
static_assert([] {
	ctll::function<int()> f = nullptr;
	bool was_empty = (f == nullptr);
	f = [] { return 1; };
	bool became_engaged = (f != nullptr) && (nullptr != f);
	f = nullptr;
	return was_empty && became_engaged && !f;
}());

// a null function pointer produces an empty function, like std::function
static_assert([] {
	int (*null_fn)(int) = nullptr;
	ctll::function<int(int)> f = null_fn;
	return f == nullptr;
}());

// void return type discards the callable's result (INVOKE<R> semantics)
static_assert([] {
	int calls = 0;
	ctll::function<void(int)> f = [&](int x) { calls += x; return calls; };
	f(40);
	f(2);
	return calls;
}() == 42);

// pointers to member functions and to data members work via std::invoke
struct widget {
	int value;
	constexpr int doubled() const { return 2 * value; }
};
static_assert([] {
	ctll::function<int(const widget &)> m = &widget::doubled;
	ctll::function<int(const widget &)> d = &widget::value;
	constexpr widget w{21};
	return m(w) + d(w);
}() == 63);

// a null member pointer is empty too
static_assert([] {
	int (widget::*null_member)() const = nullptr;
	ctll::function<int(const widget &)> f = null_member;
	return f == nullptr;
}());

// deduction from ref-qualified operator()
struct lvalue_only {
	constexpr int operator()(int x) & { return x + 1; }
};
static_assert([] {
	ctll::function f = lvalue_only{};
	return f(41);
}() == 42);

// move-only arguments pass through the erasure boundary
struct move_only {
	int v;
	constexpr explicit move_only(int v_): v(v_) { }
	move_only(const move_only &) = delete;
	constexpr move_only(move_only &&) = default;
};
static_assert([] {
	ctll::function<int(move_only)> f = [](move_only m) { return m.v; };
	return f(move_only{42});
}() == 42);

#endif

void empty_symbol() { }
