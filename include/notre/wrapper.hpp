#ifndef NOTRE__WRAPPER__HPP
#define NOTRE__WRAPPER__HPP

#include "evaluation.hpp"
#include "range.hpp"
#include "resolve_subroutines.hpp"
#include "return_type.hpp"
#include "utf8.hpp"
#include "utility.hpp"
#ifndef NOTRE_IN_A_MODULE
#include <string_view>
#endif

namespace notre {

NOTRE_EXPORT template <typename RE, typename Method = void, typename Modifier = singleline> struct regular_expression;

struct zero_terminated_string_end_iterator {
	// this is here only because I want to support std::make_reverse_iterator
	using self_type = zero_terminated_string_end_iterator;
	using value_type = char;
	using reference = char &;
	using pointer = const char *;
	using iterator_category = std::bidirectional_iterator_tag;
	using difference_type = int;

	// it's just sentinel it won't be ever called
	auto operator++() noexcept -> self_type &;
	auto operator++(int) noexcept -> self_type;
	auto operator--() noexcept -> self_type &;
	auto operator--(int) noexcept -> self_type;
	friend auto operator==(self_type, self_type) noexcept -> bool;
	auto operator*() noexcept -> reference;

	constexpr NOTRE_FORCE_INLINE friend bool operator==(const char * ptr, zero_terminated_string_end_iterator) noexcept {
		return *ptr == '\0';
	}
	constexpr NOTRE_FORCE_INLINE friend bool operator==(const wchar_t * ptr, zero_terminated_string_end_iterator) noexcept {
		return *ptr == 0;
	}
	constexpr NOTRE_FORCE_INLINE friend bool operator!=(const char * ptr, zero_terminated_string_end_iterator) noexcept {
		return *ptr != '\0';
	}
	constexpr NOTRE_FORCE_INLINE friend bool operator!=(const wchar_t * ptr, zero_terminated_string_end_iterator) noexcept {
		return *ptr != 0;
	}
	constexpr NOTRE_FORCE_INLINE friend bool operator==(zero_terminated_string_end_iterator, const char * ptr) noexcept {
		return *ptr == '\0';
	}
	constexpr NOTRE_FORCE_INLINE friend bool operator==(zero_terminated_string_end_iterator, const wchar_t * ptr) noexcept {
		return *ptr == 0;
	}
	constexpr NOTRE_FORCE_INLINE friend bool operator!=(zero_terminated_string_end_iterator, const char * ptr) noexcept {
		return *ptr != '\0';
	}
	constexpr NOTRE_FORCE_INLINE friend bool operator!=(zero_terminated_string_end_iterator, const wchar_t * ptr) noexcept {
		return *ptr != 0;
	}
};

template <typename T> class RangeLikeType {
	template <typename Y> static auto test(Y *) -> decltype(std::declval<const Y &>().begin(), std::declval<const Y &>().end(), std::true_type());
	template <typename> static auto test(...) -> std::false_type;

public:
	static constexpr bool value = decltype(test<std::remove_reference_t<std::remove_const_t<T>>>(nullptr))::value;
};

struct match_method {
	template <typename Modifier = singleline, typename ResultIterator = void, typename RE, typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec(IteratorBegin orig_begin, IteratorBegin begin, IteratorEnd end, RE) noexcept {
		using result_iterator = std::conditional_t<std::is_same_v<ResultIterator, void>, IteratorBegin, ResultIterator>;

		return evaluate(orig_begin, begin, end, Modifier{}, return_type<result_iterator, RE>{}, ctll::list<start_mark, RE, assert_subject_end, end_mark, accept>());
	}

