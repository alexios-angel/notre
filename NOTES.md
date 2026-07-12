# notre — working notes (for Claude, future reference)

Scratch/reference notes on this project. Not user-facing docs; the README
is the real documentation. These are the things that were non-obvious or
cost time.

## What this is

`notre` is a fork of **CTRE** (Compile Time Regular Expressions, Hana
Dusíková) renamed to notre:
- namespace `notre`, macros `NOTRE_*`, include guards `NOTRE_*__HPP`,
  the `_notre` UDL, module `notre`, files `include/notre*`, `notre/`.
- The **`ctll`** sublibrary (the LL(1) parser under `include/ctll/`) is
  **kept as-is** — do NOT rename it. `pcre`/`PCRE` are also untouched
  (they don't contain the `ctre` token).
- Upstream attribution is preserved verbatim and must stay: `NOTICE`,
  the README fork-notice blockquote (`("CTRE")` + vcpkg-upstream refs),
  the `conanfile.py` author line (`ctre@hanicka.net`), and the
  `include/notre/pcre.gram` "descends from … CTRE" line.

Repo: github.com/alexios-angel/notre — work on `main`, push directly.

## Build & test

`make` = compile all `tests/*.cpp` (compile IS the test; they're
`static_assert` suites). Default `CXX_STANDARD := 20`. Flags:
`-O3 -pedantic -Wall -Wextra -Werror -Wconversion`. There's a PCH of
`include/notre.hpp` (`notre.pch` for clang, `.gch` for gcc).

- gcc C++20:   `make`
- clang C++20: `make CXX=clang++`
- **C++17 needs the CI flags**: `make CXX_STANDARD=17 CXXFLAGS=-DNOTRE_ENABLE_LITERALS PEDANTIC=""`.
  Plain `make CXX_STANDARD=17` FAILS on `tests/_unicode.cpp` because the
  `_notre` UDL needs `NOTRE_ENABLE_LITERALS` and no `-pedantic` (see
  README "Supported compilers"). This is pre-existing, not a bug.
- `.github/workflows/tests.yml` is the live CI (gcc 10–14, clang 14–22,
  C++17/20, libc++/libstdc++).

## Single-header regeneration — READ THIS

`make single-header` runs `quom`. **quom 4.0.2 (the one pip installs here)
is broken for this repo**:
- without `--include_guard` it does not dedup and the amalgam runs away
  (megabytes, then a timeout);
- with `--include_guard '[A-Z][A-Z0-9_]*_HPP'` it dedups but mis-strips
  guards, producing unbalanced `#endif` → won't compile.

The committed single-headers were made with an older/working quom that
auto-detects guards (and they keep their guards). **Do not change the
Makefile's quom call** — it works with the maintainer's quom.

When I needed to regenerate in this environment I used a tiny custom
guard-aware amalgamator (recursively inline `#include "..."` once per
resolved path, keep `<system>` includes). Also note: the committed
single-headers were **stale** (predated `resolve_subroutines`), so I
regenerated all three fresh from current `include/`.

## Diagnostics / debug layer I added

Leaf headers, included LAST in `notre.hpp` (they rely on include order —
they are NOT self-contained, like most headers here):
- `include/notre/diagnostics.hpp` — `notre::valid<P>`,
  `error_info<P>(...)`, `error_message<P>(...)` (a caret rendering).
  Both `error_*` **ignore any call args** so you can swap
  `match<P>(subject)` → `error_message<P>(subject)`. `NOTRE_VERBOSE_ERRORS`
  puts pos/line/col in the backtrace.
- `include/notre/debug.hpp` — `notre::debug::dump_ast<P>()` renders the
  compiled atom tree. Use this to check what a pattern ACTUALLY compiles
  to (better than trusting any static "supported features" list — the old
  NOTES.md's unsupported list was stale: atomic groups `(?>…)`, mode
  switches `(?i)`, and `\B` all work, for instance).
- `include/notre/assert.hpp` — `NOTRE_CONSTEXPR_ASSERT` under `NOTRE_DEBUG`.
- Tests: `tests/diagnostics.cpp`. README has a "Debugging" section.

## Codebase gotchas

- **static_assert message masking**: `static_assert(cond && incomplete_type{}, "msg")`
  swallows "msg" — the ill-formed sub-expression is reported instead. Put
  the message on its own `static_assert(bool_cond, "msg")`, and route the
  condition through a plain local `bool` so clang prints the message
  instead of dumping the whole parser result type. `regex_builder` in
  `wrapper.hpp` is where `match<>` validates; `problem_at_position<N>`
  (left undefined on purpose) surfaces the failing offset N.
- **clangd is noisy and wrong here**: the language server reports piles of
  false errors ("no template named 'list'", "no std::string_view", etc.)
  on the leaf headers because they aren't standalone-parseable. Ignore
  clangd; trust the real g++/clang build.
- **fixed_string storage** (`include/ctll/fixed_string.hpp`): default
  (byte) build stores raw UTF-8 bytes for `char`; `NOTRE_STRING_IS_UTF8`
  decodes to code points. Matters for caret column counting.
- **Parsed AST atom types** (for dump_ast specializations): `sequence`,
  `select` (alternation), `repeat<a,b,...>` (b==0 = unbounded),
  `lazy_repeat`, `possessive_repeat`, `capture<Idx,...>`,
  `capture_with_name<Idx, id<chars...>, ...>`, `back_reference`,
  `character<V>`, `string<...>`, `set`/`negative_set`/`char_range`,
  `boundary<word_chars>` = `\b`, `boundary<negative_set<word_chars>>` =
  `\B` (NOT `not_boundary`, which the parser never emits). Get a type's
  exact spelling with an undefined `template<class> struct tell;` +
  `tell<T> x;`.
- `single-header/` is generated; `tests/` and `examples/` use `include/`.

## Removed as cruft (recoverable from git)

`.gitmodules` (empty), `future.cpp` (root scratch), `.travis.yml` (dead CI),
`clang-bench.txt` / `gcc-bench.txt` (stale unreferenced bench captures).
