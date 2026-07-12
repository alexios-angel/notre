#ifndef NOTRE__ACTIONS__NAMED_CLASS__HPP
#define NOTRE__ACTIONS__NAMED_CLASS__HPP

// class_named_alnum
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_alnum, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(notre::alphanum_chars(), subject.stack), subject.parameters};
}
// class_named_alpha
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_alpha, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(notre::alpha_chars(), subject.stack), subject.parameters};
}
// class_named_digit
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_digit, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(notre::digit_chars(), subject.stack), subject.parameters};
}
// class_named_ascii
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_ascii, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(notre::ascii_chars(), subject.stack), subject.parameters};
}
// class_named_blank
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_blank, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(notre::enumeration<' ','\t'>(), subject.stack), subject.parameters};
}
// class_named_cntrl
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_cntrl, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(notre::set<notre::char_range<'\x00','\x1F'>, notre::character<'\x7F'>>(), subject.stack), subject.parameters};
}
// class_named_graph
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_graph, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(notre::char_range<'\x21','\x7E'>(), subject.stack), subject.parameters};
}
// class_named_lower
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_lower, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(notre::char_range<'a','z'>(), subject.stack), subject.parameters};
}
// class_named_upper
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_upper, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(notre::char_range<'A','Z'>(), subject.stack), subject.parameters};
}
// class_named_print
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_print, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(notre::char_range<'\x20','\x7E'>(), subject.stack), subject.parameters};
}
// class_named_space
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_space, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(space_chars(), subject.stack), subject.parameters};
}
// class_named_word
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_word, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(word_chars(), subject.stack), subject.parameters};
}
// class_named_punct
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_punct, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(punct_chars(), subject.stack), subject.parameters};
}
// class_named_xdigit
template <auto V, typename... Ts, typename Parameters> static constexpr auto apply(pcre::class_named_xdigit, ctll::term<V>, pcre_context<ctll::list<Ts...>, Parameters> subject) {
	return pcre_context{ctll::push_front(xdigit_chars(), subject.stack), subject.parameters};
}

#endif
