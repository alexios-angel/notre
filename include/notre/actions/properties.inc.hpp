#ifndef NOTRE__ACTIONS__PROPERTIES__HPP
#define NOTRE__ACTIONS__PROPERTIES__HPP

// push_property_name
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::push_property_name, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(property_name<V>(), subject.stack), subject.parameters};
}
// push_property_name (concat)
template <auto... Str, auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::push_property_name, ctll::term<V>, pcre_context<ctll::list<property_name<Str...>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(property_name<Str..., V>(), ctll::list<Ts...>()), subject.parameters};
}

// push_property_value
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::push_property_value, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(property_value<V>(), subject.stack), subject.parameters};
}
// push_property_value (concat)
template <auto... Str, auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::push_property_value, ctll::term<V>, pcre_context<ctll::list<property_value<Str...>, Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(property_value<Str..., V>(), ctll::list<Ts...>()), subject.parameters};
}

// make_property
template <auto V, auto... Name, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_property, ctll::term<V>, [[maybe_unused]] pcre_context<ctll::list<property_name<Name...>, Ts...>, Parameters> subject) {
	//return ctll::reject{};
	constexpr char name[sizeof...(Name)]{static_cast<char>(Name)...};
	constexpr auto p = uni::detail::binary_prop_from_string(get_string_view(name));

	if constexpr (uni::detail::is_unknown(p)) {
		return ctll::reject{};
	} else {
		return pcre_context{ctll::push_front(make_binary_property<p>(), ctll::list<Ts...>()), subject.parameters};
	}
}

// make_property
template <auto V, auto... Value, auto... Name, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_property, ctll::term<V>, [[maybe_unused]] pcre_context<ctll::list<property_value<Value...>, property_name<Name...>, Ts...>, Parameters> subject) {
	//return ctll::reject{};
	constexpr auto prop = property_builder<Name...>::template get<Value...>();

	if constexpr (std::is_same_v<decltype(prop), ctll::reject>) {
		return ctll::reject{};
	} else {
		return pcre_context{ctll::push_front(prop, ctll::list<Ts...>()), subject.parameters};
	}
}


// make_property_negative
template <auto V, auto... Name, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_property_negative, ctll::term<V>, [[maybe_unused]] pcre_context<ctll::list<property_name<Name...>, Ts...>, Parameters> subject) {
	//return ctll::reject{};
	constexpr char name[sizeof...(Name)]{static_cast<char>(Name)...};
	constexpr auto p = uni::detail::binary_prop_from_string(get_string_view(name));

	if constexpr (uni::detail::is_unknown(p)) {
		return ctll::reject{};
	} else {
		return pcre_context{ctll::push_front(negate<make_binary_property<p>>(), ctll::list<Ts...>()), subject.parameters};
	}
}

// make_property_negative
template <auto V, auto... Value, auto... Name, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_property_negative, ctll::term<V>, [[maybe_unused]] pcre_context<ctll::list<property_value<Value...>, property_name<Name...>, Ts...>, Parameters> subject) {
	//return ctll::reject{};
	constexpr auto prop = property_builder<Name...>::template get<Value...>();

	if constexpr (std::is_same_v<decltype(prop), ctll::reject>) {
		return ctll::reject{};
	} else {
		return pcre_context{ctll::push_front(negate<decltype(prop)>(), ctll::list<Ts...>()), subject.parameters};
	}
}

// make_grapheme_cluster (\X): approximated as an atomic \P{M}\p{M}* -
// one non-mark code point followed by all of its combining marks
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_grapheme_cluster, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	// derive 'M' from V (always 'X') so the lookup stays dependent and is
	// only evaluated at instantiation, when the unicode db definitions exist
	constexpr char name[1]{static_cast<char>(V - 'X' + 'M')};
	constexpr auto p = uni::detail::binary_prop_from_string(get_string_view(name));
	static_assert(!uni::detail::is_unknown(p), "unicode database does not know the Mark (M) category");
	using mark = make_binary_property<p>;
	return pcre_context{ctll::push_front(atomic_group<negate<mark>, star<mark>>(), subject.stack), subject.parameters};
}

#endif
