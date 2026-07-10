#ifndef CTRE__ACTIONS__CONDITIONAL__HPP
#define CTRE__ACTIONS__CONDITIONAL__HPP

// conditional patterns (?(condition)yes|no)
//
// A group-reference condition parses into a condition_ref/id marker which
// sits under the branches until make_conditional_one/_two builds the
// condition_capture node. An assertion condition is rewritten exactly:
//   (?(?=A)Y|N)  ==  (?:(?=A)Y|(?!A)N)
// DEFINE turns its (single) branch into a define_group, which is never
// evaluated; the recursion condition (?(R)...) is not supported.

// (?(N) — like a backreference, the group must already be defined
template <auto V, size_t Id, typename... Ts, size_t Counter> static constexpr auto apply(pcre::make_condition_capture_ref, ctll::term<V>, pcre_context<ctll::list<number<Id>, Ts...>, pcre_parameters<Counter>>) {
	if constexpr (Id == 0 || Counter < Id) {
		return ctll::reject{};
	} else {
		return pcre_context<ctll::list<condition_ref<Id>, Ts...>, pcre_parameters<Counter>>{};
	}
}

// (?(-N) — the most recently defined group is (?(-1)
template <auto V, size_t Id, typename... Ts, size_t Counter> static constexpr auto apply(pcre::make_condition_capture_ref_behind, ctll::term<V>, pcre_context<ctll::list<number<Id>, Ts...>, pcre_parameters<Counter>>) {
	if constexpr (Id == 0 || Counter < Id) {
		return ctll::reject{};
	} else {
		return pcre_context<ctll::list<condition_ref<(Counter + 1) - Id>, Ts...>, pcre_parameters<Counter>>{};
	}
}

// (?(+N) — the next group to be defined is (?(+1); it cannot have matched
// at this point, but PCRE accepts the syntax and so do we
template <auto V, size_t Id, typename... Ts, size_t Counter> static constexpr auto apply(pcre::make_condition_capture_ref_ahead, ctll::term<V>, pcre_context<ctll::list<number<Id>, Ts...>, pcre_parameters<Counter>>) {
	if constexpr (Id == 0) {
		return ctll::reject{};
	} else {
		return pcre_context<ctll::list<condition_ref<Counter + Id>, Ts...>, pcre_parameters<Counter>>{};
	}
}

// --- single branch: (?(cond)yes)

template <auto V, typename Yes, size_t Id, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_one, ctll::term<V>, pcre_context<ctll::list<Yes, condition_ref<Id>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(condition_capture<Id, Yes, empty>(), ctll::list<Ts...>()), subject.parameters};
}

template <auto V, typename Yes, auto... Str, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_one, ctll::term<V>, pcre_context<ctll::list<Yes, id<Str...>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(condition_capture_with_name<id<Str...>, Yes, empty>(), ctll::list<Ts...>()), subject.parameters};
}

// (?(DEFINE)...) — the branch becomes a group that is never evaluated
template <auto V, typename Yes, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_one, ctll::term<V>, pcre_context<ctll::list<Yes, id<'D','E','F','I','N','E'>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(define_group<Yes>(), ctll::list<Ts...>()), subject.parameters};
}

// (?(R)...) — recursion condition cannot be supported
template <auto V, typename Yes, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_one, ctll::term<V>, pcre_context<ctll::list<Yes, id<'R'>, Ts...>, Parameters>) {
	return ctll::reject{};
}

template <auto V, typename Yes, typename... Look, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_one, ctll::term<V>, pcre_context<ctll::list<Yes, lookahead_positive<Look...>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(select<sequence<lookahead_positive<Look...>, Yes>, lookahead_negative<Look...>>(), ctll::list<Ts...>()), subject.parameters};
}

template <auto V, typename Yes, typename... Look, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_one, ctll::term<V>, pcre_context<ctll::list<Yes, lookahead_negative<Look...>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(select<sequence<lookahead_negative<Look...>, Yes>, lookahead_positive<Look...>>(), ctll::list<Ts...>()), subject.parameters};
}

template <auto V, typename Yes, typename... Look, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_one, ctll::term<V>, pcre_context<ctll::list<Yes, lookbehind_positive<Look...>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(select<sequence<lookbehind_positive<Look...>, Yes>, lookbehind_negative<Look...>>(), ctll::list<Ts...>()), subject.parameters};
}

template <auto V, typename Yes, typename... Look, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_one, ctll::term<V>, pcre_context<ctll::list<Yes, lookbehind_negative<Look...>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(select<sequence<lookbehind_negative<Look...>, Yes>, lookbehind_positive<Look...>>(), ctll::list<Ts...>()), subject.parameters};
}

// --- two branches: (?(cond)yes|no)

template <auto V, typename No, typename Yes, size_t Id, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_two, ctll::term<V>, pcre_context<ctll::list<No, Yes, condition_ref<Id>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(condition_capture<Id, Yes, No>(), ctll::list<Ts...>()), subject.parameters};
}

template <auto V, typename No, typename Yes, auto... Str, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_two, ctll::term<V>, pcre_context<ctll::list<No, Yes, id<Str...>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(condition_capture_with_name<id<Str...>, Yes, No>(), ctll::list<Ts...>()), subject.parameters};
}

// a DEFINE group must not have an alternative
template <auto V, typename No, typename Yes, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_two, ctll::term<V>, pcre_context<ctll::list<No, Yes, id<'D','E','F','I','N','E'>, Ts...>, Parameters>) {
	return ctll::reject{};
}

template <auto V, typename No, typename Yes, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_two, ctll::term<V>, pcre_context<ctll::list<No, Yes, id<'R'>, Ts...>, Parameters>) {
	return ctll::reject{};
}

template <auto V, typename No, typename Yes, typename... Look, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_two, ctll::term<V>, pcre_context<ctll::list<No, Yes, lookahead_positive<Look...>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(select<sequence<lookahead_positive<Look...>, Yes>, sequence<lookahead_negative<Look...>, No>>(), ctll::list<Ts...>()), subject.parameters};
}

template <auto V, typename No, typename Yes, typename... Look, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_two, ctll::term<V>, pcre_context<ctll::list<No, Yes, lookahead_negative<Look...>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(select<sequence<lookahead_negative<Look...>, Yes>, sequence<lookahead_positive<Look...>, No>>(), ctll::list<Ts...>()), subject.parameters};
}

template <auto V, typename No, typename Yes, typename... Look, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_two, ctll::term<V>, pcre_context<ctll::list<No, Yes, lookbehind_positive<Look...>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(select<sequence<lookbehind_positive<Look...>, Yes>, sequence<lookbehind_negative<Look...>, No>>(), ctll::list<Ts...>()), subject.parameters};
}

template <auto V, typename No, typename Yes, typename... Look, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_conditional_two, ctll::term<V>, pcre_context<ctll::list<No, Yes, lookbehind_negative<Look...>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(select<sequence<lookbehind_negative<Look...>, Yes>, sequence<lookbehind_positive<Look...>, No>>(), ctll::list<Ts...>()), subject.parameters};
}

#endif
