#ifndef NOTRE_V2__NOTRE__FLAGS_AND_MODES__HPP
#define NOTRE_V2__NOTRE__FLAGS_AND_MODES__HPP

namespace notre {

struct singleline { };
struct multiline { };

struct case_sensitive { };
struct case_insensitive { };

using ci = case_insensitive;
using cs = case_sensitive;

// modifier attaching callout implementations to the pattern: either one
// handler type with a static callouts() map, or a pack of inline
// ctll::callout entries; bound into the callout atoms before evaluation
// (callouts.hpp)
template <typename... Handler> struct with_callouts { };

template <typename... Flags> struct flag_list { };

struct flags {
	bool block_empty_match = false;
	bool multiline = false;
	bool case_insensitive = false;
	
	constexpr flags() = default;
	constexpr flags(const flags &) = default;
	constexpr flags(flags &&) = default;
	
	constexpr NOTRE_FORCE_INLINE flags(notre::singleline v) noexcept { set_flag(v); }
	constexpr NOTRE_FORCE_INLINE flags(notre::multiline v) noexcept { set_flag(v); }
	constexpr NOTRE_FORCE_INLINE flags(notre::case_sensitive v) noexcept { set_flag(v); }
	constexpr NOTRE_FORCE_INLINE flags(notre::case_insensitive v) noexcept { set_flag(v); }
	
	
	template <typename... Args> constexpr NOTRE_FORCE_INLINE flags(ctll::list<Args...>) noexcept {
		(this->set_flag(Args{}), ...);
	}
	
	constexpr friend NOTRE_FORCE_INLINE auto operator+(flags f, pcre::mode_case_insensitive) noexcept {
		f.case_insensitive = true;
		return f;
	}
	
	constexpr friend NOTRE_FORCE_INLINE auto operator+(flags f, pcre::mode_case_sensitive) noexcept {
		f.case_insensitive = false;
		return f;
	}
	
	constexpr friend NOTRE_FORCE_INLINE auto operator+(flags f, pcre::mode_singleline) noexcept {
		f.multiline = false;
		return f;
	}
	
	constexpr friend NOTRE_FORCE_INLINE auto operator+(flags f, pcre::mode_multiline) noexcept {
		f.multiline = true;
		return f;
	}
	
	constexpr NOTRE_FORCE_INLINE void set_flag(notre::singleline) noexcept {
		multiline = false;
	}
	
	constexpr NOTRE_FORCE_INLINE void set_flag(notre::multiline) noexcept {
		multiline = true;
	}
	
	constexpr NOTRE_FORCE_INLINE void set_flag(notre::case_insensitive) noexcept {
		case_insensitive = true;
	}
	
	constexpr NOTRE_FORCE_INLINE void set_flag(notre::case_sensitive) noexcept {
		case_insensitive = false;
	}

	// the callout handler is bound into the AST, not into the runtime flags
	template <typename... Handler> constexpr NOTRE_FORCE_INLINE void set_flag(notre::with_callouts<Handler...>) noexcept { }
};

constexpr NOTRE_FORCE_INLINE auto not_empty_match(flags f) {
	f.block_empty_match = true;
	return f;
}

constexpr NOTRE_FORCE_INLINE auto consumed_something(flags f, bool condition = true) {
	if (condition) f.block_empty_match = false;
	return f;
}

constexpr NOTRE_FORCE_INLINE bool cannot_be_empty_match(flags f) {
	return f.block_empty_match;
}

constexpr NOTRE_FORCE_INLINE bool multiline_mode(flags f) {
	return f.multiline;
}

constexpr NOTRE_FORCE_INLINE bool is_case_insensitive(flags f) {
	return f.case_insensitive;
}

} // namespace notre

#endif