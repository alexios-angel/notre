# Examples

Small, self-contained programs demonstrating the library. Each file is
independently compilable against the headers in `../include` and prints
its results, and most also `static_assert` the same behaviour at compile
time.

| File | Shows |
|------|-------|
| [`match-basics.cpp`](match-basics.cpp) | `match` / `search` / `starts_with`, functors, constexpr matching |
| [`extract-date.cpp`](extract-date.cpp) | capture groups via structured bindings, fully compile-time extraction |
| [`named-captures.cpp`](named-captures.cpp) | `(?<name>...)` groups, `get<"name">()`, `to_number<R>()` |
| [`lexer.cpp`](lexer.cpp) | classifying tokens with one alternation pattern |
| [`search-all.cpp`](search-all.cpp) | `search_all` ranges, pipe syntax, `iterator`/`sentinel`, multi-subject piping |
| [`split-tokenize.cpp`](split-tokenize.cpp) | `split` (with separator captures) and `tokenize` |
| [`flags-and-modes.cpp`](flags-and-modes.cpp) | `ctre::case_insensitive`, inline `(?i)` modes, `multiline_*` functions |
| [`unicode.cpp`](unicode.cpp) | UTF-8 subjects and `\p{...}` Unicode properties |
| [`cxx17-syntax.cpp`](cxx17-syntax.cpp) | the C++17 API: `ctll::fixed_string` patterns and the `_ctre` UDL |

## Building

```bash
make        # build everything
make run    # build and run everything
make clean
```

All examples build in C++20 mode except `cxx17-syntax.cpp`, which builds
with `-std=c++17` to demonstrate the C++17 API. Individual example:

```bash
clang++ -std=c++20 -I../include search-all.cpp -o search-all && ./search-all
```