	template <typename Modifier = singleline, typename ResultIterator = void, typename RE, typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec(IteratorBegin begin, IteratorEnd end, RE) noexcept {
		return exec<Modifier, ResultIterator>(begin, begin, end, RE{});
	}
};

struct search_method {
	template <typename Modifier = singleline, typename ResultIterator = void, typename RE, typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec(IteratorBegin orig_begin, IteratorBegin begin, IteratorEnd end, RE) noexcept {
		using result_iterator = std::conditional_t<std::is_same_v<ResultIterator, void>, IteratorBegin, ResultIterator>;

		constexpr bool fixed = starts_with_anchor(Modifier{}, ctll::list<RE>{});

		auto it = begin;

		for (; end != it && !fixed; ++it) {
			if (auto out = evaluate(orig_begin, it, end, Modifier{}, return_type<result_iterator, RE>{}, ctll::list<start_mark, RE, end_mark, accept>())) {
				return out;
			}
		}

		// in case the RE is empty or fixed
		auto out = evaluate(orig_begin, it, end, Modifier{}, return_type<result_iterator, RE>{}, ctll::list<start_mark, RE, end_mark, accept>());

		// ALERT: ugly hack
		// propagate end even if it didn't match (this is needed for split function)
		if (!out) out.set_end_mark(it);
		return out;
	}

	template <typename Modifier = singleline, typename ResultIterator = void, typename RE, typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec(IteratorBegin begin, IteratorEnd end, RE) noexcept {
		return exec<Modifier, ResultIterator>(begin, begin, end, RE{});
	}
};

struct starts_with_method {
	template <typename Modifier = singleline, typename ResultIterator = void, typename RE, typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec(IteratorBegin orig_begin, IteratorBegin begin, IteratorEnd end, RE) noexcept {
		using result_iterator = std::conditional_t<std::is_same_v<ResultIterator, void>, IteratorBegin, ResultIterator>;
		return evaluate(orig_begin, begin, end, Modifier{}, return_type<result_iterator, RE>{}, ctll::list<start_mark, RE, end_mark, accept>());
	}

