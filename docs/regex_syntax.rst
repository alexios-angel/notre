Regex Syntax
============

The library supports most of the `PCRE <pcre.org>`_ syntax with a few exceptions:

- callouts
- conditional patterns with a recursion condition ((?(R)...); other conditions work)
- horizontal / vertical character classes (\\h\\H\\v\\V)
- match point reset inside lookarounds (\\K elsewhere is supported)
- named characters
- octal numbers without a leading zero (\\o{ddd} and \\0dd work)
- options / modes
- recursion ((?R), (?0), recursive subroutine calls; non-recursive calls work)
- full UAX #29 grapheme clusters (\\X uses the \\P{M}\\p{M}* approximation)

TODO more detailed regex information