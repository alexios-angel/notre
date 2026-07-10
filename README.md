> **Fork notice:** This repository is a derivative work based on
> [hanickadot/compile-time-regular-expressions](https://github.com/hanickadot/compile-time-regular-expressions)
> ("CTRE") by Hana Dusíková and contributors, licensed under the Apache
> License 2.0 with LLVM Exceptions. It was created by manually cloning the
> upstream source and reinitializing git history rather than using GitHub's
> fork feature, so the original commit history and authorship metadata are
> not preserved in this repo's `git log`. See [NOTICE](NOTICE) for full
> attribution. All code as of the initial commit is unmodified upstream
> code; changes made afterward are tracked in this repository's own history.

# Compile Time Regular Expressions v3

[![Build Status](https://github.com/alexios-angel/notre/actions/workflows/tests.yml/badge.svg)](https://github.com/alexios-angel/notre/actions/workflows/tests.yml)

Fast compile-time regular expressions with support for matching/searching/capturing during compile-time or runtime.

```c++
ctre::match<"REGEX">(subject);   // C++20
"REGEX"_ctre.match(subject);     // C++17 + N3599 extension
```

The pattern is parsed and compiled into a matcher when *your code* compiles — a malformed regex is a compile error, not a runtime exception — and matching itself can run at compile time (`constexpr`) or at runtime with no allocation and no regex-compilation cost.

The library is header-only. You can use the single-header version from the `single-header` directory, consume it as a CMake target (`ctre::ctre`), or import it as a C++ module (experimental, see below).

More info at [compile-time.re](https://compile-time.re/). Original upstream documentation and talks by the library's author, Hana Dusíková, cover the design in depth.

## Contents

- [Overview](#overview)
- [Basic API](#basic-api)
- [Iterating over multiple matches](#iterating-over-multiple-matches)
- [Working with results](#working-with-results)
- [Flags and modes](#flags-and-modes)
- [Callouts](#callouts)
- [Pattern syntax variants (C++17 / C++20)](#pattern-syntax-variants)
- [Unicode support](#unicode-support)
- [Examples](#examples)
- [Supported compilers](#supported-compilers)
- [Installation and integration](#installation-and-integration)
- [Running tests](#running-tests-for-developers)
- [License](#license)

## Overview

What the library can do:

* Matching (`match` — whole input must match)
* Searching (`search` — find a match anywhere; `starts_with` — match a prefix)
* Iterating over all matches (`search_all`), splitting (`split`), tokenizing (`tokenize`)
* Capturing content, including named captures — `(?<name>...)`, `(?P<name>...)`, and `(?'name'...)` syntaxes
* Back-references (`\g{N}` syntax, and `\1`...`\9` too)
* Subroutine calls — `(?N)`, relative `(?+N)`/`(?-N)`, `(?&name)`, `(?P>name)`, `\g<name>`, `\g'name'` — inlined at compile time as atomic, non-capturing copies of the called group (recursive calls are a compile error)
* Match point reset (`\K`) — the part before it must match but is excluded from the reported match (not allowed inside lookarounds, as in PCRE2)
* Conditional patterns — `(?(1)yes|no)`, relative `(?(-1)`/`(?(+1)`, named `(?(<name>)`/`(?('name')`/`(?(name)`, assertion conditions `(?(?=...)yes|no)`, and `(?(DEFINE)...)` for building reusable subpattern libraries with subroutine calls
* Lookahead `(?=...)` / `(?!...)` and lookbehind `(?<=...)` / `(?<!...)`
* Atomic groups `(?>...)` and possessive quantifiers (`a++`, `a*+`, ...)
* Lazy and greedy quantifiers
* Multiline matching (`multiline_*` function variants)
* Case-insensitive matching (`ctre::case_insensitive` modifier or inline `(?i)`)
* Comments with the `(?#...)` syntax
* Callouts — `(?Cn)` and `(?C'name')` observation points that can veto match positions, dispatched through a user handler (see [Callouts](#callouts))
* Octal escapes (`\o{ddd...}` and `\0dd`) and control characters (`\cX`)
* Unicode properties (`\p{...}`), UTF-8 support, and grapheme clusters (`\X`, approximated as an atomic `\P{M}\p{M}*`)
* Everything works in `constexpr` context (you can `static_assert` a match)

The library implements most of the PCRE syntax with a few exceptions:

* named characters
* options / modes — except the inline mode switches `(?i)`, `(?c)`, `(?s)`, `(?m)` (and combinations like `(?im)`), which are supported
* recursion — `(?R)`, `(?0)`, and recursive subroutine calls (non-recursive calls are supported and expanded at compile time; unbounded recursion cannot be)
* octal escapes without a leading zero (`\100`) — write `\o{100}` or `\040`; `\1`..`\9` always mean backreferences

More documentation on the PCRE syntax at [pcre.org](https://www.pcre.org/current/doc/html/pcre2syntax.html).

### Unknown character escape behaviour

Not every escaped character inserts itself literally: escaped characters with a special meaning are interpreted, and an *unknown* escaped character is a compile-time syntax error.

Explicitly allowed character escapes which insert just the character are:

```
\- \" \< \>
```

## Basic API

This is an approximated API specification from a user perspective (omitting `constexpr` and `noexcept`, which are everywhere, and using C++20 syntax even though the API is also C++17 compatible):

```c++
// whole input must match the regex:
template <ctll::fixed_string regex> auto ctre::match(auto Range &&) -> regex_results;
template <ctll::fixed_string regex> auto ctre::match(auto First &&, auto Last &&) -> regex_results;

// look for a match anywhere inside the input:
template <ctll::fixed_string regex> auto ctre::search(auto Range &&) -> regex_results;
template <ctll::fixed_string regex> auto ctre::search(auto First &&, auto Last &&) -> regex_results;

// check if the input starts with a match (doesn't need to match everything):
template <ctll::fixed_string regex> auto ctre::starts_with(auto Range &&) -> regex_results;
template <ctll::fixed_string regex> auto ctre::starts_with(auto First &&, auto Last &&) -> regex_results;
```

Each function also has a `multiline_` variant (see [Flags and modes](#flags-and-modes)).

### Functors

All of the functions (`ctre::match`, `ctre::search`, `ctre::starts_with`, `ctre::search_all`, `ctre::split`, `ctre::tokenize`, `ctre::iterator`) are functors — variable templates instantiating a callable — so you can store and pass them without parentheses:

```c++
constexpr auto matcher = ctre::match<"[a-z]+">;
if (matcher(input)) { ... }
```

### Possible subjects (inputs)

* `std::string_view`, `std::wstring_view`, `std::u8string_view` (matched as UTF-8), `std::u16string_view`, `std::u32string_view`
* `std::string`-like objects (anything providing `begin`/`end` with at least forward iterators)
* zero-terminated `const char *` / `const wchar_t *` (no `strlen` needed — the terminator is detected during matching)
* pairs of forward iterators

## Iterating over multiple matches

```c++
// search for the regex repeatedly, returning each occurrence (skips non-matching input):
template <ctll::fixed_string regex> auto ctre::search_all(auto Range &&) -> range of regex_results;

// consecutive matches anchored to each previous match's end, stopping at the first gap:
template <ctll::fixed_string regex> auto ctre::tokenize(auto Range &&) -> range of regex_results;

// parts of the input between matches (the separator regex's own captures stay accessible on each result):
template <ctll::fixed_string regex> auto ctre::split(auto Range &&) -> range of regex_results;
```

> `ctre::range` is a deprecated alias of `ctre::search_all` and will warn on use.

These are lazy ranges — matching happens as you iterate:

```c++
for (auto match : ctre::search_all<"[0-9]+">("1, 2, 42")) {
    std::cout << match.to_view() << '\n';   // 1  2  42
}
```

They also support a pipeline style:

```c++
for (auto match : input | ctre::search_all<"[0-9]+">) { ... }
```

Piping a range of *subjects* (e.g. a vector of strings) into a non-range function like `ctre::match` yields one result per subject:

```c++
std::vector<std::string_view> lines = ...;
for (auto m : lines | ctre::match<"[a-z]+">) {
    if (m) { ... }   // one regex_results per line
}
```

If you need explicit iterators, `ctre::iterator<"regex">(input)` returns an iterator you can compare against `ctre::sentinel`.

## Working with results

The result of `match`/`search`/`starts_with` (and of dereferencing the range/iterator forms) is a `regex_results` object; each capture is accessible through `get`:

```c++
template <...> struct regex_results {
    operator bool() const;                       // did it match?

    auto to_view() const -> std::string_view;    // also view()
    auto to_string() const -> std::string;       // also str()
    operator std::string_view() const;           // implicit conversion (all char variants supported)
    explicit operator std::string() const;

    auto to_optional_view() const -> std::optional<std::string_view>;
    auto to_optional_string() const -> std::optional<std::string>;

    template <typename R = int> auto to_number(...) const -> R;                 // via std::from_chars
    template <typename R = int> auto to_optional_number(...) const -> std::optional<R>;

    size_t count() const;                        // number of captures (including implicit capture 0)
    template <size_t Id> auto get() const;       // capture by number (0 = whole match)
    template <ctll::fixed_string Name> auto get() const;  // capture by name (C++20)

    // also size(), data(), begin(), end(), comparison with string_view, and ostream <<
};
```

Notes:

* Capture `0` is the whole match; explicit capture groups are numbered from `1`.
* Each capture object has the same conversion API as the whole result (`to_view`, `to_string`, `to_number`, `operator bool` for "did this group participate", `size()`, ...).
* `to_view` / `data()` require the subject to have been a contiguous, non-reverse range; matching itself works with plain forward iterators.
* `to_number` uses `std::from_chars`, so it's `constexpr` only where your standard library makes `from_chars` `constexpr` (C++23).
* `regex_results` supports structured bindings: `auto [whole, first, second] = ctre::match<...>(input);`

## Flags and modes

Case-insensitive and multiline matching can be selected three ways:

```c++
// 1. modifier as an extra template argument (ctre::case_insensitive, ctre::case_sensitive,
//    aliases ctre::ci / ctre::cs, and ctre::multiline / ctre::singleline):
ctre::match<"hello", ctre::case_insensitive>("HeLLo");

// 2. inline mode switch inside the pattern: (?i) (?c) (?s) (?m), e.g.
ctre::match<"(?i)hello">("HELLO");

// 3. multiline_ function variants, which make ^ and $ match at line boundaries:
ctre::multiline_search<"^bar$">("foo\nbar");
```

Every function has a `multiline_` twin: `multiline_match`, `multiline_search`, `multiline_starts_with`, `multiline_search_all`, `multiline_split`, `multiline_tokenize`, `multiline_iterator` (+ `multiline_sentinel`).

## Callouts

`(?Cn)` (numbered) and `(?C'name')` / `(?C"name")` (named) are zero-width observation points. Attach implementations with the `ctre::with_callouts` modifier as inline entries — one `ctll::callout` per name or number:

```c++
// the veto makes the greedy + backtrack: group 1 is "12", not "123"
ctre::search<"([0-9]+)(?C'limit')", ctre::with_callouts<
    ctll::callout<"limit", [](const auto & c) { return c.position - c.match_start <= 2; }>,
    ctll::callout<2,       [](const auto & c) { /* pure observer for (?C2) */ }>
>>("xx123yy");
```

Each entry is matched to its callout at compile time (numbers by their decimal spelling: `ctll::callout<7, f>` serves `(?C7)`) and invoked directly, with no lookup at match time. The callable receives a `ctre::callout_data` — the callout's number and name, the whole subject, and the code-unit offsets of the current position and the match attempt's start — and may return `ctre::callout_result`, `bool` (`true` = proceed), or `void` (observer). `fail`/`false` vetoes the position and normal backtracking continues, like PCRE's nonzero callout return. Generic lambdas work with any subject character type.

For dispatch chosen at match time instead, pass one handler type whose static `callouts()` returns a `ctll::map` from names to `ctll::function`s:

```c++
struct my_handler {
    static constexpr auto callouts() {
        return ctll::map{
            std::pair{"limit", ctll::function<ctre::callout_result(const ctre::callout_data<char> &)>(
                [](const auto & c) {
                    return c.position - c.match_start <= 2 ? ctre::callout_result::proceed
                                                           : ctre::callout_result::fail;
                })},
        };
    }
};
ctre::search<"([0-9]+)(?C'limit')", ctre::with_callouts<my_handler>>("xx123yy");
```

In both forms, a callout with no matching entry or key — or a pattern used without the modifier — is a no-op, like PCRE with no callout function set. Both require C++20 (entries need class-type template parameters, the map form needs `ctll::function`). Restrictions: the subject must be a contiguous character range (no zero-terminated pointers, no UTF-8 range), and callouts inside lookbehinds are not supported (both are compile errors).

## Pattern syntax variants

### C++20 syntax (cNTTP)

With compilers supporting class-type non-type template parameters you can write the pattern directly as a string literal template argument:

```c++
constexpr auto match(std::string_view sv) noexcept {
    return ctre::match<"h.*">(sv);
}
```

### C++17 syntax

Provide the pattern as a `constexpr ctll::fixed_string` variable:

```c++
static constexpr auto pattern = ctll::fixed_string{ "h.*" };

constexpr auto match(std::string_view sv) noexcept {
    return ctre::match<pattern>(sv);
}
```

To access a named capture in C++17, the name also needs to be a `ctll::fixed_string` variable with linkage:

```c++
static constexpr ctll::fixed_string year_name = "year";
auto year = result.get<year_name>();
```

### Template UDL syntax

The compiler must support the N3599 extension (a GNU extension in gcc and clang):

```c++
constexpr auto match(std::string_view sv) noexcept {
    using namespace ctre::literals;
    return "h.*"_ctre.match(sv);
}
```

If you need the N3599 extension in GCC 9.1+, you can't use `-pedantic`, and you need to define the macro `CTRE_ENABLE_LITERALS`.

## Unicode support

To enable Unicode (UTF-8 decoding and `\p{...}` property classes) include:

* `<ctre-unicode.hpp>`
* or `<ctre.hpp>` and `<unicode-db.hpp>`

Otherwise you will get missing symbols if you try to use the Unicode support without enabling it. Subjects of type `std::u8string_view` are decoded as UTF-8 during matching.

```c++
#include <ctre-unicode.hpp>
#include <iostream>

// needed if you want to output to the terminal
std::string_view cast_from_unicode(std::u8string_view input) noexcept {
    return std::string_view(reinterpret_cast<const char *>(input.data()), input.size());
}

int main() {
    using namespace std::literals;
    std::u8string_view original = u8"Tu es un génie"sv;

    for (auto match : ctre::search_all<"\\p{Letter}+">(original))
        std::cout << cast_from_unicode(match) << std::endl;
}
```

[link to compiler explorer](https://godbolt.org/z/erTshe6sz)

## Examples

Runnable versions of these (and more) live in the [`examples/`](examples/) directory — each is a self-contained program, built with `make` in that directory.

### Extracting a number from input

```c++
std::optional<std::string_view> extract_number(std::string_view s) noexcept {
    if (auto m = ctre::match<"[a-z]+([0-9]+)">(s)) {
        return m.get<1>().to_view();
    } else {
        return std::nullopt;
    }
}
```

[link to compiler explorer](https://gcc.godbolt.org/z/5U67_e)

### Extracting values from a date — at compile time

```c++
struct date { std::string_view year; std::string_view month; std::string_view day; };

constexpr std::optional<date> extract_date(std::string_view s) noexcept {
    if (auto [whole, year, month, day] = ctre::match<"(\\d{4})/(\\d{1,2})/(\\d{1,2})">(s); whole) {
        return date{year, month, day};
    } else {
        return std::nullopt;
    }
}

static_assert(extract_date("2018/08/27"sv).has_value());
static_assert((*extract_date("2018/08/27"sv)).year == "2018"sv);
static_assert((*extract_date("2018/08/27"sv)).month == "08"sv);
static_assert((*extract_date("2018/08/27"sv)).day == "27"sv);
```

[link to compiler explorer](https://gcc.godbolt.org/z/x64CVp)

### Using named captures

```c++
auto result = ctre::match<"(?<year>\\d{4})/(?<month>\\d{1,2})/(?<day>\\d{1,2})">(s);
return date{result.get<"year">(), result.get<"month">(), result.get<"day">()};

// or use numbered access — capture 0 is the whole match
return date{result.get<1>(), result.get<2>(), result.get<3>()};
```

### Converting captures to numbers

```c++
int port_of(std::string_view address) noexcept {
    if (auto m = ctre::search<":([0-9]+)">(address)) {
        return m.get<1>().to_number<int>();
    }
    return -1;
}
```

### Lexer

```c++
enum class type {
    unknown, identifier, number
};

struct lex_item {
    type t;
    std::string_view c;
};

std::optional<lex_item> lexer(std::string_view v) noexcept {
    if (auto [m, id, num] = ctre::match<"([a-z]+)|([0-9]+)">(v); m) {
        if (id) {
            return lex_item{type::identifier, id};
        } else if (num) {
            return lex_item{type::number, num};
        }
    }
    return std::nullopt;
}
```

[link to compiler explorer](https://gcc.godbolt.org/z/PKTiCC)

### Iterating over all matches in input

```c++
auto input = "123,456,768"sv;

for (auto match : ctre::search_all<"([0-9]+),?">(input)) {
    std::cout << std::string_view{match.get<0>()} << "\n";
}
```

### Splitting

```c++
for (auto part : ctre::split<",">("a,b,c"sv)) {
    std::cout << part.to_view() << "\n";   // a  b  c
}
```

## Supported compilers

The CI matrix (`.github/workflows/tests.yml`) builds every test in both C++17 and C++20 mode where applicable:

* clang 14.0+ (C++17 syntax, C++20 cNTTP syntax, template UDL; libc++ and libstdc++)
* Apple clang / Xcode 15.0+ (C++17 syntax, C++20 cNTTP syntax, template UDL)
* gcc 9.0+ (C++17 & C++20 cNTTP syntax; UDL needs `CTRE_ENABLE_LITERALS` and no `-pedantic` on 9.1+)
* MSVC 14.29+ (Visual Studio 16.11+) (C++20 cNTTP syntax)

## Installation and integration

CTRE is header-only; the minimum viable integration is copying one file.

### Single header

Copy `single-header/ctre.hpp` (or `single-header/ctre-unicode.hpp` together with `single-header/unicode-db.hpp` for Unicode support) into your project. These files can be regenerated from `include/` with `make single-header` (requires Python with the [quom](https://pypi.org/project/quom/) tool).

### CMake

Add this repository as a subdirectory (or via `FetchContent`) and link against the target:

```cmake
add_subdirectory(notre)             # or FetchContent
target_link_libraries(your-target PRIVATE ctre::ctre)
```

Or install system-wide (`cmake --install`) and use:

```cmake
find_package(ctre REQUIRED)
target_link_libraries(your-target PRIVATE ctre::ctre)
```

The install also ships a `pkg-config` file (`ctre.pc`), and CPack can produce DEB/RPM/archive packages. The `CTRE_CXX_STANDARD` cache variable selects the C++ standard advertised by the target (default 20).

### C++ module (experimental)

With CMake 3.30+ and a modules-capable toolchain, configure with `-DCTRE_MODULE=ON` to build `ctre.cppm` as a named module (uses `import std;`, so it requires C++23):

```c++
import ctre;
```

### vcpkg

You can download and install ctre using the [vcpkg](https://github.com/Microsoft/vcpkg) dependency manager:

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install ctre
```

Note: the ctre port in vcpkg tracks the upstream project, not this fork. It is kept up to date by Microsoft team members and community contributors. If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository.

### Conan

A `conanfile.py` recipe is included in the repository root.

## Running tests (for developers)

Run `make` in the root of the project to build and run the test suite (add `CXX_STANDARD=17` for C++17 mode). Alternatively, configure with CMake and build the `ctre-test` target:

```bash
cmake -B build -DCTRE_BUILD_TESTS=ON
cmake --build build --target ctre-test
```

## License

Apache License 2.0 with LLVM Exceptions (see [LICENSE](LICENSE), unmodified from upstream). Attribution for the original work is preserved in [NOTICE](NOTICE).