	template <typename Modifier = singleline, typename ResultIterator = void, typename RE, typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec(IteratorBegin begin, IteratorEnd end, RE) noexcept {
		return exec<Modifier, ResultIterator>(begin, begin, end, RE{});
	}
};

// wrapper which calls search on input
struct range_method {
	template <typename Modifier = singleline, typename ResultIterator = void, typename RE, typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec(IteratorBegin begin, IteratorEnd end, RE) noexcept {
		using result_iterator = std::conditional_t<std::is_same_v<ResultIterator, void>, IteratorBegin, ResultIterator>;
		using wrapped_regex = regular_expression<RE, search_method, Modifier>;

		return regex_range<IteratorBegin, IteratorEnd, wrapped_regex, result_iterator>(begin, end);
	}
};

struct tokenize_method {
	template <typename Modifier = singleline, typename ResultIterator = void, typename RE, typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec(IteratorBegin begin, IteratorEnd end, RE) noexcept {
		using result_iterator = std::conditional_t<std::is_same_v<ResultIterator, void>, IteratorBegin, ResultIterator>;
		using wrapped_regex = regular_expression<RE, starts_with_method, Modifier>;

		return regex_range<IteratorBegin, IteratorEnd, wrapped_regex, result_iterator>(begin, end);
	}
};

struct split_method {
	template <typename Modifier = singleline, typename ResultIterator = void, typename RE, typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec(IteratorBegin begin, IteratorEnd end, RE) noexcept {
		using result_iterator = std::conditional_t<std::is_same_v<ResultIterator, void>, IteratorBegin, ResultIterator>;
		using wrapped_regex = regular_expression<RE, search_method, Modifier>;

		return regex_split_range<IteratorBegin, IteratorEnd, wrapped_regex, result_iterator>(begin, end);
	}
};

struct iterator_method {
	template <typename Modifier = singleline, typename ResultIterator = void, typename RE, typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec(IteratorBegin begin, IteratorEnd end, RE) noexcept {
		using result_iterator = std::conditional_t<std::is_same_v<ResultIterator, void>, IteratorBegin, ResultIterator>;
		using wrapped_regex = regular_expression<RE, search_method, Modifier>;

		return regex_iterator<IteratorBegin, IteratorEnd, wrapped_regex, result_iterator>(begin, end);
	}
	constexpr NOTRE_FORCE_INLINE static auto exec() noexcept {
		return regex_end_iterator{};
	}
};

NOTRE_EXPORT template <typename RE, typename Method, typename Modifier> struct regular_expression {
	constexpr NOTRE_FORCE_INLINE regular_expression() noexcept { }
	constexpr NOTRE_FORCE_INLINE regular_expression(RE) noexcept { }

	template <typename ResultIterator, typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec_with_result_iterator(IteratorBegin orig_begin, IteratorBegin begin, IteratorEnd end) noexcept {
		return Method::template exec<Modifier, ResultIterator>(orig_begin, begin, end, RE{});
	}
	template <typename ResultIterator, typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec_with_result_iterator(IteratorBegin begin, IteratorEnd end) noexcept {
		return Method::template exec<Modifier, ResultIterator>(begin, end, RE{});
	}
	template <typename Range> constexpr NOTRE_FORCE_INLINE static auto multi_exec(Range && range) noexcept {
		return multi_subject_range<Range, regular_expression>{std::forward<Range>(range)};
	}
	constexpr NOTRE_FORCE_INLINE static auto exec() noexcept {
		return Method::exec();
	}
	template <typename IteratorBegin, typename IteratorEnd> constexpr NOTRE_FORCE_INLINE static auto exec(IteratorBegin begin, IteratorEnd end) noexcept {
		return Method::template exec<Modifier>(begin, end, RE{});
	}
	static constexpr NOTRE_FORCE_INLINE auto exec(const char * s) noexcept {
		return Method::template exec<Modifier>(s, zero_terminated_string_end_iterator(), RE{});
	}
	static constexpr NOTRE_FORCE_INLINE auto exec(const wchar_t * s) noexcept {
		return Method::template exec<Modifier>(s, zero_terminated_string_end_iterator(), RE{});
	}
	static constexpr NOTRE_FORCE_INLINE auto exec(std::string_view sv) noexcept {
		return exec(sv.begin(), sv.end());
	}
	static constexpr NOTRE_FORCE_INLINE auto exec(std::wstring_view sv) noexcept {
		return exec(sv.begin(), sv.end());
	}
#ifdef NOTRE_ENABLE_UTF8_RANGE
	static constexpr NOTRE_FORCE_INLINE auto exec(std::u8string_view sv) noexcept {
		return exec_with_result_iterator<const char8_t *>(utf8_range(sv).begin(), utf8_range(sv).end());
	}
#endif
	static constexpr NOTRE_FORCE_INLINE auto exec(std::u16string_view sv) noexcept {
		return exec(sv.begin(), sv.end());
	}
	static constexpr NOTRE_FORCE_INLINE auto exec(std::u32string_view sv) noexcept {
		return exec(sv.begin(), sv.end());
	}
	template <typename Range, typename = typename std::enable_if<RangeLikeType<Range>::value>::type> static constexpr NOTRE_FORCE_INLINE auto exec(Range && range) noexcept {
		return exec(std::begin(range), std::end(range));
	}

	// another api
	template <typename... Args> NOTRE_FORCE_INLINE constexpr auto operator()(Args &&... args) const noexcept {
		return exec(std::forward<Args>(args)...);
	}
	// api for pattern matching
	template <typename... Args> NOTRE_FORCE_INLINE constexpr auto try_extract(Args &&... args) const noexcept {
		return exec(std::forward<Args>(args)...);
	}

	// for compatibility with _notre literal
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto match(Args &&... args) noexcept {
		return regular_expression<RE, match_method, singleline>::exec(std::forward<Args>(args)...);
	}
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto search(Args &&... args) noexcept {
		return regular_expression<RE, search_method, singleline>::exec(std::forward<Args>(args)...);
	}
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto starts_with(Args &&... args) noexcept {
		return regular_expression<RE, starts_with_method, singleline>::exec(std::forward<Args>(args)...);
	}
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto range(Args &&... args) noexcept {
		return regular_expression<RE, range_method, singleline>::exec(std::forward<Args>(args)...);
	}
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto split(Args &&... args) noexcept {
		return regular_expression<RE, split_method, singleline>::exec(std::forward<Args>(args)...);
	}
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto tokenize(Args &&... args) noexcept {
		return regular_expression<RE, tokenize_method, singleline>::exec(std::forward<Args>(args)...);
	}
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto iterator(Args &&... args) noexcept {
		return regular_expression<RE, iterator_method, singleline>::exec(std::forward<Args>(args)...);
	}

	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto multiline_match(Args &&... args) noexcept {
		return regular_expression<RE, match_method, multiline>::exec(std::forward<Args>(args)...);
	}
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto multiline_search(Args &&... args) noexcept {
		return regular_expression<RE, search_method, multiline>::exec(std::forward<Args>(args)...);
	}
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto multiline_starts_with(Args &&... args) noexcept {
		return regular_expression<RE, starts_with_method, multiline>::exec(std::forward<Args>(args)...);
	}
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto multiline_range(Args &&... args) noexcept {
		return regular_expression<RE, range_method, multiline>::exec(std::forward<Args>(args)...);
	}
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto multiline_split(Args &&... args) noexcept {
		return regular_expression<RE, split_method, multiline>::exec(std::forward<Args>(args)...);
	}
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto multiline_tokenize(Args &&... args) noexcept {
		return regular_expression<RE, tokenize_method, multiline>::exec(std::forward<Args>(args)...);
	}
	template <typename... Args> static constexpr NOTRE_FORCE_INLINE auto multiline_iterator(Args &&... args) noexcept {
		return regular_expression<RE, iterator_method, multiline>::exec(std::forward<Args>(args)...);
	}
};

// range style API support for tokenize/range/split operations
template <typename Range, typename RE, typename Modifier> constexpr auto operator|(Range && range, regular_expression<RE, range_method, Modifier> re) noexcept {
	return re.exec(std::forward<Range>(range));
}

template <typename Range, typename RE, typename Modifier> constexpr auto operator|(Range && range, regular_expression<RE, tokenize_method, Modifier> re) noexcept {
	return re.exec(std::forward<Range>(range));
}

template <typename Range, typename RE, typename Modifier> constexpr auto operator|(Range && range, regular_expression<RE, split_method, Modifier> re) noexcept {
	return re.exec(std::forward<Range>(range));
}

template <typename Range, typename RE, typename Modifier> constexpr auto operator|(Range && range, regular_expression<RE, iterator_method, Modifier> re) noexcept = delete;

template <typename Range, typename RE, typename Method, typename Modifier> constexpr auto operator|(Range && range, regular_expression<RE, Method, Modifier> re) noexcept {
	return re.multi_exec(std::forward<Range>(range));
}

// error reporting of problematic position in a regex
template <size_t> struct problem_at_position; // do not define!

template <> struct problem_at_position<~static_cast<size_t>(0)> {
	constexpr operator bool() const noexcept {
		return true;
	}
};

#ifdef NOTRE_VERBOSE_ERRORS
// with NOTRE_VERBOSE_ERRORS the position, line and column of a syntax
// error appear as template arguments in the compiler's backtrace. Kept
// self-contained here (no diagnostics.hpp dependency) so the core
// include graph is unchanged; see diagnostics.hpp for the full caret
// rendering via error_message<>().
template <size_t Position, size_t Line, size_t Column> struct verbose_regex_error {
	static_assert(Position == ~static_cast<size_t>(0),
	              "notre: the regular expression has a syntax error - its position, line and column "
	              "are the template arguments of verbose_regex_error<...> in this diagnostic");
	static constexpr bool instantiated = true;
};
template <typename FS> constexpr size_t verbose_line_of(const FS & s, size_t off) noexcept {
	size_t line = 1;
	for (size_t i = 0; i < off && i < s.size(); ++i) {
		if (s[i] == static_cast<decltype(s[i])>('\n')) { ++line; }
	}
	return line;
}
template <typename FS> constexpr size_t verbose_column_of(const FS & s, size_t off) noexcept {
	size_t column = 1;
	for (size_t i = 0; i < off && i < s.size(); ++i) {
		if (s[i] == static_cast<decltype(s[i])>('\n')) {
			column = 1;
		} else {
			++column;
		}
	}
	return column;
}
#endif

#if NOTRE_CNTTP_COMPILER_CHECK
#define NOTRE_REGEX_INPUT_TYPE ctll::fixed_string
#define NOTRE_REGEX_TEMPLATE_COPY_TYPE auto
#else
#define NOTRE_REGEX_INPUT_TYPE const auto &
#define NOTRE_REGEX_TEMPLATE_COPY_TYPE const auto &
#endif

template <NOTRE_REGEX_TEMPLATE_COPY_TYPE input> struct regex_builder {
	static constexpr auto _input = input;
	using result = typename ctll::parser<notre::pcre, _input, notre::pcre_actions>::template output<pcre_context<>>;

	static constexpr auto n = result::is_correct ? ~static_cast<size_t>(0) : result::position;

	// A plain bool, so the message-bearing static_assert below prints just
	// its message instead of dumping the whole parser result type.
	static constexpr bool syntax_ok = result::is_correct;

	// The readable message goes on its OWN static_assert so the compiler
	// actually prints it. The problem_at_position<N> trigger below is an
	// incomplete type on failure, so a static_assert whose *condition*
	// mentions it is reported as "incomplete type" and its message is
	// swallowed - which is why this one only tests syntax_ok.
	static_assert(syntax_ok,
	              "notre: the regular expression has a syntax error - the failing offset is the N "
	              "in problem_at_position<N> below; call notre::error_message<pattern>() for a caret "
	              "at that spot, or define NOTRE_VERBOSE_ERRORS to see the line and column here");

	// the classic position-bearing trigger: names the failing offset N in
	// the diagnostic (problem_at_position<N> is left undefined on purpose)
	static_assert(syntax_ok || problem_at_position<n>{});

#ifdef NOTRE_VERBOSE_ERRORS
	static_assert(result::is_correct
	              || verbose_regex_error<n, verbose_line_of(_input, n), verbose_column_of(_input, n)>::instantiated);
#endif

	using parsed_type = ctll::conditional<result::is_correct, decltype(ctll::front(typename result::output_type::stack_type())), ctll::list<reject>>;

	// subroutine calls are inlined here, after the whole pattern is known
	using type = decltype(notre::resolve_subroutines(parsed_type{}));
};

// a with_callouts<Handler> modifier is bound into the pattern's callout
// atoms here, after parsing and subroutine resolution (callouts.hpp)
template <typename RE, typename... Modifiers> using with_callouts_bound = typename callout_binding<Modifiers...>::template bound<RE>;

// case-sensitive

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto match = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, match_method, ctll::list<singleline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto search = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, search_method, ctll::list<singleline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto starts_with = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, starts_with_method, ctll::list<singleline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto search_all = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, range_method, ctll::list<singleline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> [[deprecated("use search_all")]] constexpr auto range = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, range_method, ctll::list<singleline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto split = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, split_method, ctll::list<singleline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto tokenize = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, tokenize_method, ctll::list<singleline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto iterator = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, iterator_method, ctll::list<singleline, Modifiers...>>();

NOTRE_EXPORT constexpr auto sentinel = regex_end_iterator();

// multiline

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto multiline_match = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, match_method, ctll::list<multiline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto multiline_search = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, search_method, ctll::list<multiline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto multiline_starts_with = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, starts_with_method, ctll::list<multiline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto multiline_search_all = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, range_method, ctll::list<multiline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> [[deprecated("use multiline_search_all")]] constexpr auto multiline_range = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, range_method, ctll::list<multiline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto multiline_split = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, split_method, ctll::list<multiline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto multiline_tokenize = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, tokenize_method, ctll::list<multiline, Modifiers...>>();

NOTRE_EXPORT template <NOTRE_REGEX_INPUT_TYPE input, typename... Modifiers> constexpr auto multiline_iterator = regular_expression<with_callouts_bound<typename regex_builder<input>::type, Modifiers...>, iterator_method, ctll::list<multiline, Modifiers...>>();

NOTRE_EXPORT constexpr auto multiline_sentinel = regex_end_iterator();

} // namespace notre


#endif
