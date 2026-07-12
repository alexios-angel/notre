#ifndef NOTRE__ACTIONS__CHARACTERS__HPP
#define NOTRE__ACTIONS__CHARACTERS__HPP

// push character
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::push_character, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(character<V>(), subject.stack), subject.parameters};
}
// push_any_character
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::push_character_anything, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(any(), subject.stack), subject.parameters};
}
// character_alarm
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::push_character_alarm, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(character<'\x07'>(), subject.stack), subject.parameters};
}
// character_escape
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::push_character_escape, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(character<'\x14'>(), subject.stack), subject.parameters};
}
// character_formfeed
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::push_character_formfeed, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(character<'\x0C'>(), subject.stack), subject.parameters};
}
// push_character_newline
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::push_character_newline, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(character<'\x0A'>(), subject.stack), subject.parameters};
}
// push_character_return_carriage
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::push_character_return_carriage, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(character<'\x0D'>(), subject.stack), subject.parameters};
}
// push_character_tab
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::push_character_tab, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(character<'\x09'>(), subject.stack), subject.parameters};
}

// make_control_character (\cX): uppercase X, then flip bit 0x40
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::make_control_character, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	constexpr auto uppercased = (V >= 'a' && V <= 'z') ? static_cast<decltype(V)>(V - 'a' + 'A') : V;
	// the result is always ASCII (<= 0x7F), same as finish_hexdec's small case
	return pcre_context{ctll::push_front(character<static_cast<char>(uppercased ^ 0x40)>(), subject.stack), subject.parameters};
}

#endif
