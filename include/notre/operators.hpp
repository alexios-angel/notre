#ifndef NOTRE_V2__NOTRE__OPERATORS__HPP
#define NOTRE_V2__NOTRE__OPERATORS__HPP

template <typename A, typename B> constexpr auto operator|(notre::regular_expression<A>, notre::regular_expression<B>) -> notre::regular_expression<notre::select<A,B>> {
	return {};
}

template <typename A, typename B> constexpr auto operator>>(notre::regular_expression<A>, notre::regular_expression<B>) -> notre::regular_expression<notre::sequence<A,B>> {
	return {};
}

#endif
