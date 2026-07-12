# notre

Compile-time PCRE regex library. Header-only, C++17/20 (experimental C++23 module). **A renamed fork of CTRE** (hanickadot/compile-time-regular-expressions). A pattern is a `ctll::fixed_string` NTTP, parsed and lowered to a type-level AST at compile time, then `evaluate()` walks it over the subject at compile *or* run time. Repo: github.com/alexios-angel/notre (work on `main`). Deeper maintainer notes: `NOTES.md`.

## Fork attribution — do not remove
- Only `notre` and public `NOTRE_*` names are renamed. The `ctll/` sublibrary and `pcre`/PCRE grammar are **NOT** renamed — leave `ctll::`, `CTLL_*`, `pcre` as-is; `notre` builds on top of them.
- Upstream CTRE attribution MUST be preserved in: `NOTICE`, the README fork-notice, the `conanfile.py` author line, and `pcre.gram`.

## Build & test
Tests are `static_assert` suites — **compiling IS running them**. Clean compile = green. No binaries to run (except benchmarks). Must be warning-clean under `-Werror -Wconversion`.
```bash
make                    # C++20, uses $CXX; builds PCH then every test
make CXX=g++ -j$(nproc)
make CXX=clang++
make clean
make pch                # build the notre.hpp PCH alone
```
Flags are fixed (via `override CXXFLAGS`): `-std=c++20 -Iinclude -O3 -pedantic -Wall -Wextra -Werror -Wconversion`.

**C++17** — plain `make CXX_STANDARD=17` FAILS (`tests/_unicode.cpp` UDL under `-pedantic`; pre-existing, not a bug). Use the CI incantation:
```bash
make CXX=c++ CXX_STANDARD=17 CXXFLAGS=-DNOTRE_ENABLE_LITERALS PEDANTIC=""
```

Other builds:
```bash
cd examples && make run                 # separate build, -O2, not -Werror
cmake . -B build                        # header-only INTERFACE target (notre::notre)
cmake . -B build -DNOTRE_MODULE=ON      # C++23 module (import notre; needs `import std`)
cmake . -G Ninja -B build -DNOTRE_BUILD_TESTS=ON && cmake --build build --target notre-test
```

## Layout (`include/`)
- `ctll/` — generic, regex-agnostic compile-time LL(1) parser toolkit (`fixed_string`, `parser`, `grammars`, `list`/`map`). Keeps `ctll` namespace.
- `notre/` — the regex layer: grammar table, semantic actions, atom AST, evaluator, public API, diagnostics.
- `unicode-db/` — vendored third-party Unicode DB (`uni::`); optional, only via `atoms_unicode.hpp`.
- Aggregators: `notre.hpp` (core), `notre-unicode.hpp` (core + `\p{}` + UTF-8), `ctll.hpp`, `unicode-db.hpp`.

## Pipeline (pattern → matcher)
`fixed_string` NTTP → `ctll::parser<notre::pcre, ..., notre::pcre_actions>` → semantic actions build a type-level atom AST → `resolve_subroutines()` inlines `(?N)`/`(?&name)` → `regex_builder<input>` (emits syntax-error `static_assert`s) → `regular_expression<RE,Method,Modifier>` + `evaluate()`.

Key files: `wrapper.hpp` (public API + `regex_builder` + `regular_expression`), `evaluation.hpp` (matcher engine — largest logic file), `pcre_actions.hpp` + `actions/*.inc.hpp` (AST-building actions), `atoms*.hpp` (AST node types), `resolve_subroutines.hpp`, `return_type.hpp` (result object), `ctll/parser.hpp` (LL(1) driver).

## Grammar is generated — do not hand-edit
- `include/notre/pcre.hpp` is a **generated** LL(1) table. Source of truth: `include/notre/pcre.gram`.
- `make regrammar` regenerates it via Tablewright (the open-source `desatomat` replacement).
- Gotcha: naming a new single-char terminal removes it from the implicit `other` set — re-add it to `nonspecial_characters` if it should still match literally.

## Single-header (generated; `single-header/` is not hand-edited)
`make single-header` amalgamates via `quom` (`python3.9 -m quom`; override `PYTHON=`). **quom 4.0.2 is broken on this repo** — committed single-headers keep their guards (older quom). Do NOT change the Makefile's quom call.

## Public API (namespace `notre`, all in `wrapper.hpp`)
Matcher variable templates (store the functor without `()`): `match`, `search`, `starts_with`, `search_all` (`range` is `[[deprecated]]`), `split`, `tokenize`, `iterator`/`sentinel`, plus a `multiline_*` twin for each. Range-pipe API via `operator|`. Factory `re()` in `functions.hpp`; UDL `_notre` in `notre::literals`.
Modifiers are trailing template args: `notre::match<"hi", notre::case_insensitive>(sv)`. Flags in `flags_and_modes.hpp` (`singleline`/`multiline`/`case_sensitive`/`case_insensitive`/`ci`/`cs`/`with_callouts<...>`); inline `(?i)(?s)(?m)(?c)` also work.
Results (`regex_results`): `operator bool`, `to_view()`/`to_string()`, `to_number<R>()`, `get<N>()` (0 = whole match), `get<"name">()`, structured bindings.

## Conventions
- **Tabs** for indent; K&R/Egyptian braces; short bodies one-lined. `constexpr`/`noexcept` everywhere; matchers add `NOTRE_FLATTEN`+`NOTRE_FORCE_INLINE`.
- Macros are `NOTRE_*` (in `notre/utility.hpp`). Prefix every public template/struct/var/enum with `NOTRE_EXPORT`. Guard every stdlib `#include` with `#ifndef NOTRE_IN_A_MODULE`.
- **C++17/20 split:** never use a bare `ctll::fixed_string` pattern param — use `NOTRE_REGEX_INPUT_TYPE` (wrapper) / `NOTRE_PATTERN_INPUT` (diagnostics). `NOTRE_CNTTP_COMPILER_CHECK` gates cNTTP (C++20 literal patterns) vs C++17 (`constexpr auto p = ctll::fixed_string{...}` passed by ref).
- Include guards: match neighbouring files (`NOTRE_V2__NOTRE__<NAME>__HPP` older, `NOTRE__<NAME>__HPP` newer).
- `static_assert` message masking: `static_assert(cond && incomplete_type{}, "msg")` swallows the message — put the message on a separate `static_assert` over a plain bool (see `regex_builder`/`problem_at_position<N>`).

## Diagnostics / debug leaf layer (included LAST in `notre.hpp`)
`diagnostics.hpp`, `debug.hpp`, `assert.hpp` depend only on already-available core headers (keeps the single-header graph stable). They are **NOT standalone-parseable** — clangd reports many FALSE errors on them; trust the real g++/clang build, not clangd.
- `notre::valid<"...">`, `error_info<"...">()`, `error_message<"...">()` — never hard-error and ignore call args; swap `match<P>(args)` → `error_message<P>(args)` to debug in place. `NOTRE_VERBOSE_ERRORS` surfaces position/line/column in the backtrace.
- `notre::debug::dump_ast<"...">()` — renders the resolved atom tree (post desugar/quantifier-lowering/subroutine-inlining) as an indented string.
- `NOTRE_CONSTEXPR_ASSERT(cond, msg)` — armed only when `NOTRE_DEBUG` is defined; compiles away otherwise.
