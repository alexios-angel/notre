Regex Syntax
============

The library supports most of the `PCRE <pcre.org>`_ syntax with a few exceptions:

- callouts
- conditional patterns with a recursion condition ((?(R)...); other conditions work)
- control characters (\\cX)
- horizontal / vertical character classes (\\h\\H\\v\\V)
- match point reset inside lookarounds (\\K elsewhere is supported)
- named characters
- octal numbers
- options / modes
- recursion ((?R), (?0), recursive subroutine calls; non-recursive calls work)
- unicode grapheme cluster (\\X)

TODO more detailed regex information