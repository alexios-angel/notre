#ifndef CTRE__ACTIONS__COMMENT__HPP
#define CTRE__ACTIONS__COMMENT__HPP

// comment (?#...) contributes nothing to the pattern: drop the capture id
// prepared for the opening parenthesis and leave an empty atom, which
// make_sequence erases
template <auto V, typename... Ts, size_t Id, size_t Counter> static constexpr auto apply(pcre::make_comment, ctll::term<V>, pcre_context<ctll::list<capture_id<Id>, Ts...>, pcre_parameters<Counter>>) {
	return pcre_context<ctll::list<empty, Ts...>, pcre_parameters<Counter-1>>{};
}

#endif
