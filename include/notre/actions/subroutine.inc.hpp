#ifndef NOTRE__ACTIONS__SUBROUTINE__HPP
#define NOTRE__ACTIONS__SUBROUTINE__HPP

// subroutine calls put a placeholder into the AST; the placeholder is
// replaced by an inlined copy of the referenced group after parsing
// (see resolve_subroutines.hpp)

// (?N) — the capture id prepared for the opening parenthesis is dropped
template <auto V, size_t Id, typename... Ts, size_t PrepId, size_t Counter> static constexpr auto apply(pcre::make_subroutine_call, ctll::term<V>, pcre_context<ctll::list<number<Id>, capture_id<PrepId>, Ts...>, pcre_parameters<Counter>>) {
	if constexpr (Id == 0) {
		// (?0) is recursion of the whole pattern, which cannot be supported
		return ctll::reject{};
	} else {
		return pcre_context<ctll::list<subroutine_call<Id>, Ts...>, pcre_parameters<Counter-1>>{};
	}
}

// \g<N> and \g'N' — plain backslash context, no capture id to drop
template <auto V, size_t Id, typename... Ts, size_t Counter> static constexpr auto apply(pcre::make_subroutine_call, ctll::term<V>, pcre_context<ctll::list<number<Id>, Ts...>, pcre_parameters<Counter>>) {
	if constexpr (Id == 0) {
		return ctll::reject{};
	} else {
		return pcre_context<ctll::list<subroutine_call<Id>, Ts...>, pcre_parameters<Counter>>{};
	}
}

// (?-N) — the most recently defined group is (?-1); Counter includes the
// call's own prepared id, so the target is Counter - N
template <auto V, size_t Id, typename... Ts, size_t PrepId, size_t Counter> static constexpr auto apply(pcre::make_subroutine_call_behind, ctll::term<V>, pcre_context<ctll::list<number<Id>, capture_id<PrepId>, Ts...>, pcre_parameters<Counter>>) {
	if constexpr (Id == 0 || Counter <= Id) {
		return ctll::reject{};
	} else {
		return pcre_context<ctll::list<subroutine_call<Counter - Id>, Ts...>, pcre_parameters<Counter-1>>{};
	}
}

// (?+N) — the next group to be defined is (?+1); after dropping the call's
// own prepared id the target is (Counter - 1) + N
template <auto V, size_t Id, typename... Ts, size_t PrepId, size_t Counter> static constexpr auto apply(pcre::make_subroutine_call_ahead, ctll::term<V>, pcre_context<ctll::list<number<Id>, capture_id<PrepId>, Ts...>, pcre_parameters<Counter>>) {
	if constexpr (Id == 0) {
		return ctll::reject{};
	} else {
		return pcre_context<ctll::list<subroutine_call<Counter - 1 + Id>, Ts...>, pcre_parameters<Counter-1>>{};
	}
}

// (?&name) and (?P>name)
template <auto... Str, auto V, typename... Ts, size_t PrepId, size_t Counter> static constexpr auto apply(pcre::make_subroutine_call_with_name, ctll::term<V>, pcre_context<ctll::list<id<Str...>, capture_id<PrepId>, Ts...>, pcre_parameters<Counter>>) {
	return pcre_context<ctll::list<subroutine_call_with_name<id<Str...>>, Ts...>, pcre_parameters<Counter-1>>{};
}

// \g<name> and \g'name'
template <auto... Str, auto V, typename... Ts, size_t Counter> static constexpr auto apply(pcre::make_subroutine_call_with_name, ctll::term<V>, pcre_context<ctll::list<id<Str...>, Ts...>, pcre_parameters<Counter>>) {
	return pcre_context<ctll::list<subroutine_call_with_name<id<Str...>>, Ts...>, pcre_parameters<Counter>>{};
}

#endif
