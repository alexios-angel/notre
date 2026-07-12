#ifndef NOTRE__ACTIONS__CALLOUT__HPP
#define NOTRE__ACTIONS__CALLOUT__HPP

// callouts (?C), (?Cn), (?C"name"), (?C'name') put a zero-width marker
// atom into the AST; the capture id prepared for the opening parenthesis
// is dropped since a callout is not a group

// (?C)
template <auto V, typename... Ts, size_t Id, size_t Counter> static constexpr auto apply(pcre::make_callout, ctll::term<V>, pcre_context<ctll::list<capture_id<Id>, Ts...>, pcre_parameters<Counter>>) {
	return pcre_context<ctll::list<callout_numbered<0>, Ts...>, pcre_parameters<Counter-1>>{};
}

// (?Cn)
template <auto V, size_t Number, typename... Ts, size_t Id, size_t Counter> static constexpr auto apply(pcre::make_callout_numbered, ctll::term<V>, pcre_context<ctll::list<number<Number>, capture_id<Id>, Ts...>, pcre_parameters<Counter>>) {
	return pcre_context<ctll::list<callout_numbered<Number>, Ts...>, pcre_parameters<Counter-1>>{};
}

// (?C"name") and (?C'name')
template <auto... Str, auto V, typename... Ts, size_t Id, size_t Counter> static constexpr auto apply(pcre::make_callout_named, ctll::term<V>, pcre_context<ctll::list<id<Str...>, capture_id<Id>, Ts...>, pcre_parameters<Counter>>) {
	return pcre_context<ctll::list<callout_named<id<Str...>>, Ts...>, pcre_parameters<Counter-1>>{};
}

#endif
